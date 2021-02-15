<p align="center"><img src="https://user-images.githubusercontent.com/17078589/107881245-7d7d5580-6f1e-11eb-9f66-6ac589e5f95c.png"/></p>

![image](https://user-images.githubusercontent.com/17078589/107857220-05ecef00-6e68-11eb-9fa0-506b32052dba.png)


[![Build Status](https://github.com/windowsair/wireless-esp8266-dap/workflows/build/badge.svg?branch=master)](https://github.com/windowsair/wireless-esp8266-dap/actions?query=branch%3Amaster) master　
[![Build Status](https://github.com/windowsair/wireless-esp8266-dap/workflows/build/badge.svg?branch=develop)](https://github.com/windowsair/wireless-esp8266-dap/actions?query=branch%3Adevelop) develop

[![](https://img.shields.io/badge/license-MIT-green.svg?style=flat-square)](https://github.com/windowsair/wireless-esp8266-dap/LICENSE)　[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-blue.svg?style=flat-square)](https://github.com/windowsair/wireless-esp8266-dap/pulls)　[![%e2%9d%a4](https://img.shields.io/badge/made%20with-%e2%9d%a4-ff69b4.svg?style=flat-square)](https://github.com/windowsair/wireless-esp8266-dap)


## Introduce

Wireless debugging with ***only one ESP8266*** !

Realized by USBIP and CMSIS-DAP protocol stack.

> 👉 5m distance, 100kb size firmware(Hex) flash test:

<p align="center"><img src="https://user-images.githubusercontent.com/17078589/107896674-e5a95700-6f71-11eb-90f7-bf7362045537.gif"/></p>

## Feature

1. Debug Communication Mode & Debug Port
    - [x] SWD(SW-DP)
    - [x] JTAG(JTAG-DP)
    - [x] SWJ-DP

2. USB Communication Mode
    - [x] USB-HID
    - [x] WCID & WinUSB (Default)

3. Debug Trace
    - [ ] UART Serial Wire Output(SWO)
    - [ ] SWO Streaming Trace

4. More..
    - [x] Custom maximum debug clock (40MHz, SWD only)
    - [x] SWD protocol based on SPI acceleration
    - [x] ...



## Link your board

### WIFI

The default connected WIFI SSID is `DAP` , password `12345678`

You can change `WIFI_SSID` and ` WIFI_PASS` in [wifi_configuration.h](main/wifi_configuration.h)

### Debugger




| SWD            |        |
|----------------|--------|
| SWCLK          | GPIO14 |
| SWDIO          | GPIO12 |
| SWDIO_MOSI     | GPIO13 |
| LED\_CONNECTED | GPIO2  |
| LED\_RUNNING   | GPIO15 |
| TVCC           | 3V3    |
| GND            | GND    |


--------------


| JTAG               |         |
|--------------------|---------|
| TCK                | GPIO14  |
| TMS                | GPIO13  |
| TDI                | GPIO4   |
| TDO                | GPIO16  |
| nTRST \(optional\) | GPIO0\* |
| nRESET             | GPIO5   |
| LED\_CONNECTED     | GPIO2   |
| LED\_RUNNING       | GPIO15  |
| TVCC               | 3V3     |
| GND                | GND     |


In order to use SPI acceleration, you need to physically connect `SWDIO(GPIO12)` to `SWDIO_MOSI(GPIO13)`.

Here, we give a simple example for reference:

![sch](https://user-images.githubusercontent.com/17078589/107851862-f9589e80-6e47-11eb-9eca-e80760822a6a.png)

Alternatively, you can connect directly with wires as we gave at the beginning, without additional circuits.


> If you need to modify the LED or JTAG pins, please refer to the instructions in [DAP_config.h](components/DAP/config/DAP_config.h) to modify them carefully.


------


## Build And Flash

You can build locally or use Github Action to build online and then download firmware to flash.

### Build with Github Action Online

See: [Build with Github Action](https://github.com/windowsair/wireless-esp8266-dap/wiki/Build-with-Github-Action)


### General build and Flash

1. Get ESP8266 RTOS Software Development Kit

    For now, use the 3.3-rc1 version of the SDK (this is a known issue)
    See: [ESP8266_RTOS_SDK](https://github.com/espressif/ESP8266_RTOS_SDK/releases/tag/v3.3-rc1 "ESP8266_RTOS_SDK")

2. Build & Flash

    Build with ESP-IDF build system.
    More information can be found at the following link: [Build System](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html "Build System")

The following example shows a possible way to build on Windows:

```bash
# Build
python ./idf.py build
# Flash
python ./idf.py -p /dev/ttyS5 flash
```

> We also provided sample firmware quick evaluation. See [Releases](https://github.com/windowsair/wireless-esp8266-dap/releases)


## Usage

1. Get USBIP project

- Windows: [usbip-win](https://github.com/cezanne/usbip-win) .
- Linux: Distributed as part of the Linux kernel, but we have not yet tested on Linux platform, and the following instructions are all under Windows platform.

2. Start esp8266 and connect it to the device to be debugged

3. Connect it with usbip:

```bash
# HID Mode only
# for pre-compiled version on SourceForge
# or usbip old version
.\usbip.exe -D -a <your-esp8266-ip-address>  1-1

# 👉 Recommend
# HID Mode Or WinUSB Mode
# for usbip-win 0.3.0 kmdf ude
.\usbip.exe attach_ude -r <your-esp8266-ip-address> -b 1-1

```

If all goes well, you should see your device connected.

![image](https://user-images.githubusercontent.com/17078589/107849548-f903d780-6e36-11eb-846f-3eaf0c0dc089.png)


Here, we use MDK for testing:

![target](https://user-images.githubusercontent.com/17078589/73830040-eb3c6f00-483e-11ea-85ee-c40b68a836b2.png)


------


## Speed Strategy

The maximum rate of esp8266 pure IO is about 2MHz.
When you select max clock, we will take the following actions:

- `clock < 2Mhz` : Similar to the clock speed you choose.
- `2MHz <= clock < 10MHz` : Use the fastest pure IO speed.
- `clock >= 10MHz` : SPI acceleration using 40MHz clock.

> Note that the most significant speed constraint of this project is still the TCP connection speed.

## Develop

0.  Check other branches to know the latest development progress.

1. Use WinUSB Mode(enabled by default):

    change `USE_WINUSB` macor in [USBd_config.h](components/USBIP/USBd_config.h)



In this repo you can find the complete implementation of the USB protocol stack including USB-HID, WCID, WinUSB. ~~Although WinUSB-based mode currently does not work on USBIP~~ :disappointed_relieved: . They are very easy and can help you quickly build your own DAP on other hardware platforms.


Currently TCP transmission speed needs to be further improved, If you have any ideas, welcome:
- [New issues](https://github.com/windowsair/wireless-esp8266-dap/issues)
- [New pull](https://github.com/windowsair/wireless-esp8266-dap/pulls)


### Issue

2020.12.1

TCP transmission speed needs to be further improved.

2020.11.11

Winusb is now available, but it is very slow.


2020.2.4

Due to the limitation of USB-HID (I'm not sure if this is a problem with USBIP or Windows), now each URB packet can only reach 255 bytes (About 1MBps bandwidth), which has not reached the upper limit of ESP8266 transmission bandwidth.

I now have an idea to construct a Man-in-the-middle between the two to forward traffic, thereby increasing the bandwidth of each transmission.

2020.1.31

At present, the adaptation to WCID, WinUSB, etc. has all been completed. However, when transmitting data on the endpoint, we received an error message from USBIP. This is most likely a problem with the USBIP project itself.

Due to the completeness of the USBIP protocol document, we have not yet understood its role in the Bulk transmission process, which may also lead to errors in subsequent processes.

We will continue to try to make it work on USB HID. Once the USBIP problem is solved, we will immediately transfer it to work on WinUSB


------

# Credit


Credits to the following project, people and organizations:

> - https://github.com/thevoidnn/esp8266-wifi-cmsis-dap for adapter firmware based on CMSIS-DAP v1.0
> - https://github.com/ARM-software/CMSIS_5 for CMSIS
> - https://github.com/cezanne/usbip-win for usbip windows


- @HeavenSpree
- @Zy19930907
- @caiguang1997


## License
[MIT LICENSE](LICENSE)