<!--
 * @Date: 2024-12-03
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-12-03
 * @FilePath: /1-STM32MP157/08-Timer.md
 * @Description: 
-->
# Timer

### 0. 

### 1. 
在 CortexA7 内核中有个通用定时器，在《Cortex-A7 Technical ReferenceManua.pdf》的“9:Generic Timer”
章节有简单的讲解，关于这个通用定时器的详细内容，可以参考《ARM ArchitectureReference 
Manual ARMv7-A and ARMv7-R edition.pdf》的“chapter B8 The Generic Timer”章节。这个通用
定时器是可选的，按照笔者学习 FreeRTOS 和 STM32 的经验，猜测 Linux 会将这个通用定时器
作为 Linux 系统时钟源(前提是 SOC 得选配这个通用定时器)。

- Cortex-M内核使用systic做为系统时钟
- 硬件定时器和软件定时器都依靠系统定时器
- 

linux内核频率设置，图形化界面配置
```bash
make menuconfig

-> Kernel Features 
 -> Timer frequency (<choice> [=y])
```

Linux 内核使用全局变量 jiffies 来记录系统从启动以来的系统节拍数，系统启动的时候会
将 jiffies 初始化为 0，jiffies 定义在文件 include/linux/jiffies.h 中，定义如下：
示例代码 30.1.1.2 include/jiffies.h 文件代码段
80 extern u64 __cacheline_aligned_in_smp jiffies_64;
81 extern unsigned long volatile __cacheline_aligned_in_smp 
__jiffy_arch_data jiffies;
第 80 行，定义了一个 64 位的 jiffies_64。
第 81 行，定义了一个 unsigned long 类型的 32 位的 jiffies。
jiffies_64 和 jiffies 其实是同一个东西，jiffies_64 用于 64 位系统，而 jiffies 用于 32 位系统。
为了兼容不同的硬件，jiffies 其实就是 jiffies_64 的低 32 位

HZ 表示每秒的节拍数，jiffies 表示系统运行的 jiffies 节拍数，所以 jiffies/HZ 就
是系统运行时间，单位为秒。不管是 32 位还是 64 位的 jiffies，都有溢出的风险，溢出以后会重
新从 0 开始计数，相当于绕回来了，因此有些资料也将这个现象也叫做绕回。假如 HZ 为最大
值 1000 的时候，32 位的 jiffies 只需要 49.7 天就发生了绕回，对于 64 位的 jiffies 来说大概需要
5.8 亿年才能绕回，因此 jiffies_64 的绕回忽略不计。

函数 描述
time_after(unkown, known)
unkown 通常为 jiffies，known            
                                                通常是需要对比的值。 time_before(unkown, known)
time_after_eq(unkown, known)
time_before_eq(unkown, known)

Linux 内核提供了几个 jiffies 和 ms、us、ns 之间的转换函数，如表 30.1.1.2
所示：
函数 描述
int jiffies_to_msecs(const unsigned long j) 
                                                        将 jiffies 类型的参数 j 分别转换为对应的毫秒、
微秒、纳秒。 int jiffies_to_usecs(const unsigned long j)
u64 jiffies_to_nsecs(const unsigned long j)

----------------------------------------------------------------------------------
long msecs_to_jiffies(const unsigned int m)
                                                将毫秒、微秒、纳秒转换为 jiffies 类型。 
long usecs_to_jiffies(const unsigned int u)
unsigned long nsecs_to_jiffies(u64 n)

### 2. 内核定时器

1. 软件定时器不像硬件定时器一样，直接给周期值（设置期满以后的时间点）
2. 内核定时器不是周期性，到时间自动关闭，需要重新打开

Linux 内核使用 timer_list 结构
体表示内核定时器，timer_list 定义在文件 include/linux/timer.h 中，定义如下：
示例代码 30.1.2.1 timer_list 结构体
11 struct timer_list {
12 /*
13 * All fields that change during normal runtime grouped to the
14 * same cacheline
15 */
16 struct hlist_node entry;
17 unsigned long expires; /* 定时器超时时间，单位是节拍数 */
18 void (*function)(struct timer_list *);/* 定时处理函数*/
19 u32 flags; /* 标志位 */
20
21 #ifdef CONFIG_LOCKDEP
22 struct lockdep_map lockdep_map;
23 #endif
24 };

定义好定时器以后还需要通过一系列的 API 函数来初始化此定时器


### 3. 驱动编写

1. 定义一个timer_list结构体

