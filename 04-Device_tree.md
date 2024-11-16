<!--
 * @Date: 2024-11-08
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-11-16
 * @FilePath: /1-STM32MP157/04-Device_tree.md
 * @Description: 
-->
# 设备树

> 0. 设备树：描述硬件信息的数据结构，便于linux内核启动时提供设备信息从而使其操作硬件
> [设备树官网](www.devicetreee.rog)
> [语法入门文档](https://elinux.org/Device_Tree_Usage)

### 设备树相关
1. linux设备树缘由：CUP和芯片的接口信息不再以代码的形式存在，而是以.dts文件存在，编译为.dtb文件，当linux内核启动时传递给linux内核，再变成paltform_device（和CPU的接口信息，管脚...）
设备树结构示意图
![设备树结构示意图](image.png)
- 树的主干就是系统总线，IIC 控制器、GPIO 控制器、SPI 控制器等都是接到系统主线上的分支。IIC 控制器有分为 IIC1 和 IIC2 两种，其中 IIC1 上接了 FT5206 和 AT24C02这两个 IIC 设备，IIC2 上只接了 MPU6050 这个设备。DTS 文件的主要功能就是按照图 23.1.1
所示的结构来描述板子上的设备信息，
1. 涉及文件：
-  dts:设备树源码
-  dtsi:通用设备树源码，类似C语言中的头文件
-  dtc:设备树编译器
-  dtb:编译设备树源码所得文件
-  dtc工具源码:scripts/dtc/

### DTS语法
1. .dts文件
以 stm32mp157d-atk.dts 这个文件为例来讲解一下 DTS 语法。关于设备树详细
的语法规则请参考《Devicetree SpecificationV0.2.pdf》和《Power_ePAPR_APPROVED_v1.12.pdf》

在.dts设备树文件中,可以通过“#include”来引用.h,.dtsi和.dts文件,但是最好选择.dtsi后缀

**SOC（片上系统 System on chip）**
- SoC 是指你在开发板上使用的核心硬件部分，它集成了各种计算和控制功能，而 开发板 是将这个 SoC 和一些外围硬件（如内存、闪存、电源管理、接口、调试端口等）集成在一起的完整开发平台。
**设备节点**
设备树是采用树形结构来描述板子上的设备信息的文件，每个设备都是一个节点，叫做设
备节点，每个节点都通过一些属性信息来描述节点信息，属性就是键—值对。

“/”是根节点，每个设备树文件只有一个根节点。
设备
树中节点命名格式如下：
node-name@unit-address
其中“node-name”是节点名字，为 ASCII 字符串，节点名字应该能够清晰的描述出节点的
功能
但是我们在示例代码 23.3.2.1 中我们看到的节点命名却如下所示：
cpu0:cpu@0
上述命令并不是“node-name@unit-address”这样的格式，而是用“：”隔开成了两部分，“：”
前面是节点标签(label)，“：”后面的才是节点名字，格式如下所示：
label: node-name@unit-address
引入 label 的目的就是为了方便访问节点，可以直接通过&label 来访问这个节点，比如通过
&cpu0 就可以访问“cpu@0”这个节点，而不需要输入完整的节点名字。再比如节点 “sram: 
sram@10000000”，节点 label 是 sram，而节点名字就很长了，为“sram@10000000”。很明显通
过&sram 来访问“sram@10000000”这个节点要方便很多

**标准属性**
用户可以
自定义属性。除了用户自定义属性，有很多属性是标准属性，Linux 下的很多外设驱动都会使用
这些标
1. compatible属性
compatible 属性的值是
一个字符串列表，compatible 属性用于将设备和驱动绑定起来。字符串列表用于选择设备所要
使用的驱动程序，compatible 属性的值格式如下所示：
"manufacturer,model"
其中 manufacturer 表示厂商，model 一般是模块对应的驱动名字。
一般驱动程序文件都会有一个 OF 匹配表，此 OF 匹配表保存着一些 compatible 值，如果设
备节点的 compatible 属性值和 OF 匹配表中的任何一个值相等，那么就表示设备可以使用这个
驱动。
const struct of_device_id cs42l51_of_match[] = {
        { .compatible = "cirrus,cs42l51", },
        { }
};
2、model 属性
model 属性值也是一个字符串，一般 model 属性描述开发板的名字或者设备模块信息，比
如名字什么的，比如：
model = "STMicroelectronics STM32MP157C-DK2 Discovery Board";
3、status 属性
status 属性看名字就知道是和设备状态有关的，status 属性值也是字符串，字符串是设备的
状态信息，可选的状态如表 23.3.3.1 所示：
值 描述
“okay” 表明设备是可操作的。
“disabled”
表明设备当前是不可操作的，但是在未来可以变为可操作的，比如热插拔设备
插入以后。至于 disabled 的具体含义还要看设备的绑定文档。
“fail”
表明设备不可操作，设备检测到了一系列的错误，而且设备也不大可能变得可
操作。
“fail-sss” 含义和“fail”相同，后面的 sss 部分是检测到的错误内容。
#address-cells 和#size-cells 属性
这两个属性的值都是无符号 32 位整形，#address-cells 和#size-cells 这两个属性可以用在任
何拥有子节点的设备中，用于描述子节点的地址信息。#address-cells 属性值决定了子节点 reg 属
性中地址信息所占用的字长(32 位)，#size-cells 属性值决定了子节点 reg 属性中长度信息所占的
字长(32 位)。#address-cells 和#size-cells 表明了子节点应该如何编写 reg 属性值，一般 reg 属性
都是和地址有关的内容，和地址相关的信息有两种：起始地址和地址长度，reg 属性的格式为：
reg = <address1 length1 address2 length2 address3 length3……>
5.reg属性
，reg 属性的值一般是(address，length)对。reg 属性一般用于描
述设备地址空间资源信息或者设备地址信息，比如某个外设的寄存器地址范围信息，或者 IIC
器件的设备地址等，比

第八行
`reg = <0x0 0x60000>;`

解释：
- `reg` 定义了 `sram` 设备的子地址空间：
  - 子地址空间的起始地址：`0x0`
  - 地址范围长度：`0x60000`（384KB）

第六行
ranges = <0 0x10000000 0x100000>;

含义是：
子地址空间的起始地址：0
父地址空间的起始地址：0x10000000
映射范围大小：0x100000（1MB）

7、name 属性
name 属性值为字符串，name 属性用于记录节点名字，name 属性已经被弃用，不推荐使用
name 属性，一些老的设备树文件可能会使用此属性。
8、device_type 属性
device_type 属性值为字符串，IEEE 1275 会用到此属性，用于描述设备的 FCode，但是设
备树没有 FCode，所以此属性也被抛弃了。此属性只能用于 cpu 节点或者 memory 节点。




























