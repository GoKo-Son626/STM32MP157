<!--
 * @Date: 2024-12-19
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-12-24
 * @FilePath: /1-STM32MP157/16-Input.md
 * @Description:
-->
# INPUT

> 按键、鼠标、键盘、触摸屏等都属于输入(input)设备，Linux 内核为此专门做了一个叫做 input
子系统的框架来处理输入事件。输入设备本质上还是字符设备，只是在此基础上套上了 input 框
架，用户只需要负责上报输入事件，比如按键值、坐标等信息，input 核心层负责处理这些事件

### 1. INPUT子系统

##### 1. INPUT子系统简介

input 子系统分为 input 驱动层、input 核心层、input 事件处理层，最终给用户空间提供可访问的设备节点

![struct_INPUT-son](File/images/struct_INPUT-son.png)

驱动层：输入设备的具体驱动程序，比如按键驱动程序，向内核层报告输入内容。
核心层：承上启下，为驱动层提供输入设备注册和操作接口。通知事件层对输入事件进行
处理。
事件层：主要和用户空间进行交互。

##### 2. INPUT驱动编写流程

input 核心层会向 Linux 内核注册一个字符设备
drivers/input/input.c 这个文件
input.c 就是 input 输入子系统的核心层

1. 注册input_dev
在使用 input 子系统的时候我们只需要注册一个 input 设备即可，input_dev 结构体表示 input
设备
```c
131 struct input_dev {
132 const char *name;
133 const char *phys;
134 const char *uniq;
135 struct input_id id;
136 
137 unsigned long propbit[BITS_TO_LONGS(INPUT_PROP_CNT)];
138 
139 unsigned long evbit[BITS_TO_LONGS(EV_CNT)];/* 事件类型的位图 */
140 unsigned long keybit[BITS_TO_LONGS(KEY_CNT)];/* 按键值的位图 */
141 unsigned long relbit[BITS_TO_LONGS(REL_CNT)];/* 相对坐标的位图*/
142 unsigned long absbit[BITS_TO_LONGS(ABS_CNT)];/* 绝对坐标的位图 */
143 unsigned long mscbit[BITS_TO_LONGS(MSC_CNT)];/* 杂项事件的位图 */
144 unsigned long ledbit[BITS_TO_LONGS(LED_CNT)];/*LED 相关的位图 */
145 unsigned long sndbit[BITS_TO_LONGS(SND_CNT)];/* sound 有关的位*/
146 unsigned long ffbit[BITS_TO_LONGS(FF_CNT)];/* 压力反馈的位图 */
147 unsigned long swbit[BITS_TO_LONGS(SW_CNT)];/*开关状态的位图 */
148
......
201 bool devres_managed;
202
203 ktime_t timestamp[INPUT_CLK_MAX];
204 };
```
`evbit` 表示输入事件类型，可选的事件类型定义在 include/uapi/linux/input.h 文件中
```c
38 #define EV_SYN 0x00/* 同步事件*/
39 #define EV_KEY 0x01/* 按键事件*/
40 #define EV_REL 0x02/* 相对坐标事件*/
41 #define EV_ABS 0x03/* 绝对坐标事件*/
42 #define EV_MSC 0x04/* 杂项(其他)事件*/
43 #define EV_SW 0x05/* 开关事件*/
44 #define EV_LED 0x11/* LED*/
45 #define EV_SND 0x12/* sound(声音)*/
46 #define EV_REP 0x14/* 重复事件*/
47 #define EV_FF 0x15/* 压力事件*/
48 #define EV_PWR 0x16/* 电源事件*/
49 #define EV_FF_STATUS 0x17/* 压力状态事件*/
```

编写input驱动时需 要 先 申 请 一 个 input_dev 结 构 体 变 量  , 使 用input_allocate_device 函数来申请一个 input_dev，此函数原型如下所示：
struct input_dev *input_allocate_device(void)
函数参数和返回值含义如下：
参数：无。
返回值：申请到的 input_dev。

如果要注销的 input 设备的话需要使用 input_free_device 函数来释放掉前面申请到的input_dev，input_free_device 函数原型如下：
void input_free_device(struct input_dev *dev)
函数参数和返回值含义如下：
dev：需要释放的 input_dev。
返回值：无。

申请好一个 input_dev 以后就需要初始化这个 input_dev，需要初始化的内容主要为事件类
型(evbit)和事件值(keybit)这两种。

input_dev 初始化完成以后就需要向 Linux 内核注册 input_dev
了，需要用到 input_register_device 函数，此函数原型如下：
int input_register_device(struct input_dev *dev)
函数参数和返回值含义如下：
dev：要注册的 input_dev  
返回值：0，input_dev 注册成功；负值，input_dev 注册失败。

同样的，注销 input 驱动的时候也需要使用 input_unregister_device 函数来注销掉前面注册
的 input_dev，input_unregister_device 函数原型如下：
void input_unregister_device(struct input_dev *dev)
函数参数和返回值含义如下：
dev：要注销的 input_dev  
返回值：无。

综上所述，input_dev 注册过程如下：
①、使用 input_allocate_device 函数申请一个 input_dev。

②、初始化 input_dev 的事件类型以及事件值。
③、使用 input_register_device 函数向 Linux 系统注册前面初始化好的 input_dev。
④、
卸载 input 驱动的时候需要先使用 input_unregister_device 函数注销掉注册的 input_dev，
然后使用 input_free_device 函数释放掉前面申请的 input_dev。

##### 2. 上报输入事件

向 Linux 内核注册好 input_dev 以后还不能高枕无忧的使用 input 设备，input 设备都
是具有输入功能的，但是具体是什么样的输入值 Linux 内核是不知道的，我们需要获取到具体
的输入值，或者说是输入事件，然后将输入事件上报给 Linux 内核。


常
用的事件上报 API 函数。
首先是 input_event 函数，此函数用于上报指定的事件以及对应的值，函数原型如下：
void input_event(struct input_dev
*dev,
unsigned int
type,
unsigned int
code,
int
value)
函数参数和返回值含义如下：
dev：需要上报的 input_dev。
type: 上报的事件类型，比如 EV_KEY。
code：事件码，也就是我们注册的按键值，比如 KEY_0、KEY_1 等等。
value：事件值，比如 1 表示按键按下，0 表示按键松开。
返回值：无。
input_event 函数可以上报所有的事件类型和事件值，Linux 内核也提供了其他的针对具体
事 件 的 上 报 函 数  , 这 些 函 数 其 实 都 用 到 了 input_event 函 数  
例如：
```c
static inline void input_report_key(struct input_dev *dev, unsigned int code, int value)
{
        input_event(dev, EV_KEY, code, !!value);
}
```
input_report_key 函数的本质就是 input_event 函数，如果
要上报按键事件的话还是建议大家使用 input_report_key 函数。

上报事件以后还需要使用 input_sync 函数来告诉 Linux 内核 input 子系统上报结束，
input_sync 函数本质是上报一个同步事件，此函数原型如下所示：
void input_sync(struct input_dev *dev)
函数参数和返回值含义如下：
dev：需要上报同步事件的 input_dev。
返回值：无。

上报事件参考代码：
```c
1 /* 用于按键消抖的定时器服务函数 */
2 void timer_function(unsigned long arg)
3 {
4       unsigned char value;
5
6       value = gpio_get_value(keydesc->gpio);/* 读取 IO 值*/
7       if(value == 0){/* 按下按键*/
8       /* 上报按键值 */
9               input_report_key(inputdev, KEY_0, 1);   /* 最后一个参数 1，按下 */
10              input_sync(inputdev);                   /*同步事件*/
11      } else {                        /* 按键松开*/
12              input_report_key(inputdev, KEY_0, 0); /* 最后一个参数 0，松开 */
13              input_sync(inputdev);                   /*同步事件*/
14      }
15 }
```
##### 3. input_event结构体

Linux 内核使用 input_event 这个结构体来表示所有的输入事件,input_envent 结构体定义在include/uapi/linux/input.h 文件中，结构体内容如下：
**input_event 结构体**
```c
28 struct input_event {
29 #if (__BITS_PER_LONG != 32 || !defined(__USE_TIME_BITS64))&& !defined(__KERNEL__)
30      struct timeval time;    // 也就是此事件发生的时间
31 #define input_event_sec time.tv_sec
32 #define input_event_usec time.tv_usec
33 #else
34      __kernel_ulong_t __sec;
35 #if defined(__sparc__) && defined(__arch64__)
36      unsigned int __usec;
37      unsigned int __pad;
38 #else
39      __kernel_ulong_t __usec;
40 #endif
41 #define input_event_sec __sec
42 #define input_event_usec __usec
43 #endif
44      __u16 type;
45      __u16 code;
46      __s32 value;
47 };
```

### 2. About_code:
这个驱动代码是通过 `platform_driver` 框架与平台设备（Platform Device）进行集成的，因此它的执行流程并不依赖于传统的 `__init` 函数，而是通过内核的设备驱动模型来进行管理。我们可以从以下几个关键点来理解执行的起始流程。

##### 1. 平台驱动的注册
```c
static struct platform_driver atk_key_driver = {
    .driver = {
        .name = "stm32mp1-key",
        .of_match_table = key_of_match,
    },
    .probe = atk_key_probe,
    .remove = atk_key_remove,
};

module_platform_driver(atk_key_driver);
```
- platform_driver 是一个定义了 probe 和 remove 函数的结构体，它告诉内核如何加载和卸载驱动。
- module_platform_driver(atk_key_driver) 宏会将 atk_key_driver 驱动注册到内核中，这会在内核初始化时通过 platform_driver_register 被调用。

##### 2. 内核模块加载和执行
- module_platform_driver 宏的内部实现实际上是调用了 platform_driver_register，这个函数会把驱动信息注册到内核的驱动模型中。
- 当内核发现有匹配的 platform_device 时，会触发 platform_driver 的 probe 函数。

##### 3. 设备的匹配和 probe 函数的调用
```c
static const struct of_device_id key_of_match[] = {
    {.compatible = "alientek,key"},
    {/* Sentinel */}
};
```
- of_device_id 表示设备与驱动的匹配信息，compatible 字段中的 "alientek,key" 是设备树中定义的设备标识符。
- 设备树中的设备信息与驱动程序中的匹配表（key_of_match）进行匹配，内核通过 of_match_device 查找是否有匹配的设备。
- 如果匹配成功，内核会调用 atk_key_probe 函数。

##### 4. probe 函数

probe 函数是内核在设备成功匹配后执行的函数，这个函数的作用是完成硬件设备的初始化。atk_key_probe 函数的实现如下：
```c
static int atk_key_probe(struct platform_device *pdev)
{
    int ret;
    
    ret = key_gpio_init(pdev->dev.of_node);  // 初始化GPIO和中断
    if (ret < 0)
        return ret;
        
    timer_setup(&key.timer, key_timer_function, 0);  // 初始化定时器
    
    key.idev = input_allocate_device();  // 分配输入设备
    key.idev->name = KEYINPUT_NAME;     // 设置设备名称

    key.idev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);  // 设置输入事件
    input_set_capability(key.idev, EV_KEY, KEY_0);  // 设置支持的按键类型

    ret = input_register_device(key.idev);  // 注册输入设备
    if (ret) {
        printk("register input device failed!\r\n");
        goto free_gpio;
    }
    
    return 0;

free_gpio:
    free_irq(key.irq_key, NULL);
    gpio_free(key.gpio_key);
    del_timer_sync(&key.timer);
    return -EIO;
}
```
- 在 probe 函数中，会调用 key_gpio_init 来初始化GPIO和中断。
- 然后，分配并配置输入设备 input_dev，并将其注册到内核中，以便能够向用户空间报告按键事件。

##### 5. 内核模块加载的顺序
当你加载该内核模块（比如使用 insmod 命令加载时），内核会通过 module_platform_driver(atk_key_driver) 自动调用 platform_driver_register 来注册平台驱动。
在平台设备（比如一个嵌入式板卡）与该驱动匹配时，内核会调用 atk_key_probe 函数初始化设备。
所以，驱动的执行是从内核的驱动模型匹配设备和驱动开始的。
##### 6. __init 函数的缺失
传统的内核模块代码中，我们会看到 __init 函数，它标记某些初始化操作在模块加载时执行，且在模块卸载时会被释放掉。
但是在这个驱动中，platform_driver 的注册和 probe 函数的调用就是驱动的初始化入口。所以，这里没有显式的 __init 函数。
__init 是专门用于标记内核模块的初始化部分并在模块卸载时释放内存的，而 platform_driver 的 probe 函数本身就承担了初始化的角色。
总结
这个驱动的执行是通过 platform_driver 注册和设备匹配来触发的，不需要显式的 __init 函数。
驱动加载后，通过设备树匹配找到相应的 platform_device，并触发 probe 函数的执行，这个函数会进行按键硬件的初始化。