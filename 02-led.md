<!--
 * @Date: 2024-11-11
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-11-11
 * @FilePath: /1-STM32MP157/02-led.md
 * @Description: 
-->
# led

### 1. 驱动原理

Linux 下的任何外设驱动，最终都是要配置相应的硬件寄存器。
Linux 内核启动的时候会初始化 MMU，设置好内存映射，设置好以后 CPU 访问的都是虚拟地址
这里就涉及到了物理内存和虚拟内存之间的转换，需要用到
两个函数：ioremap 和 iounmap。


void __iomem *ioremap(resource_size_t res_cookie, size_t size);
#define GPIOI_MODER (0X5000A000)
static void __iomem* GPIO_MODER_PI;
GPIO_MODER_PI = ioremap(GPIOI_MODER, 4);

void iounmap (volatile void __iomem *addr)
iounmap(GPIO_MODER_PI);

I/O 内存访问函数
这里说的 I/O 是输入/输出的意思，并不是我们学习单片机的时候讲的 GPIO 引脚。这里涉
及到两个概念：I/O 端口和 I/O 内存。当外部寄存器或内存映射到 IO 空间时，称为 I/O 端口。
当外部寄存器或内存映射到内存空间时，称为 I/O 内存。但是对于 ARM 来说没有 I/O 空间这个
概念，因此 ARM 体系下只有 I/O 内存(可以直接理解为内存)。

读操作函数
读操作函数有如下几个：
示例代码 21.1.2.1 读操作函数
1 u8 readb(const volatile void __iomem *addr)
2 u16 readw(const volatile void __iomem *addr)
3 u32 readl(const volatile void __iomem *addr)

写操作函数
写操作函数有如下几个：
示例代码 21.1.2.2 写操作函数
1 void writeb(u8 value, volatile void __iomem *addr)
2 void writew(u16 value, volatile void __iomem *addr)
3 void writel(u32 value, volatile void __iomem *addr)

VCC3.3 => R16 => LED0 => A98 PI0





