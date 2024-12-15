<!--
 * @Date: 2024-12-12
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-12-15
 * @FilePath: /1-STM32MP157/11-asynchronous.md
 * @Description: 
-->
# Asynchronous-notification

> 在前面使用阻塞或者非阻塞的方式来读取驱动中按键值都是应用程序主动读取的，对于非阻塞方式来说还需要应用程序通过 poll 函数不断的轮询。最好的方式就是驱动程序能主动向应用程序发出通知，报告自己可以访问，然后应用程序再从驱动程序中读取或写入数据，类似于中断。Linux 提供了异步通知这个机制来完成此功能

### 1. about Asynchronous-notification
回顾一下“中断”
，中断是处理器提供的一种异步机制，我们配置好中断以后就
可以让处理器去处理其他的事情了，当中断发生以后会触发我们事先设置好的中断服务函数，
在中断服务函数中做具体的处理。

信号类似于我们硬件上使用的“中断”
，只不过信号是软件层次上
的。算是在软件层次上对中断的一种模拟，驱动可以通过主动向应用程序发送信号的方式来报
告自己可以访问了，应用程序获取到信号以后就可以从驱动设备中读取或者写入数据了。整个
过程就相当于应用程序收到了驱动发送过来了的一个中断，然后应用程序去响应这个中断，在
整个处理过程中应用程序并没有去查询驱动设备是否可以访问，一切都是由驱动设备自己告诉
给应用程序的。

异步通知的核心就是信号，在 arch/xtensa/include/uapi/asm/signal.h 文件中定义了 Linux 所支
持的所有信号，这些信号如下所示：
```c
18 #define SIGHUP1/* 终端挂起或控制进程终止*/
19 #define SIGINT2/* 终端中断(Ctrl+C 组合键)*/
20 #define SIGQUIT3/* 终端退出(Ctrl+\组合键)*/
21 #define SIGILL4/* 非法指令*/
22 #define SIGTRAP5/* debug 使用，有断点指令产生*/
23 #define SIGABRT6/* 由 abort(3)发出的退出指令*/
24 #define SIGIOT6/* IOT 指令*/
25 #define SIGBUS7/* 总线错误*/
26 #define SIGFPE8/* 浮点运算错误*/
27 #define SIGKILL9/* 杀死、终止进程*/
28 #define SIGUSR110/* 用户自定义信号 1*/
30 #define SIGSEGV11/* 段违例(无效的内存段)*/
31 #define SIGUSR212/* 用户自定义信号 2*/
32 #define SIGPIPE13/* 向非读管道写入数据*/
33 #define SIGALRM14/* 闹钟*/
34 #define SIGTERM15/* 软件终止*/
35 #define SIGSTKFLT16/* 栈异常*/
36 #define SIGCHLD17/* 子进程结束*/
37 #define SIGCONT18/* 进程继续*/
38 #define SIGSTOP19/* 停止进程的执行，只是暂停*/
39 #define SIGTSTP20/* 停止进程的运行(Ctrl+Z 组合键)*/
40 #define SIGTTIN21/* 后台进程需要从终端读取数据*/
41 #define SIGTTOU22/* 后台进程需要向终端写数据*/
42 #define SIGURG23/* 有"紧急"数据*/
43 #define SIGXCPU24/* 超过 CPU 资源限制*/
44 #define SIGXFSZ25/* 文件大小超额*/
45 #define SIGVTALRM26/* 虚拟时钟信号*/
46 #define SIGPROF27/* 时钟信号描述*/
47 #define SIGWINCH28/* 窗口大小改变*/
48 #define SIGIO29/* 可以进行输入/输出操作*/
49 #define SIGPOLL SIGIO
50 /* #define SIGLOS29 */
51 #define SIGPWR30/* 断点重启*/
52 #define SIGSYS31/* 非法的系统调用*/
53 #define31 /* 未使用信号*/
```
```c
1#include <stdlib.h>
2#include <stdio.h>
3#include <signal.h>
4
5void sigint_handler(int num)
6{
7printf("\r\nSIGINT signal!\r\n");
8exit(0);
9 }
10
11 int main(void)
12 {
13signal(SIGINT, sigint_handler);
14while(1);
15return 0;
16 }
```
##### 1.2 驱动中的信号处理
1. fasync_struct 结构体
首先我们需要在驱动程序中定义一个 fasync_struct 结构体指针变量
```c
struct fasync_struct {
spinlock_tfa_lock;
intmagic;
intfa_fd;
struct fasync_struct *fa_next;
struct file*fa_file;
struct rcu_headfa_rcu;
};
```
一般将 fasync_struct 结构体指针变量定义到设备结构体中
2. fasync 函数

如果要使用异步通知，需要在设备驱动中实现 file_operations 操作集中的 fasync 函数

fasync_helper 函数的前三个参数就是 fasync 函数的那三个参数，第四个参数就是要初始化
的 fasync_struct 结构体指针变量。当应用程序通过“fcntl(fd, F_SETFL, flags | FASYNC)”改变
fasync 标记的时候，驱动程序 file_operations 操作集中的 fasync 函数就会执行。
```c
1struct xxx_dev {
2......
3struct fasync_struct *async_queue;
4};
5
6static int xxx_fasync(int fd, struct file *filp, int on)
7{
8 struct xxx_dev *dev = (xxx_dev)filp->private_data;
9
10 if (fasync_helper(fd, filp, on, &dev->async_queue) < 0)
11 return -EIO;
12 return 0;
13 }
14
15 static struct file_operations xxx_ops = {
16 ......
17 .fasync = xxx_fasync,
18 ......
19 };
```
在关闭驱动文件的时候需要在 file_operations 操作集中的 release 函数中释放 fasync_struct，
fasync_struct 的释放函数同样为 fasync_helper，release 函数参数参考实例如下：
示例代码 33.1.2.4 释放 fasync_struct 参考示例
```c
1 static int xxx_release(struct inode *inode, struct file *filp)
2 {
3       return xxx_fasync(-1, filp, 0); /* 删除异步通知 */
4 }
5
6 static struct file_operations xxx_ops = {
7 ......
8 .release = xxx_release,
9 };
```
3. kill_fasync 函数
当设备可以访问的时候，驱动程序需要向应用程序发出信号，相当于产生“中断”
。kill_fasync
函数负责发送指定的信号，kill_fasync 函数原型如下所示：
void kill_fasync(struct fasync_struct **fp, int sig, int band)
函数参数和返回值含义如下：
fp：要操作的 fasync_struct。
sig：要发送的信号。
band：可读时设置为 POLL_IN，可写时设置为 POLL_OUT。
返回值：无。

##### 1.3 应用程序对异步通知的处理
应用程序对异步通知的处理包括以下三步：
1. 注册信号处理函数
应用程序根据驱动程序所使用的信号来设置信号的处理函数，应用程序使用 signal 函数来
设置信号的处理函数。前面已经详细的讲过了，这里就不细讲了。
2. 将本应用程序的进程号告诉给内核
使用 fcntl(fd, F_SETOWN, getpid())将本应用程序的进程号告诉给内核。
3. 开启异步通知
使用如下两行程序开启异步通知：
```c
flags = fcntl(fd, F_GETFL);/* 获取当前的进程状态 */
fcntl(fd, F_SETFL, flags | FASYNC); /* 开启当前进程异步通知功能 */
```
重点就是通过 fcntl 函数设置进程状态为 FASYNC，经过这一步，驱动程序中的 fasync 函
数就会执行。















