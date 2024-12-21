<!--
 * @Date: 2024-12-19
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-12-19
 * @FilePath: /1-STM32MP157/13-platform_device.md
 * @Description: 
-->
# platform_device

> 最新的 Linux 内核已经支持了设备树，因此在设备树下如何编写 platform驱动就显得尤为重要

### 1. 设备树下platform驱动简介

##### 1. 修改pinctrl-stm32.c文件

在使用设备树的时候，设备的描述被放到了设备树中，因此 platform_device 就
不需要我们去编写了，我们只需要实现 platform_driver 即可。

我们讲解了 pinctrl，但是在后续的实验中却一直没有使用pinctrl。按道理来讲，在我们使用某个引脚的时候需要先配置其电气属性，比如复用、输入还是输入、默认上下拉等！但是在前面的实验中均没有配置引脚的电气属性，也就是引脚的 pinctrl配置。这是因为 ST 针对 STM32MP1 提供的 Linux 系统中，其 pinctrl 配置的电气属性只能在platform 平台下被引用，前面的实验都没用到 platform，所以 pinctrl 配置是不起作用的！

对于 STM32MP1 来说，在使用 pinctrl 的时候需要修改一下 pinctrl-stm32.c 这个文件，否则
当某个引脚用作 GPIO 的时候会提示此引脚无法申请到
打开pinctrl-stm32.c 这个文件，找到如下所示代码：stm32_pmx_ops 结构体
```c
1 static const struct pinmux_ops stm32_pmx_ops = {
2 .get_functions_count = stm32_pmx_get_funcs_cnt,
3 .get_function_name = stm32_pmx_get_func_name,
4 .get_function_groups = stm32_pmx_get_func_groups,
5 .set_mux = stm32_pmx_set_mux,
6 .gpio_set_direction = stm32_pmx_gpio_set_direction,
7 .strict = true,
8 };
```
第 7 行的 strict 成员变量默认为 true，我们需要将其改为 false

##### 2. 创建设备的pinctrl节点

打开 stm32mp15-pinctrl.dtsi 文件，
STM32MP1 的所有引脚 pinctrl 配置都是在这个文件里面完成的，在 pinctrl 节点下添加
```c
1 led_pins_a: gpioled-0 {
2       pins {
3             pinmux = <STM32_PINMUX('I', 0, GPIO)>;
4             drive-push-pull;
5             bias-pull-up;
6             output-high;
7             slew-rate = <0>;
8       };
9 };
```
第 3 行，设置 PI0 复用为 GPIO 功能。
第 4 行，设置 PI0 为推挽输出。
第 5 行，设置 PI0 内部上拉。
第 6 行，设置 PI0 默认输出高电平。
第 7 行，设置 PI0 的速度为 0 档，也就是最慢。

##### 3. 在设备树中创建设备节点

```c
1 gpioled {
2       compatible = "alientek,led";
3       pinctrl-names = "default";
4       status = "okay";
5       pinctrl-0 = <&led_pins_a>;
6       led-gpio = <&gpioi 0 GPIO_ACTIVE_LOW>;
7 };
```
##### 4. 编写 platform 驱动的时候要注意兼容属性
上一章已经详细的讲解过了，在使用设备树的时候 platform 驱动会通过 of_match_table 来
保存兼容性值，也就是表明此驱动兼容哪些设备。所以，of_match_table 将会尤为重要，比如本
例程的 platform 驱动中 platform_driver 就可以按照如下所示设置：
示例代码 35.1.4.1 of_match_table 匹配表的设置
1
static const struct of_device_id led_of_match[] = {
2
3
4
{ .compatible = "alientek,led" },
{ /* Sentinel */ }
/* 兼容属性 */
};
5
6
MODULE_DEVICE_TABLE(of, led_of_match);
7
8
static struct platform_driver led_platform_driver = {
9.driver = {
10.name
11.of_match_table = led_of_match,
= "stm32mp1-led",
12},13.probe= led_probe,
14.remove= led_remove,
15 };
第 1~4 行，of_device_id 表，
也就是驱动的兼容表，是一个数组，每个数组元素为 of_device_id
类型。每个数组元素都是一个兼容属性，表示兼容的设备，一个驱动可以跟多个设备匹配。这
里我们仅仅匹配了一个设备，那就是示例代码 35.1.2 中创建的 gpioled 这个设备。第 2 行的
compatible 值为“alientek,led”
，驱动中的 compatible 属性和设备中的 compatible 属性相匹配，
因此驱动中对应的 probe 函数就会执行。注意第 3 行是一个空元素，**在编写 of_device_id 的时候最后一个元素一定要为空！**
第 6 行，通过 MODULE_DEVICE_TABLE 声明一下 led_of_match 这个设备匹配表。
第 11 行，设置 platform_driver 中的 of_match_table 匹配表为上面创建的 leds_of_match，至
此我们就设置好了 platform 驱动的匹配表了。

最后就是编写驱动程序，基于设备树的 platform 驱动和上一章无设备树的 platform 驱动基
本一样，都是当驱动和设备匹配成功以后先根据设备树里的 pinctrl 属性设置 PIN 的电气特性再
去执行 probe 函数。我们需要在 probe 函数里面执行字符设备驱动那一套，当注销驱动模块的
时候 remove 函数就会执行，都是大同小异的。

### 2. 检查引脚复用配置

##### 1. 检查引脚 pinctrl 配置

正点原子 STM32MP1 开发板上将 PI0 连接到了 LED0 上，也就是将其用作普通的 GPIO，
对应的 pinctrl 配置就是示例代码 35.1.2.1。但是 stm32mp15-pinctrl.dtsi 是 ST 根据自己官方 EVK
开发板编写的，因此 PI0 就可能被 ST 官方用作其他功能

##### 2. 检查 GPIO 占用

因为我们是在 ST 官方提供的设备树上修改的，因此还要检查一下当 PI0 作为 GPIO 的
时候，ST 官方有没有将这个 GPIO 分配给其他设备。其实对于 PI0 这个引脚来说不会的，因为
ST 官方将其复用为了 LCD_G5，所以也就不存在说将其在作为 GPIO 分配给其他设备。但是我
们在实际开发中要考虑到这一点，说不定其他的引脚就会被分配给某个设备做 GPIO，而我们没
有检查，导致两个设备都用这一个 GPIO，那么肯定有一个因为申请不到 GPIO 而导致驱动无法
工作。



