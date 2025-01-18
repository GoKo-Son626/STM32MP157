<!--
 * @Date: 2025-01-10
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2025-01-18
 * @FilePath: /1-STM32MP157/29-block_driver.md
 * @Description: 
-->
# block_driver

> 块设备驱动是Linux 三大驱动类型之一。块设备驱动要远比字符设备驱动复杂得多，不同类型的存储设备又对应不同的驱动子系统，本章我们重点学习一下块设备相关驱动概念，不涉及到具体的存储设备。最后，我们使用 STM32MP1 开发板板载 RAM 模拟一个块设备，学习块设备驱动框架的使用。

### 1. 块设备

- 块设备是针对存储设备的，比如 SD 卡、EMMC、NAND Flash、Nor Flash、SPI Flash、机械硬盘、固态硬盘等。因此块设备驱动其实就是这些存储设备驱动，块设备驱动相比字符设备驱动的主要区别如下：
  1. 块设备只能以块为单位进行读写访问，块是 linux 虚拟文件系统(VFS)基本的数据传输单位。字符设备是以字节为单位进行数据传输的，不需要缓冲。
  2. 块设备在结构上是可以进行随机访问的，对于这些设备的读写都是按块进行的，块设备使用缓冲区来暂时存放数据，等到条件成熟以后再一次性将缓冲区中的数据写入块设备中。这么做的目的为了提高块设备寿命，大家如果仔细观察的话就会发现有些硬盘或者 NANDFlash 就会标明擦除次数(flash 的特性，写之前要先擦除)，比如擦除 100000 次等。因此，为了提高块设备寿命而引入了缓冲区，数据先写入到缓冲区中，等满足一定条件后再一次性写入到真正的物理存储设备中，这样就减少了对块设备的擦除次数，提高了块设备寿命。字符设备是顺序的数据流设备，字符设备是按照字节进行读写访问的。字符设备不需要缓冲区，对于字符设备的访问都是实时的，而且也不需要按照固定的块大小进行访问。
  3. Linux 内 核 使 用 block_device 表 示 块 设 备 ， block_device 为 一 个 结 构 体 ， 定 义 在include/linux/fs.h 文件中，结构体内容如下所示：

### 2. 块设备驱动框架 

##### 1. block_device 结构体
```c
struct block_device {
    dev_t bd_dev;                    /* not a kdev_t - it's a search key */
    int bd_openers;                   /* number of openers */
    struct inode *bd_inode;           /* associated inode */
    struct super_block *bd_super;     /* associated superblock */
    struct mutex bd_mutex;            /* open/close mutex */
    void *bd_claiming;                /* the claiming owner */
    void *bd_holder;                  /* the holder of the block device */
    int bd_holders;                   /* number of holders */
    bool bd_write_holder;             /* whether it has a write holder */

#ifdef CONFIG_SYSFS
    struct list_head bd_holder_disks; /* list of disks held by the block device */
#endif

    struct block_device *bd_contains; /* the block device that contains this one */
    unsigned bd_block_size;           /* block size */
    u8 bd_partno;                     /* partition number */
    struct hd_struct *bd_part;        /* partition information */
    unsigned bd_part_count;           /* number of times partitions have been opened */
    int bd_invalidated;               /* invalidation status */

    struct gendisk *bd_disk;          /* the general disk structure */
    struct request_queue *bd_queue;   /* request queue */
    struct backing_dev_info *bd_bdi;  /* backing device info */

    struct list_head bd_list;         /* list of block devices */

    /*
     * Private data. You must have bd_claim'ed the block_device
     * to use this. NOTE: If the same device is used multiple
     * times, the owner must take special care to not mess up bd_private.
     */
    unsigned long bd_private;

    /* The counter of freeze processes */
    int bd_fsfreeze_count;

    /* Mutex for freeze */
    struct mutex bd_fsfreeze_mutex;
} __randomize_layout;
```
根据你的要求，以下是格式正确的 `struct block_device` 定义：

```c
struct block_device {
    dev_t bd_dev;                    /* not a kdev_t - it's a search key */
    int bd_openers;                   /* number of openers */
    struct inode *bd_inode;           /* associated inode */
    struct super_block *bd_super;     /* associated superblock */
    struct mutex bd_mutex;            /* open/close mutex */
    void *bd_claiming;                /* the claiming owner */
    void *bd_holder;                  /* the holder of the block device */
    int bd_holders;                   /* number of holders */
    bool bd_write_holder;             /* whether it has a write holder */

#ifdef CONFIG_SYSFS
    struct list_head bd_holder_disks; /* list of disks held by the block device */
#endif

    struct block_device *bd_contains; /* the block device that contains this one */
    unsigned bd_block_size;           /* block size */
    u8 bd_partno;                     /* partition number */
    struct hd_struct *bd_part;        /* partition information */
    unsigned bd_part_count;           /* number of times partitions have been opened */
    int bd_invalidated;               /* invalidation status */

    struct gendisk *bd_disk;          /* the general disk structure */
    struct request_queue *bd_queue;   /* request queue */
    struct backing_dev_info *bd_bdi;  /* backing device info */

    struct list_head bd_list;         /* list of block devices */

    /*
     * Private data. You must have bd_claim'ed the block_device
     * to use this. NOTE: If the same device is used multiple
     * times, the owner must take special care to not mess up bd_private.
     */
    unsigned long bd_private;

    /* The counter of freeze processes */
    int bd_fsfreeze_count;

    /* Mutex for freeze */
    struct mutex bd_fsfreeze_mutex;
} __randomize_layout;
```
在此结构体中：
- `bd_dev` 是设备的标识符，但并非 `kdev_t` 类型，而是作为搜索关键字使用。
- `bd_openers` 表示该设备的打开次数。
- `bd_inode` 和 `bd_super` 分别指向与设备相关的 `inode` 和 `super_block`。
- `bd_mutex` 是设备的互斥锁，用于同步设备的打开和关闭操作。
- `bd_claiming` 和 `bd_holder` 表示设备的所有者和持有者。
- `bd_holders` 表示有多少个持有者。
- `bd_write_holder` 用于标记是否存在写持有者。
- `bd_contains` 表示该设备是否包含另一个设备。
- `bd_block_size` 表示设备的块大小。
- `bd_partno` 是设备的分区号。
- `bd_part` 是与设备相关的分区信息。
- `bd_part_count` 是该设备分区被打开的次数。
- `bd_invalidated` 是设备的失效状态。
- `bd_disk` 是关联的通用磁盘结构体。
- `bd_queue` 和 `bd_bdi` 分别是设备的请求队列和后备设备信息。
- `bd_list` 是设备链表中的节点。
- `bd_private` 存储与设备相关的私有数据。
- `bd_fsfreeze_count` 和 `bd_fsfreeze_mutex` 用于冻结设备时的计数和互斥锁。
- 重点关注一下`bd_disk` 成员变量，此成员变量为 `gendisk` 结构体指针类型。内核使用 block_device 来表示一个具体的块设备对象，比如一个硬盘或者分区，如果是硬盘的话 `bd_disk` 就指向通用磁盘结构 `gendisk`

**注册块设备**
和字符设备驱动一样，我们需要向内核注册新的块设备、申请设备号，块设备注册函数为
register_blkdev，函数原型如下：
int register_blkdev(unsigned int major, const char *name)
函数参数和返回值含义如下：
major ：主设备号。
name ：块设备名字。
返回值：如果参数 major 在 1~(BLKDEV_MAJOR_MAX-1)之间的话表示自定义主设备号，
那么返回 0 表示注册成功，如果返回负值的话表示注册失败。如果 major 为 0 的话表示由系
统自动分配主设备号，那么返回值就是系统分配的主设备号，如果返回负值那就表示注册失败。

**注销块设备**
和字符设备驱动一样，如果不使用某个块设备了，那么就需要注销掉，函数为
unregister_blkdev，函数原型如下：
void unregister_blkdev(unsigned int major, const char *name)
函数参数和返回值含义如下：
major ：要注销的块设备主设备号。
name： ： 要注销的块设备名字。
返回值：无。

##### 2. gendisk 结构体

linux内核使用gendisk描述一个磁盘设备
```c
//gendisk structure

```
第 5 行，major 为磁盘设备的主设备号。
第 6 行，first_minor 为磁盘的第一个次设备号。
第 7 行，minors 为磁盘的此设备号数量，也就是磁盘的分区数量，这些分区的主设备号
一样，此设备号不同。
第 21 行，part_tbl 为磁盘对应的分区表，为结构体 disk_part_tbl 类型，disk_part_tbl 的核
心是一个 hd_struct 结构体指针数组，此数组每一项都对应一个分区信息。
第 24 行，fops 为块设备操作集，为 block_device_operations 结构体类型。和字符设备操
作集 file_operations 一样，是块设备驱动中的重点！
第 25 行，queue 为磁盘对应的请求队列，所以针对该磁盘设备的请求都放到此队列中，
驱动程序需要处理此队列中的所有请求。
编写块的设备驱动的时候需要分配并初始化一个 gendisk，linux 内核提供了一组 gendisk
操作函数，我们来看一下一些常用的 API 函数。
1、申请 gendisk
使用 gendisk 之前要先申请，allo_disk 函数用于申请一个 gendisk，函数原型如下：
struct gendisk *alloc_disk(int minors)
函数参数和返回值含义如下：
minors：次设备号数量，也就是 gendisk对应的分区数量
返回值：成功：返回申请到的 gendisk，失败：NULL。
2、删除 gendisk
如果要删除 gendisk 的话可以使用函数 del_gendisk，函数原型如下：
void del_gendisk(struct gendisk *gp)
函数参数和返回值含义如下：
gp: 删除的 gendisk。
返回值：无。
3、将 gendisk 添加到内核
使用 alloc_disk 申请到 gendisk 以后系统还不能使用，必须使用 add_disk 函数将申请到
的 gendisk 添加到内核中，add_disk 函数原型如下：
void add_disk(struct gendisk *disk)
函数参数和返回值含义如下：
disk: 添加到内核的 gendisk。
返回值：无。
4、设置 gendisk 容量
每一个磁盘都有容量，所以在初始化 gendisk 的时候也需要设置其容量，使用函数
set_capacity，函数原型如下：
void set_capacity(struct gendisk *disk, sector_t size)
函数参数和返回值含义如下：
disk: 设置容量的 gendisk。
size：盘容量大小，注意这里是扇区数量。块设备中最小的可寻址单元是扇区，一个扇区一
般是 512 字节，有些设备的物理扇区可能不是 512 字节。不管物理扇区是多少，内核和块设
备驱动之间的扇区都是 512 字节。所以 set_capacity 函数设置的大小就是块设备实际容量除以
512 字节得到的扇区数量。比如一个 2MB 的磁盘，其扇区数量就是(2*1024*1024)/512=4096。
返回值：无。
5、调整 gendisk 引用计数
内核会通过 get_disk_and_module 和 put_disk 这两个函数来调整 gendisk 的引用计数，根
据名字就可以知道，get_disk_and_module 是增加 gendisk 的引用计数，put_disk 是减少 gendisk
的引用计数，这两个函数原型如下所示：
truct kobject * get_disk_and_module (struct gendisk *disk)
void put_disk(struct gendisk *disk)

##### 3. block_device_operations structure

- 和file_operations一样，block_device_operations为块设备的操作集合
```c
block_device_operations structure

```
第 2 行，open 函数用于打开指定的块设备。
第 3 行，release 函数用于关闭(释放)指定的块设备。
第 4 行，rw_page 函数用于读写指定的页。
第 5 行，ioctl 函数用于块设备的 I/O 控制。
第 6 行，compat_ioctl 函数和 ioctl 函数一样，都是用于块设备的 I/O 控制。区别在于在
64 位系统上，32 位应用程序的 ioctl 会调用 compat_iotl 函数。在 32 位系统上运行的 32 位
应用程序调用的就是 ioctl 函数。
第 13 行，getgeo 函数用于获取磁盘信息，包括磁头、柱面和扇区等信息。
第 18 行，owner 表示此结构体属于哪个模块，一般直接设置为 THIS_MODULE。

##### 4. 块设备IO请求

在 block_device_operations 结构体中并没有找到 read 和 write 这样
的读写函数，那么块设备是怎么从物理块设备中读写数据？这里就引处理块设备驱动中非常重
要的 request_queue、request 和 bio

**request_queue、request**
......

既然 bio 是块设备最小的数据传输单元，那么 bio 就有必要描述清楚这
些信息，其中 bi_iter 这个结构体成员变量就用于描述物理存储设备地址信息，比如要操作的扇
区地址。bi_io_vec 指向 bio_vec 数组首地址，bio_vec 数组就是 RAM 信息，比如页地址、页
偏移以及长度，
“页地址”是 linux 内核里面内存管理相关的概念，这里我们不深究 linux 内存
管理，我们只需要知道对于 RAM 的操作最终会转换为页相关操作

















