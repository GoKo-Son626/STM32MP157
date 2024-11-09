<!--
 * @Date: 2024-11-08
 * @LastEditors: GoKo Son626
 * @LastEditTime: 2024-11-09
 * @FilePath: /1-STM32MP157/02-Device_tree.md
 * @Description: 
-->
# 设备树

> 0. 设备树：描述硬件信息的数据结构，便于linux内核启动时提供设备信息从而使其操作硬件

1. linux设备树缘由：CUP和芯片的接口信息不再以代码的形式存在，而是以.dts文件存在，编译为.dtb文件，当linux内核启动时传递给linux内核，再变成paltform_device（和CPU的接口信息，管脚...）

2. 涉及文件：
-  dts:设备树源码
-  dtsi:通用设备树源码，类似C语言中的头文件
-  dtc:设备树编译器
-  dtb:编译设备树源码所得文件

3. [设备树官网](www.devicetreee.rog)
4. [语法入门文档](https://elinux.org/Device_Tree_Usage)
