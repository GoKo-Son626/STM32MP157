<!--
 * @Date: 2024-11-09
 * @LastEditors: GoKo Son626
 * @LastEditTime: 2024-11-10
 * @FilePath: /1-STM32MP157/01-char_device.md
 * @Description: 
-->
# Char_device

> 0. 字符设备是 Linux 驱动中最基本的一类设备驱动，字符设备就是一个一个字节，按照字节流进行读写操作的设备，读写数据是分先后顺序的。比如我们最常见的点灯、按键、IIC、SPI，LCD 等等都是字符设备，这些设备的驱动就叫做字符设备驱动。

1. linux应用程序对驱动程序的调用：
The open().close.read().write() and so on of APP, calling the  