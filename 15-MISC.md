<!--
 * @Date: 2024-12-19
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-12-23
 * @FilePath: /1-STM32MP157/15-MISC.md
 * @Description: 
-->
# MISC

> misc 的意思是混合、杂项的，因此 MISC 驱动也叫做杂项驱动，也就是当我们板子上的某些外设无法进行分类的时候就可以使用 MISC 驱动。MISC 驱动其实就是最简单的字符设备驱动，通常嵌套在 platform 总线驱动中，实现复杂的驱动

### 1. MISC 设备驱动简介

所有的 MISC 设备驱动的主设备号都为 10，不同的设备使用不同的从设备号。MISC 设备会自动创建 cdev，不需要像我们以前那样手动创建，因此采用 MISC 设
备驱动可以简化字符设备驱动的编写。我们需要向 Linux 注册一个 miscdevice 设备，miscdevice
是一个结构体，定义在文件 include/linux/miscdevice.h 中，内容如下：
**miscdevice 结构体代码**
```c
struct miscdevice
{
        int minor;/* 子设备号*/
        const char *name;/* 设备名字*/
        const struct file_operations *fops;/* 设备操作集*/
        struct list_head list;
        struct device *parent;
        struct device *this_device;
        const struct attribute_group **groups;
        const char *nodename;
        umode_t mode;
 };
```
定义一个 MISC 设备(miscdevice 类型)以后我们需要设置 minor、name 和 fops 这三个成员
变量。
Linux 系统已经预定义了一些 MISC 设备的子设备号
我们在使用的时候可以从这些预定义的子设备号中挑选一个，当然也可以自己定义，只要
这个子设备号没有被其他设备使用接口。
name 就是此 MISC 设备名字，
当此设备注册成功以后就会在/dev 目录下生成一个名为 name
的设备文件。fops 就是字符设备的操作集合，MISC 设备驱动最终是需要使用用户提供的 fops
操作集合。

当设置好 miscdevice 以后就需要使用 misc_register 函数向系统中注册一个 MISC 设备，此
函数原型如下：
int misc_register(struct miscdevice * misc)

以前我们需要自己调用一堆的函数去创建设备，比如在以前的字符设备驱动中我们会使用
如下几个函数完成设备创建过程：
示例代码 37.1.3 传统的创建设备过程
1 alloc_chrdev_region();/* 申请设备号*/
2 cdev_init();/* 初始化 cdev*/
3 cdev_add();/* 添加 cdev*/
4 class_create();/* 创建类*/
5 device_create();/* 创建设备*/
现在我们可以直接使用 misc_register 一个函数来完成示例代码 37.1.3 中的这些步骤。当我
们卸载设备驱动模块的时候需要调用 misc_deregister 函数来注销掉 MISC 设备，函数原型如下：
int misc_deregister(struct miscdevice *misc)
函数参数和返回值含义如下：
misc：要注销的 MISC 设备。
返回值：负数，失败；0，成功。
以前注销设备驱动的时候，我们需要调用一堆的函数去删除此前创建的 cdev、设备等等内
容，如下所示：
示例代码 37.1.4 传统的删除设备的过程
1 cdev_del();/*删除 cdev*/
2 unregister_chrdev_region();/* 注销设备号*/
3 device_destroy();/* 删除设备*/
4 class_destroy();/* 删除类*/
现在我们只需要一个 misc_deregister 函数即可完成示例代码 37.1.4 中的这些工作。关于
MISC 设备驱动就讲解到这里，接下来我们就使用 platform 加 MISC 驱动框架来编写 beep 蜂鸣
器驱动。





































