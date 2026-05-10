/**
 * @file http_server.h
 * @brief HTTP management server for offline programmer
 *
 * Provides:
 *   GET  /          - Management web page
 *   POST /firmware  - Upload target firmware binary
 *   GET  /status    - JSON status endpoint
 *   POST /flash     - Trigger flash sequence
 *   POST /erase     - Erase stored firmware
 */
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief FreeRTOS task entry for HTTP server.
 * Listens on HTTP_SERVER_PORT (default 8080).
 */
void http_server_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* HTTP_SERVER_H */
