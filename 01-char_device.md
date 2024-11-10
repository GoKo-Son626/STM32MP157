<!--
 * @Date: 2024-11-09
 * @LastEditors: GoKo Son626
 * @LastEditTime: 2024-11-10
 * @FilePath: /1-STM32MP157/01-char_device.md
 * @Description: 
-->
# Char_device

### 1. intro of Char_device

> 0. The character device is the most basic kind of device driver in the Linux driver. The character device is a device which reads and writes data according to the byte stream, one by one. For example, our most common lighting, keys, IIC, SPI, LCD, etc..., are character devices, and the drivers of these devices are called character device drivers.

1. The process of linux application calls to drivers:
- The open().close().read().write() and so on of APP, calling the library function of library, and then into the core through the system call, finally actuating the open().close().read().write() and so on of driver to driver specific hardware devices.

2. 在 Linux 中一切皆为文件，驱动加载成功以后会在“/dev”目录下生成一个相应的文件，应
用程序通过对这个名为“/dev/xxx”(xxx 是具体的驱动文件名字)的文件进行相应的操作即可实
现对硬件的操作。

应用程序运行在用户空间，而 Linux 驱动属于内核的一部分，因此驱动运行于内核空间。
当我们在用户空间想要实现对内核的操作，比如使用 open 函数打开/dev/led 这个驱动，因为用
户空间不能直接对内核进行操作，因此必须使用一个叫做“系统调用”的方法来实现从用户空
间“陷入”到内核空间，

在 Linux 内核文件 include/linux/fs.h 中
有个叫做 file_operations 的结构体，此结构体就是 Linux 内核驱动操作函数集合，

### 2. Development procedure of char_device
 
1. Linux 驱动有两种运行方式，第一种就是将驱动编译进 Linux 内核中，这样当 Linux 内核启
动的时候就会自动运行驱动程序。第二种就是将驱动编译成模块(Linux 下模块扩展名为.ko)，在
Linux 内核启动以后使用“modprobe”或者“insmod”命令加载驱动模块，

insmod 命令不能解决模块的依赖关系 modprobe 会分析模块的依赖关系，然后会将所有的依赖模块都加载到内核中


驱动模块的卸载使用命令“rmmod”即可，比如要卸载 drv.ko，使用如下命令即可：
rmmod drv.ko
也可以使用“modprobe -r”命令卸载驱动，比如要卸载 drv.ko，命令如下：
modprobe -r drv
使用 modprobe 命令可以卸载掉驱动模块所依赖的其他模块，前提是这些依赖模块已经没
有被其他模块所使用，否则就不能使用 modprobe 来卸载驱动模块

本教程我们统一使用
“modprobe”命令
module_init(xxx_init); //注册模块加载函数
module_exit(xxx_exit); //注册模块卸载函数
```c
/* 驱动入口函数 */
2 static int __init xxx_init(void)
3 {
4 /* 入口函数具体内容 */
5 return 0;
6 }
7 
8 /* 驱动出口函数 */
9 static void __exit xxx_exit(void)
10 {
11 /* 出口函数具体内容 */
12 }
13
14 /* 将上面两个函数指定为驱动的入口和出口函数 */
15 module_init(xxx_init);
16 module_exit(xxx_exit);
```

对于字符设备驱动而言，当驱动模块加载成功以后需要注册字符设备，同样，卸载驱动模
块的时候也需要注销掉字符设备。字符设备的注册和注销函数原型如下所示:
static inline int register_chrdev(unsigned int major, 
                                        const char *name,
                                        const struct file_operations *fops)
static inline void unregister_chrdev(unsigned int major, 
                                        const char *name)


一般字符设备的注册在驱动模块的入口函数 xxx_init 中进行，字符设备的注销在驱动模块
的出口函数 xxx_exit 中进行。

实现设备的具体操作函数：
```c
static struct file_operations test_fops;
2
3 /* 驱动入口函数 */
4 static int __init xxx_init(void)
5 {
6 /* 入口函数具体内容 */
7 int retvalue = 0;
8
9 /* 注册字符设备驱动 */
10 retvalue = register_chrdev(200, "chrtest", &test_fops);
11 if(retvalue < 0){
12 /* 字符设备注册失败,自行处理 */
13 }
14 return 0;
15 }
16
17 /* 驱动出口函数 */
18 static void __exit xxx_exit(void)
19 {
20 /* 注销字符设备驱动 */
21 unregister_chrdev(200, "chrtest");
22 }
23
24 /* 将上面两个函数指定为驱动的入口和出口函数 */
25 module_init(xxx_init);
26 module_exit(xxx_exit);
```
"cat /proc/devices" => 查看当前设备号

```c
/* 打开设备 */
2 static int chrtest_open(struct inode *inode, struct file *filp)
3 {
4 /* 用户实现具体功能 */
5 return 0;
6 }
7 
8 /* 从设备读取 */
9 static ssize_t chrtest_read(struct file *filp, char __user *buf, 
size_t cnt, loff_t *offt)
10 {
11 /* 用户实现具体功能 */
12 return 0;
13 }
14
15 /* 向设备写数据 */
16 static ssize_t chrtest_write(struct file *filp,
const char __user *buf,
size_t cnt, loff_t *offt)
17 {
18 /* 用户实现具体功能 */
19 return 0;
20 }
21
22 /* 关闭/释放设备 */
23 static int chrtest_release(struct inode *inode, struct file *filp)
24 {
25 /* 用户实现具体功能 */
26 return 0;
27 }
28
29 static struct file_operations test_fops = {
30 .owner = THIS_MODULE, 
31 .open = chrtest_open,
32 .read = chrtest_read,
33 .write = chrtest_write,
34 .release = chrtest_release,
35 };
36
37 /* 驱动入口函数 */
38 static int __init xxx_init(void)
39 {
40 /* 入口函数具体内容 */
41 int retvalue = 0;
42
43 /* 注册字符设备驱动 */
44 retvalue = register_chrdev(200, "chrtest", &test_fops);
45 if(retvalue < 0){
46 /* 字符设备注册失败,自行处理 */
47 }
48 return 0;
49 }
50
51 /* 驱动出口函数 */
52 static void __exit xxx_exit(void)
53 {
54 /* 注销字符设备驱动 */
55 unregister_chrdev(200, "chrtest");
56 }
57
58 /* 将上面两个函数指定为驱动的入口和出口函数 */
59 module_init(xxx_init);
60 module_exit(xxx_exit);
```


添加LICENSE和作者信息(LICENSE必须添加)

```c
2 MODULE_LICENSE("GPL");
63 MODULE_AUTHOR("GoKu");
```

### 3. Device number of linux

1. ，Linux 中每个设备都有一个设备号，设备号由主设备号和次设备号两部分
组成，主设备号表示某一个具体的驱动，次设备号表示使用这个驱动的各个设备。Linux 提供了
一个名为 dev_t 的数据类型表示设备号，dev_t 定义在文件 include/linux/types.h 里面，定义如下：
示例代码 20.3.1.1 设备号 dev_t
13 typedef __u32 __kernel_dev_t;
......
16 typedef __kernel_dev_t dev_t;
可以看出 dev_t 是__u32 类型的，而__u32 定义在文件 include/uapi/asm-generic/int-ll64.h
typedef unsigned int __u32;


dev_t 其实就是 unsigned int 类型，是一个 32 位的数据类型
高 12 位为主设备号，低 20 位为次设备号。因此 Linux
系统中主设备号范围为 0~4095，所

动态分配设备号
静态分配设备号需要我们检查当前系统中所有被使用了的设备号，然后挑选一个没有使用
的。而且静态分配设备号很容易带来冲突问题，Linux 社区推荐使用动态分配设备号，在注册字
符设备之前先申请一个设备号，系统会自动给你一个没有被使用的设备号，这样就避免了冲突。
卸载驱动的时候释放掉这个设备号即可，设备号的申请函数如下：
int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)
函数 alloc_chrdev_region 用于申请设备号，此函数有 4 个参数：
dev：保存申请到的设备号。
baseminor：次设备号起始地址，alloc_chrdev_region 可以申请一段连续的多个设备号，这
些设备号的主设备号一样，但是次设备号不同，次设备号以 baseminor 为起始地址地址开始递
增。一般 baseminor 为 0，也就是说次设备号从 0 开始。
count：要申请的设备号数量。
name：设备名字。
注销字符设备之后要释放掉设备号，设备号释放函数如下：
void unregister_chrdev_region(dev_t from, unsigned count)

from：要释放的设备号。
count：表示从 from 开始，要释放的设备号数量。



















