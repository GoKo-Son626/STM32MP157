<!--
 * @Date: 2024-11-28
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-11-30
 * @FilePath: /1-STM32MP157/06-Concurrency-competition.md
 * @Description: 
-->
# Concurrency and competition

### 1. 原子操作
Linux 内核定义了叫做 atomic_t 的结构体来完成整形数据的原子操作，在使用中用原子变
量来代替整形变量，此结构体定义在 include/linux/types.h 文件中
```c
171 typedef struct {
172 int counter;
173 } atomic_t;
```
要使用原子操作 API 函数，首先要先定义一个 atomic_t 的变量，如下所示：
atomic_t a; //定义 a

函数 描述
ATOMIC_INIT(int i) 定义原子变量的时候对其初始化。
int atomic_read(atomic_t *v) 读取 v 的值，并且返回。
void atomic_set(atomic_t *v, int i) 向 v 写入 i 值。
void atomic_add(int i, atomic_t *v) 给 v 加上 i 值。
void atomic_sub(int i, atomic_t *v) 从 v 减去 i 值。
void atomic_inc(atomic_t *v) 给 v 加 1，也就是自增。
void atomic_dec(atomic_t *v) 从 v 减 1，也就是自减
int atomic_dec_return(atomic_t *v) 从 v 减 1，并且返回 v 的值。
int atomic_inc_return(atomic_t *v) 给 v 加 1，并且返回 v 的值。
int atomic_sub_and_test(int i, atomic_t *v) 从 v 减 i，如果结果为 0 就返回真，否则返回假
int atomic_dec_and_test(atomic_t *v) 从 v 减 1，如果结果为 0 就返回真，否则返回假
int atomic_inc_and_test(atomic_t *v) 给 v 加 1，如果结果为 0 就返回真，否则返回假
int atomic_add_negative(int i, atomic_t *v) 给 v 加 i，如果结果为负就返回真，否则返回假

如果使用 64 位的 SOC 的话，就要用到 64 位的原子变量，Linux 内核也定义了 64 位原子
结构体，如下所示：
示例代码 27.2.2.2 atomic64_t 结构体
176 typedef struct {
177 s64 counter;
178 } atomic64_t;
可以看出，counter 变为了 s64 类型，s64 本质是 long long 类型，相应的也提供了 64 位原
子变量的操作 API 函数
和表 27.2.1.1 中的 API 函数有用法一样，
只是将“atomic_”前缀换为“atomic64_”，将 int 换为 long long。如果使用的是 64 位的 SOC，
那么就要使用 64 位的原子操作函数。Cortex-A7 是 32 位的架构

### 2. 原子位操作
原子位操作是直接对内存进行操作

函数 描述
void set_bit(int nr, void *p) 将 p 地址的第 nr 位置 1。
void clear_bit(int nr,void *p) 将 p 地址的第 nr 位清零。
void change_bit(int nr, void *p) 将 p 地址的第 nr 位进行翻转。
int test_bit(int nr, void *p) 获取 p 地址的第 nr 位的值。
int test_and_set_bit(int nr, void *p) 将 p 地址的第 nr 位置 1，并且返回 nr 位原来的值。
int test_and_clear_bit(int nr, void *p) 将 p 地址的第 nr 位清零，并且返回 nr 位原来的值。
int test_and_change_bit(int nr, void *p) 将 p 地址的第 nr 位翻转，并且返回 nr 位原来的值。

### 3. 自旋锁

原子操作只能对整形变量或者位进行保护
自旋锁的“自旋”也就是“原地打转”的意思，“原地打转”的目的是为了等待自旋锁可以用，可以访问共享资源。
从这里我们可以看到自旋锁的一个缺点：那就
等待自旋锁的线程会一直处于自旋状态，这样会浪费处理器时间，降低系统性能，所以自旋锁
的持有时间不能太长。自旋锁适用于短时期的轻量级加锁，如果遇到需要长时间持有锁的场景

Linux 内核使用结构体 spinlock_t 表示自旋锁，结构体定义

61 typedef struct spinlock {
62 union {
63 struct raw_spinlock rlock;
64
65 #ifdef CONFIG_DEBUG_LOCK_ALLOC
66 # define LOCK_PADSIZE (offsetof(struct raw_spinlock, dep_map))
67 struct {
68 u8 __padding[LOCK_PADSIZE];
69 struct lockdep_map dep_map;
70 };
71 #endif
72 };
73 } spinlock_t;

spinlock_t lock; //定义自旋锁
自旋锁 API 函数
函数 描述
DEFINE_SPINLOCK(spinlock_t lock) 定义并初始化一个自选变量。
int spin_lock_init(spinlock_t *lock) 初始化自旋锁。
void spin_lock(spinlock_t *lock) 获取指定的自旋锁，也叫做加锁。
void spin_unlock(spinlock_t *lock) 释放指定的自旋锁。
int spin_trylock(spinlock_t *lock) 尝试获取指定的自旋锁，如果没有获取到就返回 0
int spin_is_locked(spinlock_t *lock) 检查指定的自旋锁是否被获取，如果没有被获取就
返回非 0，否则返回 0。


自旋锁 API 函数适用于 SMP 或支持抢占的单 CPU 下线程之间的并发访问，
也就是用于线程与线程之间，被自旋锁保护的临界区一定不能调用任何能够引起睡眠和阻塞的
API 函数，否则的话会可能会导致死锁现象的发生。自旋锁会自动禁止抢占，也就说当线程 A
得到锁以后会暂时禁止内核抢占。如果线程 A 在持有锁期间进入了休眠状态，那么线程 A 会自
动放弃 CPU 使用权。线程 B 开始运行，线程 B 也想要获取锁，但是此时锁被 A 线程持有，而
且内核抢占还被禁止了！线程 B 无法被调度出去，那么线程 A 就无法运行，锁也就无法释放，
好了，死锁发生了！

![中断打断线程](/File/images/Interrupt_interrupt_thread.png)
最好的解决方法就是获取锁之前关闭本地中断，Linux 内核提供了相应的 API 函数
函数 描述
void spin_lock_irq(spinlock_t *lock) 禁止本地中断，并获取自旋锁。
void spin_unlock_irq(spinlock_t *lock) 激活本地中断，并释放自旋锁。
void spin_lock_irqsave(spinlock_t *lock,
unsigned long flags) 保存中断状态，禁止本地中断，并获取自旋锁。
void spin_unlock_irqrestore(spinlock_t
*lock, unsigned long flags)
将中断状态恢复到以前的状态，并且激活本地中断，
释放自旋锁。

读写自旋锁
顺序锁

4 自旋锁使用注意事项
①、因为在等待自旋锁的时候处于“自旋”状态，因此锁的持有时间不能太长，一定要短，
否则的话会降低系统性能。如果临界区比较大，运行时间比较长的话要选择其他的并发处理方
式，比如稍后要讲的信号量和互斥体。
②、自旋锁保护的临界区内不能调用任何可能导致线程休眠的 API 函数，否则的话可能导
致死锁。
③、不能递归申请自旋锁，因为一旦通过递归的方式申请一个你正在持有的锁，那么你就
必须“自旋”，等待锁被释放，然而你正处于“自旋”状态，根本没法释放锁。结果就是自己把
自己锁死了！
④、在编写驱动程序的时候我们必须考虑到驱动的可移植性，因此不管你用的是单核的还
是多核的 SOC，都将其当做多核 SOC 来编写驱动程序。

### 4. 信号量
相比于自旋锁，信号量可以使线程进入休眠状态
B 也可以告诉 A，让 A 出来以后通知他一下，然后 B 继
续回房间睡觉，这个时候相当于信号量。可以看出，使用信号量会提高处理器的使用效率，毕
竟不用一直傻乎乎的在那里“自旋”等待。但是，信号量的开销要比自旋锁大，因为信号量使
线程进入休眠状态以后会切换线程，切换线程就会有开销。总结一下信号量的特点：
①、因为信号量可以使等待资源线程进入休眠状态，因此适用于那些占用资源比较久的场
合。
②、因此信号量不能用于中断中，因为信号量会引起休眠，中断不能休眠。
③、如果共享资源的持有时间比较短，那就不适合使用信号量了，因为频繁的休眠、切换
线程引起的开销要远大于信号量带来的那点优势。

如果要互斥的访问共享资源那么信号量的值就不能大于 1，此时的信号量就是一个二值信号量。

Linux 内核使用 semaphore 结构体表示信号量

15 struct semaphore {
16 raw_spinlock_t lock;
17 unsigned int count;
18 struct list_head wait_list;
19 };

函数 描述
DEFINE_SEAMPHORE(name) 定义一个信号量，并且设置信号量的值为 1。
void sema_init(struct semaphore *sem, int val) 初始化信号量 sem，设置信号量值为 val。
void down(struct semaphore *sem)
获取信号量，因为会导致休眠，因此不能在中
断中使用。
int down_trylock(struct semaphore *sem);
尝试获取信号量，如果能获取到信号量就获
取，并且返回 0。如果不能就返回非 0，并且
不会进入休眠。
int down_interruptible(str uct semaphore *sem)
获取信号量，和 down 类似，只是使用 down 进
入休眠状态的线程不能被信号打断。而使用此
void up(struct semaphore *sem) 释放信号量

### 5 互斥体

。在我们编写 Linux 驱动的时候遇到需要互斥访问的地方建议使用 mutex。Linux 内核使用 mutex 结构体表示互斥体，
struct mutex {
 atomic_long_t owner;
 spinlock_t wait_lock;
};
在使用 mutex 之前要先定义一个 mutex 变量。在使用 mutex 的时候要注意如下几点：
①、mutex 可以导致休眠，因此不能在中断中使用 mutex，中断中只能使用自旋锁。
②、和信号量一样，mutex 保护的临界区可以调用引起阻塞的 API 函数。
③、因为一次只有一个线程可以持有 mutex，因此，必须由 mutex 的持有者释放 mutex。并
且 mutex 不能递归上锁和解锁。

 互斥体 API 函数
函数 描述
DEFINE_MUTEX(name) 定义并初始化一个 mutex 变量。
void mutex_init(mutex *lock) 初始化 mutex。
void mutex_lock(struct mutex *lock)
获取 mutex，也就是给 mutex 上锁。如果获
取不到就进休眠。
void mutex_unlock(struct mutex *lock) 释放 mutex，也就给 mutex 解锁。
int mutex_trylock(struct mutex *lock)
尝试获取 mutex，如果成功就返回 1，如果失
败就返回 0。
int mutex_is_locked(struct mutex *lock)
判断 mutex 是否被获取，如果是的话就返回
1，否则返回 0。
int mutex_lock_interruptible(struct mutex *lock)
使用此函数获取信号量失败进入休眠以后可
以被信号打断。


### 6. 原子操作、自旋锁、信号量和互斥体。
























