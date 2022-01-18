<p align="center"><img src="https://user-images.githubusercontent.com/17078589/120061980-49274280-c092-11eb-9916-4965f6c48388.png"/></p>

![image](https://user-images.githubusercontent.com/17078589/107857220-05ecef00-6e68-11eb-9fa0-506b32052dba.png)


[![Build Status](https://github.com/windowsair/wireless-esp8266-dap/workflows/build/badge.svg?branch=master)](https://github.com/windowsair/wireless-esp8266-dap/actions?query=branch%3Amaster) master　
[![Build Status](https://github.com/windowsair/wireless-esp8266-dap/workflows/build/badge.svg?branch=develop)](https://github.com/windowsair/wireless-esp8266-dap/actions?query=branch%3Adevelop) develop

[![](https://img.shields.io/badge/license-MIT-green.svg?style=flat-square)](https://github.com/windowsair/wireless-esp8266-dap/LICENSE)　[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-blue.svg?style=flat-square)](https://github.com/windowsair/wireless-esp8266-dap/pulls)　[![%e2%9d%a4](https://img.shields.io/badge/made%20with-%e2%9d%a4-ff69b4.svg?style=flat-square)](https://github.com/windowsair/wireless-esp8266-dap)

## 简介

只需要**一枚ESP8266**即可实现的无线调试器！通过USBIP协议栈和CMSIS-DAP协议栈实现。

> 👉在5M速度范围下，擦除并下载100kb大小的固件(Hex固件) 实测图：

<p align="center"><img src="https://user-images.githubusercontent.com/17078589/120925694-4bca0d80-c70c-11eb-91b7-ffa54770faea.gif"/></p>

## 特性

1. 支持的调试协议模式和调试接口：
    - [x] SWD(SW-DP)
    - [x] JTAG(JTAG-DP)
    - [x] SWJ-DP

2. 支持的USB通信协议：
    - [x] USB-HID
    - [x] WCID & WinUSB (Default)

3. 支持的调试跟踪器：
    - [ ] UART Serial Wire Output(SWO)
    - [ ] SWO Streaming Trace

4. 其它
    - [x] 通过SPI接口加速的SWD协议
    - [x] ...

## 连接你的开发板

### WIFI连接
&nbsp;&nbsp;&nbsp;&nbsp;固件默认的WIFI SSID是`DAP`，密码是`12345678`。你可以在[wifi_configuration.h](main/wifi_configuration.h)文件中修改`WIFI_SSID`和` WIFI_PASS`字段来修改ESP8266连接的目标WIFI。你还可以在上面的配置文件中修改IP地址（但是我们更推荐你通过在路由器上绑定静态IP地址）。

![WIFI](https://user-images.githubusercontent.com/17078589/118365659-517e7880-b5d0-11eb-9a5b-afe43348c2ba.png)

### 调试接口连接

| SWD接口        | ESP8266引脚 |
|----------------|--------|
| SWCLK          | GPIO14 |
| SWDIO          | GPIO13 |
| LED\_CONNECTED | GPIO2  |
| LED\_RUNNING   | GPIO15 |
| TVCC           | 3V3    |
| GND            | GND    |

--------------

|      JTAG接口      | ESP8266引脚 |
| :----------------: | :---------: |
|        TCK         |   GPIO14    |
|        TMS         |   GPIO13    |
|        TDI         |    GPIO4    |
|        TDO         |   GPIO16    |
| nTRST \(optional\) |   GPIO0\*   |
|       nRESET       |    GPIO5    |
|   LED\_CONNECTED   |    GPIO2    |
|    LED\_RUNNING    |   GPIO15    |
|        TVCC        |     3V3     |
|        GND         |     GND     |

----

## 硬件参考电路
我们为你提供了一个简单的硬件电路例子作为参考：

![sch](https://user-images.githubusercontent.com/17078589/120953707-2a0a6e00-c780-11eb-9ad8-7221cf847974.png)

***除此之外，你也可以像我们一开始给出的那张图片直接用杜邦线连接开发板，这就不需要额外的电路。***

> 如果你想修改接口的引脚或者是LED的引脚，请小心地参考指示在[DAP_config.h](components/DAP/config/DAP_config.h)文件中修改。

此外，你还可以从贡献者那里获得一个完整的硬件参考电路，详见 [circuit](circuit)文件夹。

------

## 编译固件并烧写

你可以在本地构建或使用Github Action在线构建固件，然后下载固件进行烧写。

### 使用Github Action在线构建固件

详见：[Build with Github Action](https://github.com/windowsair/wireless-esp8266-dap/wiki/Build-with-Github-Action)

### 在本地构建并烧写

下面展示的是在Ubuntu20.04中使用终端安装依赖软件并编译烧录固件的操作流程。
1. 下载本仓库到本地：
   ```bash
   $ sudo apt update && sudo apt install -y cmake git
   $ git clone https://github.com/windowsair/wireless-esp8266-dap.git && cd wireless-esp8266-dap*
   ```
2. 获取ESP8266 RTOS软件开发套件：
   ```bash
   $ git submodule init && git submodule update && cd ESP8266_RTOS_SDK
   ```
3. 安装ESP8266 RTOS软件开发套件：
   ```bash
   $ sudo apt install -y python3 python3-pip
   $ ./install.sh && . ./export.sh && cd ..
   ```
4. 编译并烧录固件：
   ```bash
   # 编译固件
   $ python3 ./idf.py build
   # 烧录固件，“/dev/ttyS5”需要改成你对应的串口
   $ python3 ./idf.py -p /dev/ttyS5 flash
   ```

> 我们也提供了已经编译好的固件用于快速评估。在[Releases](https://github.com/windowsair/wireless-esp8266-dap/releases)中查看详细信息。

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

## 速度性能

单独使用ESP8266通用IO时的最大翻转速率只有大概2MHz。当你选择最大时钟时，我们需要采取以下操作：

- `clock < 2Mhz` ：与你选择的时钟速度类似。
- `2MHz <= clock < 10MHz` ：使用最快的纯IO速度。
- `clock >= 10MHz` ：使用40MHz时钟的SPI加速。

> 请注意，这个项目最重要的速度制约因素仍然是TCP连接速度。

## 开发

1. 检查其他分支以了解最新的开发进展。
2. 使用WinUSB模式（默认启用）。
在[dap_configuration.h](main/dap_configuration.h)修改 `USE_WINUSB` 宏。

目前TCP传输速度还需要进一步提高，如果你有什么想法，欢迎在下面提出：
- [新的Issues](https://github.com/windowsair/wireless-esp8266-dap/issues)
- [新的pull request](https://github.com/windowsair/wireless-esp8266-dap/pulls)

### 版本修订信息

2020.12.1

TCP传输速度需要进一步提高。

2020.11.11

现在可以使用Winusb了，但它非常慢。

2020.2.4

由于USB-HID的限制（不知道是USBIP的问题还是Windows的问题），现在每个URB数据包只能达到255字节（约1MBps带宽），还没有达到ESP8266传输带宽的上限。

我现在有一个想法，在两者之间构建一个中间人来转发流量，从而增加每次传输的带宽。

2020.1.31

目前，对WCID、WinUSB等的适配已经全部完成。然而，当在端点上传输数据时，我们收到了来自USBIP的错误信息。这很可能是USBIP项目本身的问题。

由于USBIP协议文件的完整性，我们还没有理解它在批量传输过程中的作用，这也可能导致后续过程中的错误。

我们将继续努力使其在USB HID上发挥作用。一旦USBIP的问题得到解决，我们将立即将其转移到WinUSB上工作

------

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
