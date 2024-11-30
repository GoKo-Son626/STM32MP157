<!--
 * @Date: 2024-11-22
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-11-27
 * @FilePath: /1-STM32MP157/05-pinctrl-gpio.md
 * @Description: 
-->
# pinctrl&gpio

> 0. Linux内核针对PIN的复用配置推出了pinctrl子系统，对于GPIO的电气属性配置推出了gpio子系统。

### 1. pinctrl子系统
传统的配置 pin 的方式就是直接操作相应的寄存器，
但是这种配置方式比较繁琐、而且容易出问题(比如 pin 功能冲突)。pinctrl 子系统就是为了解决
这个问题而引入的，pinctrl 子系统主要工作内容如下：
①、获取设备树中 pin 信息。
②、根据获取到的 pin 信息来设置 pin 的复用功能
③、根据获取到的 pin 信息来设置 pin 的电气特性，比如上/下拉、速度、驱动能力等。
对于我们使用者来讲，只需要在设备树里面设置好某个 pin 的相关属性即可，其他的初始
化工作均由 pinctrl 子系统来完成，pinctrl 子系统源码目录为 drivers/pinctrl。

要使用 pinctrl 子系统，我们需要在设备树里面设置 PIN 的配置信息
一般会在设备树里面创建一个节点来描述 PIN 的配置信息。打开 stm32mp151.dtsi 文件，找到一个叫做 pinctrl 的节点
```c
1814 pinctrl: pin-controller@50002000 {
1815    #address-cells = <1>;
1816    #size-cells = <1>;
1817    compatible = "st,stm32mp157-pinctrl";
1818    ranges = <0 0x50002000 0xa400>;
1819    interrupt-parent = <&exti>;
1820    st,syscfg = <&exti 0x60 0xff>;
1821    hwlocks = <&hsem 0 1>;
1822    pins-are-numbered;
......
1968 };  
ranges 属性表示 STM32MP1 的 GPIO 相关寄存器起始地址，STM32MP1 系列
芯片最多拥有 176 个通用 GPIO，分为 12 组，分别为：PA0~PA15、PB0~PB15、PC0~PC15、
PD0~PD15、PE0~PE15、PF0~PF15、PG0~PG15、PH0~PH15、PI0~PI15、PJ0~PJ15、PK0~PK7、
PZ0~PZ7。
其中 PA~PK 这 11 组 GPIO 的寄存器都在一起，起始地址为 0X50002000，终止地址为
0X5000C3FF。这个可以在《STM32MP157 参考手册》里面找到。PZ 组寄存器起始地址为
0X54004000，终止地址为 0X540043FF，所以 stm32mp151.dtsi 文件里面还有个名为“pinctrl_z”
的子节点来描述 PZ 组 IO。pinctrl 节点用来描述 PA~PK 这 11 组 IO，因此 ranges 属性中的
0x50002000 表示起始地址，0xa400 表示寄存器地址范围。
第 1819 行，interrupt-parent 属性值为“&exti”，父中断为 exti。

打开 stm32mp15-pinctrl.dtsi 文件
1 &pinctrl {
......
534 m_can1_pins_a: m-can1-0 {
535 pins1 {
536 pinmux = <STM32_PINMUX('H', 13, AF9)>; /* CAN1_TX */
537 slew-rate = <1>;
538 drive-push-pull;
539 bias-disable;
540 };
541 pins2 {
542 pinmux = <STM32_PINMUX('I', 9, AF9)>; /* CAN1_RX */
543 bias-disable;
544 };
545 };
......
代码 25.1.2.2 就是向 pinctrl 节点追加数据，不同的外设使用的 PIN 不同、其配置也不
同，因此一个萝卜一个坑，将某个外设所使用的所有 PIN 都组织在一个子节点里面。
- 每个 pincrtl 节点必须至少包含一个子节点来存放 pincrtl 相关信息，也就是 pinctrl 集，这个
集合里面存放当前外设用到哪些引脚(PIN)、这些引脚应该怎么配置、复用相关的配置、上下拉、
默认输出高电平还是低电平。一般这个存放 pincrtl 集的子节点名字是“pins”，如果某个外设用
到多种配置不同的引脚那么就需要多个 pins 子节点


 **pins 子节点里面存放外设的引脚描述信息**
pinmux 属性
此属性用来存放外设所要使用的所有 IO
pinmux = <STM32_PINMUX('H', 13, AF9)>;
这里使用 STM32_PINMUX 这宏来配置引脚和引脚的复用功能，此宏定义在
include/dt-bindings/pinctrl/stm32-pinfunc.h 文件里面，内容如下：
示例代码 25.1.2.3 STM32_PINMUX 宏定义
32 #define PIN_NO(port, line) (((port) - 'A') * 0x10 + (line))
33
34 #define STM32_PINMUX(port, line, mode) (((PIN_NO(port, line)) << 8)
| (mode))
可以看出，STM32_PINMUX 宏有三个参数，这三个参数含义如下所示：
port：表示用那一组 GPIO(例：H 表示为 GPIO 第 H 组，也就是 GPIOH)。
line：表示这组 GPIO 的第几个引脚(例：13 表示为 GPIOH_13，也就是 PH13)。
mode：表示当前引脚要做那种复用功能(例：AF9 表示为用第 9 个复用功能)
- 可同时设置，不可同时使用






































