/**
 * @file http_server.c
 * @brief HTTP management server — streaming firmware upload
 *
 * Key design:
 *   - Headers are read into a small buffer (2KB max).
 *   - Firmware upload uses streaming: recv chunk → crc32 → fs_write → loop.
 *   - No single large buffer — 100KB+ firmware uploads work fine.
 */
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "main/http_server.h"
#include "main/firmware_store.h"
#include "main/auto_flasher.h"
#include "main/wifi_configuration.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include "lwip/sockets.h"
#include "rom/crc.h"

/* ---- HTTP response fragments ---- */
static const char HTTP_200[]     = "HTTP/1.0 200 OK\r\n";
static const char HTTP_400[]     = "HTTP/1.0 400 Bad Request\r\n";
static const char HTTP_404[]     = "HTTP/1.0 404 Not Found\r\n";
static const char HTTP_500[]     = "HTTP/1.0 500 Internal Error\r\n";
static const char HTTP_CT_HTML[] = "Content-Type: text/html; charset=utf-8\r\n";
static const char HTTP_CT_JSON[] = "Content-Type: application/json\r\n";
static const char HTTP_CONN_CL[] = "Connection: close\r\n";
static const char HTTP_CORS[]    = "Access-Control-Allow-Origin: *\r\n";

#define HDR_BUF_SIZE  2048   /* max header size */
#define CHUNK_SIZE    1024   /* recv / flash-write chunk */

/* ---- Embedded management page ---- */

static const char INDEX_HTML[] =
"<!DOCTYPE html><html lang=en><head><meta charset=utf-8>"
"<meta name=viewport content='width=device-width,initial-scale=1'>"
"<title>DAP Offline Programmer</title>"
"<style>"
"*{box-sizing:border-box;margin:0;padding:0}"
"body{font:14px/1.5 monospace;background:#1a1a2e;color:#e0e0e0;padding:16px;max-width:640px;margin:0 auto}"
"h1{font-size:20px;color:#00d4ff;margin-bottom:16px;text-align:center}"
".card{background:#16213e;border:1px solid #0f3460;border-radius:8px;padding:14px;margin-bottom:12px}"
".card h2{font-size:14px;color:#00d4ff;margin-bottom:8px;border-bottom:1px solid #0f3460;padding-bottom:4px}"
".row{display:flex;justify-content:space-between;padding:3px 0}"
".val{color:#7fff7f}"
"button{background:#0f3460;color:#00d4ff;border:1px solid #00d4ff;padding:8px 16px;border-radius:4px;cursor:pointer;font:inherit;margin:4px}"
"button:hover{background:#1a4a7a}"
"button:disabled{opacity:.4;cursor:not-allowed}"
"button.danger{border-color:#ff6b6b;color:#ff6b6b}"
"button.danger:hover{background:#4a1a1a}"
"#dropzone{border:2px dashed #0f3460;border-radius:8px;padding:30px;text-align:center;margin:8px 0;transition:all .2s}"
"#dropzone.hover{border-color:#00d4ff;background:#0f346020}"
"#progress{width:100%;height:8px;background:#0f3460;border-radius:4px;margin:8px 0;overflow:hidden;display:none}"
"#progress div{height:100%;background:#00d4ff;width:0%;transition:width .3s}"
"#log{max-height:150px;overflow-y:auto;font-size:11px;color:#888;white-space:pre-wrap}"
"input[type=file]{display:none}"
"</style></head><body>"
"<h1>DAP Offline Programmer</h1>"
"<p style=text-align:center;color:#888;font-size:12px;margin-bottom:16px>"
"Connect target -> Auto flash (no click needed)</p>"
"<div class=card>"
"<h2>ESP32-C3 Status</h2>"
"<div class=row><span>WiFi SSID:</span><span class=val id=wifi_ssid>--</span></div>"
"<div class=row><span>IP Address:</span><span class=val id=wifi_ip>--</span></div>"
"<div class=row><span>Free Heap:</span><span class=val id=free_heap>--</span></div>"
"</div>"
"<div class=card>"
"<h2>Stored Firmware</h2>"
"<div class=row><span>Status:</span><span class=val id=fw_status>--</span></div>"
"<div class=row><span>Size:</span><span class=val id=fw_size>--</span></div>"
"<div class=row><span>CRC32:</span><span class=val id=fw_crc>--</span></div>"
"<div class=row><span>Capacity:</span><span class=val id=fw_cap>--</span></div>"
"<div id=dropzone>Drop .bin file here or <a href=# id=browse_link style=color:#00d4ff>click to browse</a></div>"
"<input type=file id=file_input accept=.bin>"
"<div id=progress><div id=progress_bar></div></div>"
"<button id=btn_erase class=danger>Erase Firmware</button>"
"</div>"
"<div class=card>"
"<h2>Target Status</h2>"
"<div class=row><span>State:</span><span class=val id=flash_state>--</span></div>"
"<div class=row><span>Target:</span><span class=val id=target_name>--</span></div>"
"<button id=btn_flash disabled>Re-Flash</button>"
"</div>"
"<div class=card>"
"<h2>Log</h2>"
"<div id=log>Waiting...</div>"
"</div>"
"<script>"
"function $(id){return document.getElementById(id);}"
"function status(){"
"fetch('/status').then(r=>r.json()).then(s=>{"
"$('wifi_ssid').textContent=s.wifi_ssid||'--';"
"$('wifi_ip').textContent=s.wifi_ip||'--';"
"$('free_heap').textContent=(s.free_heap||0)+' B';"
"$('fw_status').textContent=s.fw_size>0?'Present ('+s.fw_size+' B)':'Empty';"
"$('fw_size').textContent=(s.fw_size||0)+' / '+(s.fw_capacity||0)+' B';"
"$('fw_crc').textContent=s.fw_crc?'0x'+s.fw_crc.toString(16).toUpperCase().padStart(8,'0'):'--';"
"$('fw_cap').textContent=(s.fw_capacity||0)+' B';"
"$('flash_state').textContent=s.flash_state||'--';"
"$('target_name').textContent=s.target_name||'--';"
"$('btn_flash').disabled=!(s.fw_size>0&&s.target_name!=='--');"
"}).catch(e=>{});}"
"var dz=$('dropzone'),fi=$('file_input');"
"dz.onclick=function(){fi.click();};"
"$('browse_link').onclick=function(e){e.stopPropagation();fi.click();};"
"dz.ondragover=function(e){e.preventDefault();dz.classList.add('hover');};"
"dz.ondragleave=function(){dz.classList.remove('hover');};"
"dz.ondrop=function(e){e.preventDefault();dz.classList.remove('hover');"
"var f=e.dataTransfer.files[0];if(f)upload(f);};"
"fi.onchange=function(){if(fi.files[0])upload(fi.files[0]);};"
"function upload(file){"
"var pb=$('progress'),pbb=$('progress_bar');pb.style.display='block';"
"var x=new XMLHttpRequest();"
"x.upload.onprogress=function(e){if(e.lengthComputable)pbb.style.width=(e.loaded/e.total*100)+'%';};"
"x.onload=function(){pb.style.display='none';status();log('Firmware uploaded: '+file.name+' ('+file.size+' B)');};"
"x.onerror=function(){pb.style.display='none';log('Upload failed');};"
"x.open('POST','/firmware');x.send(file);}"
"$('btn_erase').onclick=function(){"
"if(!confirm('Erase stored firmware?'))return;"
"fetch('/erase',{method:'POST'}).then(r=>r.json()).then(j=>{status();log(j.msg||'Erased');});};"
"$('btn_flash').onclick=function(){"
"if(!confirm('Re-flash the connected target?'))return;"
"$('btn_flash').disabled=true;"
"fetch('/flash',{method:'POST'}).then(r=>r.json()).then(j=>{status();log(j.msg||'Flash triggered');});};"
"function log(msg){$('log').textContent+='\\n['+new Date().toLocaleTimeString()+'] '+msg;"
"$('log').scrollTop=$('log').scrollHeight;}"
"status();setInterval(status,2000);"
"</script></body></html>";

/* ---- Helpers ---- */

static void send_hdr(int sock, const char *code, const char *ct, size_t clen) {
    char buf[256];
    int n = snprintf(buf, sizeof(buf), "%s%s%s%sContent-Length: %u\r\n\r\n",
                     code, ct, HTTP_CORS, HTTP_CONN_CL, (unsigned int)clen);
    send(sock, buf, n, 0);
}

static void send_json(int sock, const char *code, const char *json) {
    size_t len = strlen(json);
    send_hdr(sock, code, HTTP_CT_JSON, len);
    send(sock, json, len, 0);
}

/* ---- Route handlers (no body needed) ---- */

static void handle_get_status(int sock) {
    flash_status_t fs = auto_flasher_get_status();

    char ssid[32] = "--";
    char ip[32]   = "--";
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        snprintf(ssid, sizeof(ssid), "%s", (char *)ap_info.ssid);
    }
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif) {
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
            snprintf(ip, sizeof(ip), IPSTR, IP2STR(&ip_info.ip));
        }
    }

    char json[512];
    snprintf(json, sizeof(json),
             "{\"wifi_ssid\":\"%s\",\"wifi_ip\":\"%s\",\"free_heap\":%u,"
             "\"fw_size\":%u,\"fw_capacity\":%u,\"fw_crc\":%u,"
             "\"flash_state\":\"%s\",\"target_name\":\"%s\"}",
             ssid, ip, (unsigned int)esp_get_free_heap_size(),
             (unsigned int)fs_get_stored_size(),
             (unsigned int)fs_get_capacity(),
             (unsigned int)fs_get_checksum(),
             fs.state_name ? fs.state_name : "idle",
             fs.target_name ? fs.target_name : "--");
    send_json(sock, HTTP_200, json);
}

static void handle_post_flash(int sock) {
    flash_status_t fs = auto_flasher_get_status();
    if (fs.state != FLASH_STATE_TARGET_FOUND &&
        fs.state != FLASH_STATE_ARMED) {
        send_json(sock, HTTP_400, "{\"msg\":\"Target not ready\"}");
        return;
    }
    if (fs_get_stored_size() == 0) {
        send_json(sock, HTTP_400, "{\"msg\":\"No firmware stored\"}");
        return;
    }
    auto_flasher_trigger();
    send_json(sock, HTTP_200, "{\"msg\":\"Flash triggered\"}");
}

static void handle_post_erase(int sock) {
    esp_err_t err = fs_erase();
    if (err != ESP_OK) {
        send_json(sock, HTTP_500, "{\"msg\":\"Erase failed\"}");
        return;
    }
    send_json(sock, HTTP_200, "{\"msg\":\"Firmware erased\"}");
}

/* ---- Streaming firmware upload ---- */

/*
 * Receive firmware body from sock and stream it to flash.
 * `already_buf` / `already_len` is any body data that was already
 * read past the header terminator during header reading.
 */
static void handle_post_firmware_stream(int sock,
                                        int content_length,
                                        const uint8_t *already_buf,
                                        size_t already_len) {
    if (content_length <= 0 || content_length > (int)fs_get_capacity()) {
        send_json(sock, HTTP_400, "{\"msg\":\"Invalid firmware size\"}");
        return;
    }

    size_t total = content_length;
    os_printf("[HTTP] Streaming firmware upload: %u bytes\n", (unsigned int)total);

    /* Erase any previously stored firmware (metadata + payload) */
    if (fs_erase() != ESP_OK) {
        send_json(sock, HTTP_500, "{\"msg\":\"Flash erase failed\"}");
        return;
    }

    /* Allocate a chunk buffer for recv + write */
    uint8_t *chunk = (uint8_t *)malloc(CHUNK_SIZE);
    if (!chunk) {
        send_json(sock, HTTP_500, "{\"msg\":\"Out of memory\"}");
        return;
    }

    uint32_t crc = 0;
    size_t   offset = 0;
    int      ok = 1;

    /* Process any body bytes already in the header buffer */
    if (already_len > 0) {
        size_t n = already_len;
        if (n > total) n = total;
        crc = crc32_le(crc, already_buf, n);
        if (fs_write(0, already_buf, n) != ESP_OK) {
            ok = 0;
            goto done;
        }
        offset = n;
    }

    /* Stream remaining body data */
    while (offset < total && ok) {
        size_t want = total - offset;
        if (want > CHUNK_SIZE) want = CHUNK_SIZE;

        int n = recv(sock, chunk, want, 0);
        if (n <= 0) {
            os_printf("[HTTP] recv error at offset %u/%u: n=%d\n",
                      (unsigned int)offset, (unsigned int)total, n);
            ok = 0;
            break;
        }

        crc = crc32_le(crc, chunk, n);
        if (fs_write(offset, chunk, n) != ESP_OK) {
            os_printf("[HTTP] Flash write error at offset %u\n", (unsigned int)offset);
            ok = 0;
            break;
        }
        offset += n;
    }

done:
    free(chunk);

    if (!ok || offset != total) {
        send_json(sock, HTTP_500, "{\"msg\":\"Upload interrupted\"}");
        return;
    }

    if (fs_finalize(total, crc) != ESP_OK) {
        send_json(sock, HTTP_500, "{\"msg\":\"Finalize failed\"}");
        return;
    }

    os_printf("[HTTP] Firmware stored: %u bytes, CRC32 0x%08lX\n",
              (unsigned int)total, (unsigned long)crc);

    char json[128];
    snprintf(json, sizeof(json),
             "{\"msg\":\"Firmware stored\",\"size\":%u,\"crc32\":%u}",
             (unsigned int)total, (unsigned int)crc);
    send_json(sock, HTTP_200, json);
}

/* ---- Header parsing ---- */

/*
 * Read HTTP headers from sock into buf (max bufsz bytes).
 * Stops when "\r\n\r\n" is found.
 * Returns number of bytes read (including the terminator).
 * On success, *p_body points to first byte after "\r\n\r\n",
 * and *p_body_len is how many body bytes were already read past it.
 */
static int read_headers(int sock, char *buf, size_t bufsz,
                        const char **p_body, size_t *p_body_len) {
    size_t total = 0;
    *p_body = NULL;
    *p_body_len = 0;

    while (total < bufsz - 1) {
        int n = recv(sock, buf + total, bufsz - 1 - total, 0);
        if (n <= 0) return -1;   /* connection closed or error */
        total += n;
        buf[total] = '\0';

        /* Look for end-of-headers marker */
        const char *end = strstr(buf, "\r\n\r\n");
        if (end) {
            size_t hdr_end = end - buf + 4;  /* past \r\n\r\n */
            if (total > hdr_end) {
                *p_body     = buf + hdr_end;
                *p_body_len = total - hdr_end;
            }
            return 0;
        }
    }

    return -1;  /* headers too large */
}

/* Parse first line: METHOD /path HTTP/1.x */
static int parse_first_line(const char *buf, char *method, char *path) {
    const char *sp1 = strchr(buf, ' ');
    if (!sp1) return -1;
    size_t mlen = sp1 - buf;
    if (mlen >= 8) return -1;
    memcpy(method, buf, mlen);
    method[mlen] = '\0';

    const char *sp2 = strchr(sp1 + 1, ' ');
    if (!sp2) return -1;
    size_t plen = sp2 - (sp1 + 1);
    if (plen >= 128) return -1;
    memcpy(path, sp1 + 1, plen);
    path[plen] = '\0';
    return 0;
}

/* Extract Content-Length from header buffer (return -1 if missing) */
static int get_cl(const char *hdr) {
    const char *p = strstr(hdr, "Content-Length:");
    if (!p) return -1;
    return atoi(p + 15);
}

/* ---- Server task ---- */

void http_server_task(void *pvParameters) {
    char method[8];
    char path[128];
    char *hdr_buf;
    int on = 1;

    hdr_buf = (char *)malloc(HDR_BUF_SIZE);
    if (!hdr_buf) {
        os_printf("[HTTP] Failed to allocate header buffer\n");
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (listen_sock < 0) {
            os_printf("[HTTP] Socket create failed\n");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        setsockopt(listen_sock, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
        setsockopt(listen_sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));

        struct sockaddr_in addr;
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port        = htons(HTTP_SERVER_PORT);

        if (bind(listen_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
            os_printf("[HTTP] Bind failed\n");
            close(listen_sock);
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        if (listen(listen_sock, 2) != 0) {
            os_printf("[HTTP] Listen failed\n");
            close(listen_sock);
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        os_printf("[HTTP] Listening on port %d\n", HTTP_SERVER_PORT);

        while (1) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            int client = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
            if (client < 0) {
                os_printf("[HTTP] Accept failed\n");
                break;
            }

            struct timeval tv = {.tv_sec = 10, .tv_usec = 0};
            setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            /* Read headers */
            const char *body_ptr;
            size_t body_overlap;
            if (read_headers(client, hdr_buf, HDR_BUF_SIZE, &body_ptr, &body_overlap) != 0) {
                close(client);
                continue;
            }

            /* Parse request line */
            if (parse_first_line(hdr_buf, method, path) != 0) {
                send_json(client, HTTP_400, "{\"msg\":\"Bad request\"}");
                close(client);
                continue;
            }

            /* Route */
            if (strcmp(method, "GET") == 0) {
                if (strncmp(path, "/status", 7) == 0) {
                    handle_get_status(client);
                } else {
                    /* Default: serve management page */
                    send_hdr(client, HTTP_200, HTTP_CT_HTML, sizeof(INDEX_HTML) - 1);
                    send(client, INDEX_HTML, sizeof(INDEX_HTML) - 1, 0);
                }
            }
            else if (strcmp(method, "POST") == 0) {
                if (strncmp(path, "/firmware", 9) == 0) {
                    int cl = get_cl(hdr_buf);
                    handle_post_firmware_stream(client, cl,
                                                (const uint8_t *)body_ptr,
                                                body_overlap);
                }
                else if (strncmp(path, "/flash", 6) == 0) {
                    handle_post_flash(client);
                }
                else if (strncmp(path, "/erase", 6) == 0) {
                    handle_post_erase(client);
                }
                else {
                    send_json(client, HTTP_404, "{}");
                }
            }
            else {
                send_json(client, HTTP_404, "{}");
            }

            close(client);
        }

        close(listen_sock);
    }
}
