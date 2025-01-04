# EZBOOT-APP-EXAMPLE

## 简介

本仓库包含多个基于 EZBOOT 的应用层示例工程，针对不同的芯片平台（如 CH32F20X、STM32F072、STM32F103 等）提供具体实现。

如果 MCU 使用 EZBOOT 作为其引导程序（bootloader），本仓库中的代码即为与 EZBOOT 配套的应用层程序。EZBOOT 引导至该应用程序后，通过调用 EZBOOT 提供的接口实现 OTA 功能。示例工程中展示了如何通过 Y-modem 协议传输并接收 OTA 文件，保存文件内容，并在传输完成后通过 EZBOOT 执行 OTA 升级操作。

## 演示

1. 进入project目录，找到对应MCU的工程并打开编译固件。
2. 可在ezboot/config/ezboot_config.h里修改CONFIG_OTA_VERSION的值来修改固件版本号。
3. 工程编译后，在工程的输出目录(MDK-ARM/ezboot-app-example)会内生成.ota后缀的文件，该文件就是OTA文件。
4. 打开Xshell，新建串口的会话，设置COM和波特率115200。![Snipaste_2025-01-04_18-06-37](image\Snipaste_2025-01-04_18-06-37.png)
5. 设置文件传输，默认使用Ymodem协议。![Snipaste_2025-01-04_18-07-10](image\Snipaste_2025-01-04_18-07-10.png)
6. 打开会话，将OTA文件拖到会话窗口，上传OTA文件给MCU。![Snipaste_2025-01-04_18-10-42](image\Snipaste_2025-01-04_18-10-42.png)
7. 传输完成后，MCU自动重启进入EZBOOT，开始升级，升级完成打印新版本号。![Snipaste_2025-01-04_18-11-14](image\Snipaste_2025-01-04_18-11-14.png)