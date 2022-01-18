<p align="center"><img src="https://user-images.githubusercontent.com/17078589/120061980-49274280-c092-11eb-9916-4965f6c48388.png"/></p>

![image](https://user-images.githubusercontent.com/17078589/107857220-05ecef00-6e68-11eb-9fa0-506b32052dba.png)


[![Build Status](https://github.com/windowsair/wireless-esp8266-dap/workflows/build/badge.svg?branch=master)](https://github.com/windowsair/wireless-esp8266-dap/actions?query=branch%3Amaster) master　
[![Build Status](https://github.com/windowsair/wireless-esp8266-dap/workflows/build/badge.svg?branch=develop)](https://github.com/windowsair/wireless-esp8266-dap/actions?query=branch%3Adevelop) develop

[![](https://img.shields.io/badge/license-MIT-green.svg?style=flat-square)](https://github.com/windowsair/wireless-esp8266-dap/LICENSE)　[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-blue.svg?style=flat-square)](https://github.com/windowsair/wireless-esp8266-dap/pulls)　[![%e2%9d%a4](https://img.shields.io/badge/made%20with-%e2%9d%a4-ff69b4.svg?style=flat-square)](https://github.com/windowsair/wireless-esp8266-dap)


## Introduce

Wireless debugging with ***only one ESP8266*** !

Realized by USBIP and CMSIS-DAP protocol stack.

> 👉 5m range, 100kb size firmware(Hex) earse and download test:

<p align="center"><img src="https://user-images.githubusercontent.com/17078589/120925694-4bca0d80-c70c-11eb-91b7-ffa54770faea.gif"/></p>

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
    - [x] SWD protocol based on SPI acceleration
    - [x] ...



## Link your board

### WIFI

The default connected WIFI SSID is `DAP` , password `12345678`

You can change `WIFI_SSID` and ` WIFI_PASS` in [wifi_configuration.h](main/wifi_configuration.h)

You can also specify your IP in the above file (We recommend using the static address binding feature of the router).

![WIFI](https://user-images.githubusercontent.com/17078589/118365659-517e7880-b5d0-11eb-9a5b-afe43348c2ba.png)


There is built-in ipv4 only mDNS server. You can access the device using `dap.local` .

![mDNS](https://user-images.githubusercontent.com/17078589/149659052-7b29533f-9660-4811-8125-f8f50490d762.png)



### Debugger


| SWD            |        |
|----------------|--------|
| SWCLK          | GPIO14 |
| SWDIO          | GPIO13 |
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


----

## Hardware Reference

Here we provide a simple example for reference:

![sch](https://user-images.githubusercontent.com/17078589/120953707-2a0a6e00-c780-11eb-9ad8-7221cf847974.png)

***Alternatively, you can connect directly with wires as we gave at the beginning, without additional circuits.***



> If you want to modify the LED or JTAG pins, please refer to the instructions in [DAP_config.h](components/DAP/config/DAP_config.h) to modify them carefully.


In addition, a complete hardware reference design is available from contributors, see [circuit](circuit)

------


## Build And Flash

You can build locally or use Github Action to build online and then download firmware to flash.

### Build with Github Action Online

See: [Build with Github Action](https://github.com/windowsair/wireless-esp8266-dap/wiki/Build-with-Github-Action)


### General build and Flash

1. Get ESP8266 RTOS Software Development Kit

    The SDK is already included in the project, please use it for subsequent operations.

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

## Document

### Speed Strategy

The maximum rate of esp8266 pure IO is about 2MHz.
When you select max clock, we will take the following actions:

- `clock < 2Mhz` : Similar to the clock speed you choose.
- `2MHz <= clock < 10MHz` : Use the fastest pure IO speed.
- `clock >= 10MHz` : SPI acceleration using 40MHz clock.

> Note that the most significant speed constraint of this project is still the TCP connection speed.


### For OpenOCD user

This project was originally designed to run on Keil, but now you can also perform firmware flash on OpenOCD.

Note that if you want to use a 40MHz SPI acceleration, you need to specify the speed after the target device is connected, otherwise it will fail with the beginning.

```bash
# Run before approaching the flash command
> adapter speed 10000

# > halt
# > flash write_image [erase] [unlock] filename [offset] [type]
```

> Keil's timing handling is somewhat different from OpenOCD's. For example, OpenOCD lacks the SWD line reset sequence before reading the `IDCODE` registers.


### System OTA

When this project is updated, you can update the firmware over the air.

Visit the following website for OTA operations: [online OTA](http://corsacota.surge.sh/?address=dap.local:3241)


For most ESP8266 devices, you don't need to care about flash size. However, improper setting of the flash size may cause the OTA to fail. In this case, you can change the flash size with `idf.py menuconfig`, or you can modify `sdkconfig`:

```
# Choose a flash size.
CONFIG_ESPTOOLPY_FLASHSIZE_1MB=y
CONFIG_ESPTOOLPY_FLASHSIZE_2MB=y
CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y
CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y

# Then set a flash size
CONFIG_ESPTOOLPY_FLASHSIZE="2MB"
```

If flash size is 2MB, the sdkconfig file might look like this:

```
CONFIG_ESPTOOLPY_FLASHSIZE_2MB=y
CONFIG_ESPTOOLPY_FLASHSIZE="2MB"
```


For devices with 1MB flash size such as ESP8285, the following changes must be made:

```
CONFIG_PARTITION_TABLE_FILENAME="partitions_two_ota.1MB.csv"
CONFIG_ESPTOOLPY_FLASHSIZE_1MB=y
CONFIG_ESPTOOLPY_FLASHSIZE="1MB"
CONFIG_ESP8266_BOOT_COPY_APP=y
```

The flash size of the board can be checked with the esptool.py tool:

```bash
esptool.py -p (PORT) flash_id
```

----

## Develop

0.  Check other branches to know the latest development progress.

1. Use WinUSB Mode(enabled by default):

    change `USE_WINUSB` macor in [dap_configuration.h](main/dap_configuration.h)



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

## Credit


Credits to the following project, people and organizations:

> - https://github.com/thevoidnn/esp8266-wifi-cmsis-dap for adapter firmware based on CMSIS-DAP v1.0
> - https://github.com/ARM-software/CMSIS_5 for CMSIS
> - https://github.com/cezanne/usbip-win for usbip windows


- [@HeavenSpree](https://www.github.com/HeavenSpree)
- [@Zy19930907](https://www.github.com/Zy19930907)
- [@caiguang1997](https://www.github.com/caiguang1997)
- [@ZhuYanzhen1](https://www.github.com/ZhuYanzhen1)


## License
[MIT LICENSE](LICENSE)