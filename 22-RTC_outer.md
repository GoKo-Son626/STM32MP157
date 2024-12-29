<!--
 * @Date: 2024-12-27
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-12-27
 * @FilePath: /1-STM32MP157/22-RTC_outer.md
 * @Description: 
-->
# RTC_outer

> 上一章我们学习了 STM32MP1 内置 RTC 外设，了解了 Linux 系统下 RTC 驱动框架。一般的应用场合使用 SOC 内置的 RTC 就可以了，而且成本也低，但是在一些对于时间精度要求比较高的场合，SOC 内置的 RTC 就不适用了。这个时候我们需要根据自己的应用要求选择合适的外置 RTC 芯片，正点原子 STM32MP1 开发板上板载了一个 RTC 芯片：PCF8563，这是一个IIC 接口的外置 RTC 芯片，

### 1. PCF8563

1. 简介
- PCF8563 是一个 CMOS RTC 芯片，支持时间和日历功能，支持可编程的时钟输出、中断输出以及低电压检测。PCF8563 提供了两线 IIC 接口来传输时间信息，最大传输速度为 400Kbit/S，在读写寄存器的时候地址自增，PCF8563 相关特性如下：
- 提供年、月、日、星期，时、分、秒计时，使用外置 32.768Khz 晶振。
- 低后备电流：0.25uA ，VDD=3.0V，温度 25℃。
- IIC 接口，速度最高 400KHz。
- 可编程时钟输出，可以供其他设备使用，可输出的时钟频率有 32.768kHz、1.024kHz、32Hz 和 1Hz。
- 支持闹钟和定时功能。
- IIC 读地址为 0XA3，写地址为 0XA2，也就是 IIC 器件地址为：0X51。
- 有一个开漏输出的中断引脚。

[PCF3568框图]
- 这是 PCF8563 的 32.768kHz 晶振引脚，PCF8563 必选要外接 32.768kHz 晶振。
- 这是 PCF8563 的 IIC 引脚，PCF8563 通过 IIC 接口与主控进行通信，因此 PCF8563本质是个 IIC 器件。
- 时钟输出引脚。
- 中断引脚。
- 前面说了，PCF8563 是个 IIC 器件，因此内部就有很多寄存器来实现 RTC 功能，比如配置芯片，读取时间信息等。这部分就是 PCF8563 的内部寄存器，稍后我们会详细分析PCF8563 内部寄存器。

2. 寄存器详解

- PCF8563 有 16 个内部寄存器，这些寄存器都是 8 位的。前两个寄存器(0x00 和 0x01)为控
制/状态寄存器。0X02~0X08 为时间和日期寄存器，这些寄存器保存着秒、分、时、日、星期、
月和年信息。0X09~0X0C 为闹钟寄存器，保存闹钟信息。0X0D 为时钟输出频率寄存器，0X0E
和 0X0F 这两个寄存器时钟控制寄存器。注意、时分秒、年月日、闹钟等时间信息为 BCD 格式。

**寄存器详见参考手册**

总体来说，PCF8563 还是很简单的，这是一个 IIC 接口的 RTC 芯片，因此在 Linux 系统下
就涉及到两类驱动：
①、IIC 驱动，需要 IIC 驱动框架来读写 PCF8563 芯片。
②、RTC 驱动，因为这是一个 RTC 芯片，因此要用到 RTC 驱动框架。
如果要用到中断功能的话，还需要用到 Linux 系统中的中断子系统，这些我们前面都有相
应的实验讲解。所以 PCF8563 的 Linux 驱动并不复杂，而且重点是 Linux 系统默认就已经集成
了 PCF8563 驱动，我们使用起来非常简单，直接修改设备树，添加 PCF8563 节点信息，然后使
能内核的 PCF8563 驱动即可。

### 2. 硬件原理图分析

[PCF8563原理图]
从图 44.2.1 可以看出，PCF8563 连接到了 STM32MP157 的 I2C4 接口上，引脚为 PZ5、PZ4。
另外，PCF8563 的 INT 引脚连接到了 STM32MP157 的 PI3 引脚上

### 3. 实验驱动编写

1. 修改设备树
- 添加或者查找 PCF8563 所使用的 IO 的 pinmux 配置
```c
&i2c4_pins_a: i2c4-0 {
    pins {
        pinmux = <STM32_PINMUX('Z', 4, AF6)>, /* I2C4_SCL */
                <STM32_PINMUX('Z', 5, AF6)>; /* I2C4_SDA */
        bias-disable;
        drive-open-drain;
        slew-rate = <0>;
    };
};

&i2c4_pins_sleep_a: i2c4-1 {
    pins {
        pinmux = <STM32_PINMUX('Z', 4, ANALOG)>, /* I2C4_SCL (Sleep mode) */
                <STM32_PINMUX('Z', 5, ANALOG)>; /* I2C4_SDA (Sleep mode) */
    };
};
```
- 在 I2C4 节点下添加 pinmux 并追加 pcf8563 子节点
```c
&i2c4 {
    pinctrl-names = "default", "sleep";
    pinctrl-0 = <&i2c4_pins_a>;  /* Active mode pinmux */
    pinctrl-1 = <&i2c4_pins_sleep_a>;  /* Sleep mode pinmux */
    status = "okay";  /* Enable I2C4 controller */

    pcf8563@51 {
        compatible = "nxp,pcf8563";  /* Device-specific compatible string */
        irq_gpio = <&gpioi 3 IRQ_TYPE_EDGE_FALLING>;  /* IRQ configuration for falling edge */
        reg = <0x51>;  /* I2C address of the device */
    };
};
```
2. PCF8563驱动使能

- 关闭 STM32MP157 内置 RTC 驱动
> -> Device Drivers
>       -> Real Time Clock
>               -> STM32 RTC            //取消选中
- 使能Linux内核自带的PCF8563驱动
> -> Device Drivers
>       -> Real Time Clock
>               -> STM32 RTC            //取消选中
