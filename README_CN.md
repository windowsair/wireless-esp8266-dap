<p align="center"><b>请注意：不同语言版本的翻译可能落后于项目的原始文档。请以原始文档为准。</b></p>

<p align="center"><img src="https://user-images.githubusercontent.com/17078589/120061980-49274280-c092-11eb-9916-4965f6c48388.png"/></p>

![image](https://user-images.githubusercontent.com/17078589/107857220-05ecef00-6e68-11eb-9fa0-506b32052dba.png)

[![Build Status](https://github.com/windowsair/wireless-esp8266-dap/workflows/build/badge.svg?branch=master)](https://github.com/windowsair/wireless-esp8266-dap/actions?query=branch%3Amaster) master　
[![Build Status](https://github.com/windowsair/wireless-esp8266-dap/workflows/build/badge.svg?branch=develop)](https://github.com/windowsair/wireless-esp8266-dap/actions?query=branch%3Adevelop) develop

[![](https://img.shields.io/badge/license-MIT-green.svg?style=flat-square)](https://github.com/windowsair/wireless-esp8266-dap/LICENSE)　[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-blue.svg?style=flat-square)](https://github.com/windowsair/wireless-esp8266-dap/pulls)　[![%e2%9d%a4](https://img.shields.io/badge/made%20with-%e2%9d%a4-ff69b4.svg?style=flat-square)](https://github.com/windowsair/wireless-esp8266-dap)

## 简介

只需要**一枚ESP芯片**即可开始无线调试！通过USBIP协议栈和CMSIS-DAP协议栈实现。

> 👉在5米范围内，擦除并烧写100kb大小的固件(Hex固件) ：

<p align="center"><img src="https://user-images.githubusercontent.com/17078589/120925694-4bca0d80-c70c-11eb-91b7-ffa54770faea.gif"/></p>

----

对于Keil用户，我们现在支持[elaphureLink](https://github.com/windowsair/elaphureLink)。无需usbip即可开始您的无线调试之旅！

## 特性

1. 支持的ESP芯片
    - [x] ESP8266/8285
    - [x] ESP32
    - [x] ESP32C3
    - [x] ESP32S3

2. 支持的调试接口：
    - [x] SWD
    - [x] JTAG

3. 支持的USB通信协议：
    - [x] USB-HID
    - [x] WCID & WinUSB (默认)
4. 支持的调试跟踪器：
    - [x] TCP转发的串口

5. 其它
    - [x] 通过SPI接口加速的SWD协议（最高可达40MHz）
    - [x] 支持 [elaphureLink](https://github.com/windowsair/elaphureLink)，无需驱动的快速Keil 调试
    - [x] 支持 [elaphure-dap.js](https://github.com/windowsair/elaphure-dap.js)，网页端的 ARM Cortex-M 设备固件烧录调试
    - [x] 支持 OpenOCD/pyOCD
    - [x] ...

## 连接你的开发板

### WIFI连接

固件默认的WIFI SSID是`DAP`或者`OTA`，密码是`12345678`。

你可以在[wifi_configuration.h](main/wifi_configuration.h)文件中添加多个无线接入点。

你还可以在上面的配置文件中修改IP地址（但是我们更推荐你通过在路由器上绑定静态IP地址）。

![WIFI](https://user-images.githubusercontent.com/17078589/118365659-517e7880-b5d0-11eb-9a5b-afe43348c2ba.png)

固件中已经内置了一个mDNS服务。你可以通过`dap.local`的地址访问到设备。

> ESP8266的mDNS只支持ipv4。

![mDNS](https://user-images.githubusercontent.com/17078589/149659052-7b29533f-9660-4811-8125-f8f50490d762.png)


### 调试接口连接

<details>
<summary>ESP8266</summary>

| SWD            |        |
|----------------|--------|
| SWCLK          | GPIO14 |
| SWDIO          | GPIO13 |
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
| TVCC               | 3V3     |
| GND                | GND     |

--------------

| Other              |               |
|--------------------|---------------|
| LED\_WIFI\_STATUS  | GPIO15        |
| Tx                 | GPIO2         |
| Rx                 | GPIO3 (U0RXD) |

> Rx和Tx用于TCP转发的串口，默认不开启该功能。

</details>


<details>
<summary>ESP32</summary>

| SWD            |        |
|----------------|--------|
| SWCLK          | GPIO14 |
| SWDIO          | GPIO13 |
| TVCC           | 3V3    |
| GND            | GND    |


--------------


| JTAG               |         |
|--------------------|---------|
| TCK                | GPIO14  |
| TMS                | GPIO13  |
| TDI                | GPIO18  |
| TDO                | GPIO19  |
| nTRST \(optional\) | GPIO25  |
| nRESET             | GPIO26  |
| TVCC               | 3V3     |
| GND                | GND     |

--------------

| Other              |               |
|--------------------|---------------|
| LED\_WIFI\_STATUS  | GPIO27        |
| Tx                 | GPIO23        |
| Rx                 | GPIO22        |


> Rx和Tx用于TCP转发的串口，默认不开启该功能。


</details>


<details>
<summary>ESP32C3</summary>

| SWD            |        |
|----------------|--------|
| SWCLK          | GPIO6  |
| SWDIO          | GPIO7  |
| TVCC           | 3V3    |
| GND            | GND    |


--------------


| JTAG               |         |
|--------------------|---------|
| TCK                | GPIO6   |
| TMS                | GPIO7   |
| TDI                | GPIO9   |
| TDO                | GPIO8   |
| nTRST \(optional\) | GPIO4   |
| nRESET             | GPIO5   |
| TVCC               | 3V3     |
| GND                | GND     |

--------------

| Other              |               |
|--------------------|---------------|
| LED\_WIFI\_STATUS  | GPIO10        |
| Tx                 | GPIO19        |
| Rx                 | GPIO18        |


> Rx和Tx用于TCP转发的串口，默认不开启该功能。


</details>

<details>
<summary>ESP32S3</summary>

| SWD            |        |
|----------------|--------|
| SWCLK          | GPIO12 |
| SWDIO          | GPIO11 |
| TVCC           | 3V3    |
| GND            | GND    |


--------------


| JTAG               |        |
|--------------------|--------|
| TCK                | GPIO12 |
| TMS                | GPIO11 |
| TDI                | GPIO10 |
| TDO                | GPIO9  |
| nTRST \(optional\) | GPIO14 |
| nRESET             | GPIO13 |
| TVCC               | 3V3    |
| GND                | GND    |



----

## 硬件参考电路

目前这里仅有ESP8266的参考电路。


我们为你提供了一个简单的硬件电路例子作为参考：

![sch](https://user-images.githubusercontent.com/17078589/120953707-2a0a6e00-c780-11eb-9ad8-7221cf847974.png)

***除此之外，你也可以像我们一开始给出的那张图片直接用杜邦线连接开发板，这就不需要额外的电路。***

此外，你还可以从贡献者那里获得一个完整的硬件参考电路，详见 [circuit](circuit)文件夹。

------

## 编译固件并烧写

你可以在本地构建或使用Github Action在线构建固件，然后下载固件进行烧写。

### 使用Github Action在线构建固件

详见：[Build with Github Action](https://github.com/windowsair/wireless-esp8266-dap/wiki/Build-with-Github-Action)

### 在本地构建并烧写


<details>
<summary>ESP8266</summary>

1. 获取ESP8266 SDK

    项目中已经随附了一个SDK。请不要使用其他版本的SDK。

2. 编译和烧写

    使用ESP-IDF编译系统进行构建。
    更多的信息，请见：[Build System](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html "Build System")


下面例子展示了在Windows上完成这些任务的一种可行方法：

```bash
# 编译
python ./idf.py build
# 烧写
python ./idf.py -p /dev/ttyS5 flash
```

</details>


<details>
<summary>ESP32/ESP32C3</summary>

1. 获取esp-idf

    目前，请考虑使用esp-idf v4.4.2： https://github.com/espressif/esp-idf/releases/tag/v4.4.2

2. 编译和烧写

    使用ESP-IDF编译系统进行构建。
    更多的信息，请见：[Build System](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html "Build System")


下面例子展示了在Windows上完成这些任务的一种可行方法：

```bash
# 编译
idf.py build
# 烧写
idf.py -p /dev/ttyS5 flash
```


> 位于项目根目录的`idf.py`脚本仅适用于较老的ESP8266设备，请不要在ESP32设备上使用。

</details>


> 我们还提供了预编译固件用于快速评估。详见 [Releases](https://github.com/windowsair/wireless-esp8266-dap/releases)




## 使用

1. 获取USBIP项目

- Windows: [usbip-win](https://github.com/cezanne/usbip-win)。
- Linux：USBIP作为Linux内核的一部分发布，但我们还没有在Linux平台上测试，下面的说明都是在Windows平台下的。

2. 启动ESP8266并且把ESP8266连接到同一个WIFI下。

3. 通过USBIP连接ESP8266：

```bash
# 仅HID模式，用于SourceForge上的预编译版本或者旧的USBIP版本。
.\usbip.exe -D -a <your-esp8266-ip-address>  1-1

# 👉 推荐。HID模式或者WinUSB模式。用于usbip-win 0.3.0 kmdf ude版本。
.\usbip.exe attach_ude -r <your-esp8266-ip-address> -b 1-1
```

如果一切顺利，你应该看到你的设备被连接，如下图所示。

![image](https://user-images.githubusercontent.com/17078589/107849548-f903d780-6e36-11eb-846f-3eaf0c0dc089.png)

下面我们用keil MDK来测试：

![target](https://user-images.githubusercontent.com/17078589/73830040-eb3c6f00-483e-11ea-85ee-c40b68a836b2.png)

------

## 经常会问的问题

### Keil提示“RDDI-DAP ERROR”或“SWD/JTAG Communication Failure”

1. 检查线路连接。别忘了连接3V3引脚。
2. 检查网络连接是否稳定。


## DAP很慢或者不稳定

注意，本项目受限于周围的网络环境。如果你在电脑上使用热点进行连接，你可以尝试使用wireshark等工具对网络连接进行分析。当调试闲置时，线路上应保持静默，而正常工作时一般不会发生太多的丢包。

一些局域网广播数据包可能会造成严重影响，这些包可能由这些应用发出：
- DropBox LAN Sync
- Logitech Arx Control
- ...

对于ESP8266, 这无异于UDP洪水攻击...😰


周围的射频环境同样会造成影响，此外距离、网卡性能等也可能是需要考虑的。



## 文档

### 速度策略

单独使用ESP8266通用IO时的最大翻转速率只有大概2MHz。当你选择最大时钟时，我们需要采取以下操作：

- `clock < 2Mhz` ：与你选择的时钟速度类似。
- `2MHz <= clock < 10MHz` ：使用最快的纯IO速度。
- `clock >= 10MHz` ：使用40MHz时钟的SPI加速。

> 请注意，这个项目最重要的速度制约因素仍然是TCP连接速度。

### 对于OpenOCD用户

这个项目最初是为在Keil上运行而设计的，但现在你也可以在OpenOCD上通过它来烧录程序。

```bash
> halt
> flash write_image [erase] [unlock] filename [offset] [type]
```

> 现已支持 pyOCD

### 系统 OTA

当这个项目被更新时，你可以通过无线方式更新固件。

请访问以下网站了解OTA操作。[在线OTA](http://corsacota.surge.sh/?address=dap.local:3241)

对于大多数ESP8266设备，你不需要关心闪存的大小。然而，闪存大小设置不当可能会导致OTA失败。在这种情况下，请用`idf.py menuconfig`改变闪存大小，或者修改`sdkconfig`：

```
# 选择一个flash大小
CONFIG_ESPTOOLPY_FLASHSIZE_1MB=y
CONFIG_ESPTOOLPY_FLASHSIZE_2MB=y
CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y
CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y

# 然后设置flash大小
CONFIG_ESPTOOLPY_FLASHSIZE="2MB"
```

如果闪存大小为2MB，sdkconfig文件会看起来像这样：

```
CONFIG_ESPTOOLPY_FLASHSIZE_2MB=y
CONFIG_ESPTOOLPY_FLASHSIZE="2MB"
```

对于闪存大小为1MB的设备，如ESP8285，必须做以下修改。

```
CONFIG_PARTITION_TABLE_FILENAME="partitions_two_ota.1MB.csv"
CONFIG_ESPTOOLPY_FLASHSIZE_1MB=y
CONFIG_ESPTOOLPY_FLASHSIZE="1MB"
CONFIG_ESP8266_BOOT_COPY_APP=y
```

可以用esptool.py工具检查你使用的ESP设备闪存大小：

```bash
esptool.py -p (PORT) flash_id
```

### TCP转发的串口

该功能在TCP和Uart之间提供了一个桥梁：
```
发送数据   ->  TCP  ->  Uart TX -> 外部设备

接收数据   <-  TCP  <-  Uart Rx <- 外部设备
```

![uart_tcp_bridge](https://user-images.githubusercontent.com/17078589/150290065-05173965-8849-4452-ab7e-ec7649f46620.jpg)

当TCP连接建立后，ESP芯片将尝试解决首次发送的文本。当文本是一个有效的波特率时，转发器就会切换到该波特率。例如，发送ASCII文本`115200`会将波特率切换为115200。
由于性能原因，该功能默认不启用。你可以修改 [wifi_configuration.h](main/wifi_configuration.h) 来打开它。

### elaphure-dap.js

对于 ESP8266 ，该功能默认关闭。可以在 menuconfig 中打开它：

```
CONFIG_USE_WEBSOCKET_DAP=y
```

----

## 开发

请查看其他分支以了解最新的开发进展。我们欢迎任何形式的贡献，包括但不限于新功能、关于电路的想法和文档。

如果你有什么想法，欢迎在下面提出：
- [新的Issues](https://github.com/windowsair/wireless-esp8266-dap/issues)
- [新的pull request](https://github.com/windowsair/wireless-esp8266-dap/pulls)


# 致谢

归功于以下项目、人员和组织。

> - https://github.com/thevoidnn/esp8266-wifi-cmsis-dap for adapter firmware based on CMSIS-DAP v1.0
> - https://github.com/ARM-software/CMSIS_5 for CMSIS
> - https://github.com/cezanne/usbip-win for usbip windows


- [@HeavenSpree](https://www.github.com/HeavenSpree)
- [@Zy19930907](https://www.github.com/Zy19930907)
- [@caiguang1997](https://www.github.com/caiguang1997)
- [@ZhuYanzhen1](https://www.github.com/ZhuYanzhen1)


## 许可证
[MIT 许可证](LICENSE)
