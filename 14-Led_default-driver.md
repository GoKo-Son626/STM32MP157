<!--
 * @Date: 2024-12-19
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-12-22
 * @FilePath: /1-STM32MP157/14-Led_default-driver.md
 * @Description: 
-->
# LED_default-driver

### 1. Linux 内核自带 LED 驱动使能
**`make menuconfig`**
按照如下路径打开 LED 驱动配置项：

→ Device Drivers

        → LED Support (NEW_LEDS [=y])

                →LED Support for GPIO connected LEDs

### 1. 自带LED驱动简介

##### 1. 框架分析

> LED 灯驱动文件为/drivers/leds/leds-gpio.c
```c
203 static const struct of_device_id of_gpio_leds_match[] = {
204{ .compatible = "gpio-leds", },
205{},
206 };
207
......
316 static struct platform_driver gpio_led_driver = {
317.probe
= gpio_led_probe,
318.shutdown = gpio_led_shutdown,
319.driver
= {
320.name
321.of_match_table = of_gpio_leds_match,
322
= "leds-gpio",
},
323 };
324
325 module_platform_driver(gpio_led_driver);
```
第 325 行通过 module_platform_driver 函数向 Linux 内核注册 gpio_led_driver 这个 platform
驱动。

##### 2. module_platform_driver 函数简析

```c
在 Linux 内核中会大量采用 module_platform_driver 来完成向 Linux 内核注
册 platform 驱动的操作。module_platform_driver 定义在 include/linux/platform_device.h 文件中，
为一个宏，定义如下：
示例代码 36.2.2.1 module_platform_driver 函数
237 #define module_platform_driver(__platform_driver) \
238
module_driver(__platform_driver, platform_driver_register, \
239
platform_driver_unregister)
可以看出，module_platform_driver 依赖 module_driver，module_driver 也是一个宏，定义在
include/linux/device.h 文件中，内容如下：
示例代码 36.2.2.2 module_driver 函数
1898 #define module_driver(__driver, __register, __unregister, ...) \
1899 static int __init __driver##_init(void) \
1900 { \
1901
return __register(&(__driver) , ##__VA_ARGS__); \
1902 } \
1903 module_init(__driver##_init); \
1904 static void __exit __driver##_exit(void) \
1905 { \
1906
__unregister(&(__driver) , ##__VA_ARGS__); \
1907 } \
1908 module_exit(__driver##_exit);
```
module_platform_driver(gpio_led_driver)
展开以后就是：
static int __init gpio_led_driver_init(void)
{
return platform_driver_register (&(gpio_led_driver));
}
module_init(gpio_led_driver_init);
static void __exit gpio_led_driver_exit(void)
{
platform_driver_unregister (&(gpio_led_driver) );
}
module_exit(gpio_led_driver_exit);
上面的代码不就是标准的注册和删除 platform 驱动吗？因此 module_platform_driver 函数的
功能就是完成 platform 驱动的注册和删除

##### 3. gpio_led_probe 函数简析

当驱动和设备匹配以后 gpio_led_probe 函数就会执行，此函数主要是从设备树中获取 LED
灯的 GPIO 信息
```c
gpio_led_probe 函数
256 static int gpio_led_probe(struct platform_device *pdev)     
257 {
258 struct gpio_led_platform_data *pdata = dev_get_platdata(&pdev->dev);
259 struct gpio_leds_priv *priv;
260 int i, ret = 0;
261
262 if (pdata && pdata->num_leds) {
/* 非设备树方式* /
// ......
/* 获取 platform_device 信息 */
292 } else { /* 采用设备树 */
293     priv = gpio_leds_create(pdev);
294     if (IS_ERR(priv))
295             return PTR_ERR(priv); 
296 }
297
298 platform_set_drvdata(pdev, priv);
299
300 return 0;
301 }
```

如果使用设备树的话，
使用 gpio_leds_create 函数从设备树中提取设备信息，
获取到的 LED 灯 GPIO 信息保存在返回值中

### 3. 设备树节点编写

打开文档 Documentation/devicetree/bindings/leds/leds-gpio.txt，此文档详细的讲解了 Linux 自
带驱动对应的设备树节点该如何编写，我们在编写设备节点的时候要注意以下几点：
①、创建一个节点表示 LED 灯设备，比如 dtsleds，如果板子上有多个 LED 灯的话每个 LED
灯都作为 dtsleds 的子节点。
②、dtsleds 节点的 compatible 属性值一定要为“gpio-leds”
。
③、设置 label 属性，此属性为可选，每个子节点都有一个 label 属性，label 属性一般表示
LED 灯的名字，比如以颜色区分的话就是 red、green 等等。
④、每个子节点必须要设置 gpios 属性值，表示此 LED 所使用的 GPIO 引脚！
⑤、可以设置“linux,default-trigger”属性值，也就是设置 LED 灯的默认功能，查阅
Documentation/devicetree/bindings/leds/common.txt 这个文档来查看可选功能，比如：
backlight：LED 灯作为背光。
default-on：LED 灯打开。
heartbeat：LED 灯作为心跳指示灯，可以作为系统运行提示灯。
disk-activity：LED 灯作为磁盘活动指示灯。
ide-disk：LED 灯作为硬盘活动指示灯。
timer：LED 灯周期性闪烁，由定时器驱动，闪烁频率可以修改。
⑥、可以设置“default-state”属性值，可以设置为 on、off 或 keep，为 on 的时候 LED 灯默
认打开，为 off 的话 LED 灯默认关闭，为 keep 的话 LED 灯保持当前模式。
另外还有一些其他的可选属性，比如 led-sources、color、function 等属性，这些属性的用法
在 Documentation/devicetree/bindings/leds/common.txt 里面有详细的讲解，大家自行查阅。



                