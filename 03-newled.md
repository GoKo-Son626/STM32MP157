<!--
 * @Date: 2024-11-12
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-11-14
 * @FilePath: /1-STM32MP157/03-NEW_char_device.md
 * @Description: 
-->
# NEW_char_device

> 0. How to write new character device drivers and automatically create device node files when the driver module is loaded.

### 新字符设备驱动原理
1. 设备号分配和释放
如果没有指定设备号的话就使用如下函数来申请设备号：
int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)
如果给定了设备的主设备号和次设备号就使用如下所示函数来注册设备号即可：
int register_chrdev_region(dev_t from, unsigned count, const char *name)
参数 from 是要申请的起始设备号，也就是给定的设备号；参数baseminor是分配次设备号的起始值；参数 count 是要申请的次设备号数量，一
般都是一个；参数 name 是设备名字。
注 销 字 符 设 备 之 后 要 释 放 掉 设 备 号 ， 不 管 是 通 过 alloc_chrdev_region 函 数 还 是
register_chrdev_region 函数申请的设备号，统一使用如下释放函数：
void unregister_chrdev_region(dev_t from, unsigned count)
```c
1 int major; /* 主设备号 */
2 int minor; /* 次设备号 */
3 dev_t devid; /* 设备号 */
4 
5 if (major) { /* 定义了主设备号 */
6 devid = MKDEV(major, 0); /* 大部分驱动次设备号都选择 0 */
7 register_chrdev_region(devid, 1, "test");
8 } else { /* 没有定义设备号 */
9 alloc_chrdev_region(&devid, 0, 1, "test"); /* 申请设备号 */
10 major = MAJOR(devid); /* 获取分配号的主设备号 */
11 minor = MINOR(devid); /* 获取分配号的次设备号 */
12 }
```
2. 字符设备注册方法
**字符设备结构**
cdev 结构体表示一个字符设备，cdev 结构体在 include/linux/cdev.h 文件中
```c
1 struct cdev {
        2 struct kobject kobj;
        3 struct module *owner;
        4 const struct file_operations *ops;
        5 struct list_head list;
        6 dev_t dev;
        7 unsigned int count;
8 } __randomize_layout;
```
ops 和 dev，这两个就是字符设备文件操作函数集合file_operations 以及设备号 dev_t。编写字符设备驱动之前需要定义一个 cdev 结构体变量，这个
变量就表示一个字符设备，如下所示：
```c
struct cdev test_cdev;
```
**cdev_init 和 cdev_add函数**
定义好 cdev 变量以后就要使用 cdev_init 函数对其进行初始化，cdev_init 函数原型如下：
void cdev_init(struct cdev *cdev, const struct file_operations *fops)
参数 cdev 就是要初始化的 cdev 结构体变量，参数 fops 就是字符设备文件操作函数集合。
使用 cdev_init 函数初始化 cdev 变量的示例代码如下：
示例代码 22.1.2.2 cdev_init 函数使用示例代码
1 struct cdev testcdev;
2 
3 /* 设备操作函数 */
4 static struct file_operations test_fops = {
5 .owner = THIS_MODULE,
6 /* 其他具体的初始项 */
7 };
8 
9 testcdev.owner = THIS_MODULE;
10 cdev_init(&testcdev, &test_fops); /* 初始化 cdev 结构体变量 */
完成对 cdev 结构体变量的初始化，然后使用 cdev_add 函数向 Linux 系统添加这个字符设备。
cdev_add 函数原型如下：
int cdev_add(struct cdev *p, dev_t dev, unsigned count)
参数 p 指向要添加的字符设备(cdev 结构体变量)，参数 dev 就是设备所使用的设备号，参
数 count 是要添加的设备数量。
```c
cdev_add(&testcdev, devid, 1); /* 添加字符设备 */
```
**cdev_del 函数**
卸载驱动的时候一定要使用 cdev_del 函数从 Linux 内核中删除相应的字符设备，cdev_del
函数原型如下：
void cdev_del(struct cdev *p)
参数 p 就是要删除的字符设备。如果要删除字符设备，参考如下代码：
示例代码 22.1.2.3 cdev_del 函数使用示例
1 cdev_del(&testcdev); /* 删除 cdev */

alloc_chrdev_region + cdev_init + cdev_add => register_chrdev

cdev_del + unregister_chrdev_region => unregister_chrdev

### 2. Create device nodes automatically

1. mdev(simply udev)
udev 是一个用户程序，在 Linux 下通过 udev 来实现设备文件的创建与删除，udev 可以检
测系统中硬件设备状态，可以根据系统中硬件设备状态来创建或者删除设备文件。

创建和删除类
struct class *class_create (struct module *owner, const char *name)
class_create 一共有两个参数，参数 owner 一般为 THIS_MODULE，参数 name 是类名字。
返回值是个指向结构体 class 的指针，也就是创建的类。
卸载驱动程序的时候需要删除掉类，类删除函数为 class_destroy，函数原型如下：
void class_destroy(struct class *cls);
参数 cls 就是要删除的类。
创建和删除设备
。使用 device_create 函数在类下面创建设备，device_create 函数原型如下：
struct device *device_create(struct class *cls,
                                struct device *parent, 
                                dev_t devt, 
                                void *drvdata, 
                                const char *fmt, ...)
device_create 是个可变参数函数，参数 cls 就是设备要创建哪个类下面；参数 parent 是父设
备，一般为 NULL，也就是没有父设备；参数 devt 是设备号；参数 drvdata 是设备可能会使用的
一些数据，一般为 NULL；参数 fmt 是设备名字，如果设置 fmt=xxx 的话，就会生成/dev/xxx 这
个设备文件。返回值就是创建好的设备。
void device_destroy(struct class *cls, dev_t devt)
参数 classs 是要删除的设备所处的类，参数 devt 是要删除的设备号。

参考示例
```c
1 struct class *class; /* 类 */ 
2 struct device *device; /* 设备 */
3 dev_t devid; /* 设备号 */ 
4 
5 /* 驱动入口函数 */
6 static int __init xxx_init(void)
7 {
8 /* 创建类 */
9 class = class_create(THIS_MODULE, "xxx");
10 /* 创建设备 */
11 device = device_create(class, NULL, devid, NULL, "xxx");
12 return 0;
13 }
14
15 /* 驱动出口函数 */
16 static void __exit led_exit(void)
17 {
18 /* 删除设备 */
19 device_destroy(newchrled.class, newchrled.devid);
20 /* 删除类 */
21 class_destroy(newchrled.class);
22 }
23
24 module_init(led_init);
25 module_exit(led_exit);
```

设置文件私有数据
/* 设备结构体 */
1 struct test_dev{
2 dev_t devid; /* 设备号 */
3 struct cdev cdev; /* cdev */
4 struct class *class; /* 类 */
5 struct device *device; /* 设备 */
6 int major; /* 主设备号 */
7 int minor; /* 次设备号 */
8 };
9 
10 struct test_dev testdev;
11 
12 /* open 函数 */
13 static int test_open(struct inode *inode, struct file *filp)
14 {
15 filp->private_data = &testdev; /* 设置私有数据 */
16 return 0;
17 }

在 open 函数里面设置好私有数据以后，在 write、read、close 等函数中直接读取 private_data
即可得到设备结构体。






















