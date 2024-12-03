<!--
 * @Date: 2024-12-03
 * @LastEditors: GoKo-Son626
 * @LastEditTime: 2024-12-03
 * @FilePath: /1-STM32MP157/07_key.md
 * @Description: 
-->
# key

### 0. 

### 1. lsls

KEY0 接了一个 10K 的上拉电阻，因此 KEY0 没有按下的时候 PG3 应该是
高电平，当 KEY0 按下以后 PG3 就是低电平。

在根节点“/”下创建 KEY 节点，节点名为“key”，节点内容如下

1 key {
2 compatible = "alientek,key";
3 status = "okay";
4 key-gpio = <&gpiog 3 GPIO_ACTIVE_LOW>;
5 };


atomic_t keyvalue; /* 按键值 */

/* 打开 key 驱动 */
44 fd = open(filename, O_RDWR);
45 if(fd < 0){
46 printf("file %s open failed!\r\n", argv[1]);
47 return -1;
48 }
49
50 /* 循环读取按键值数据！ */
51 while(1) {
52 read(fd, &keyvalue, sizeof(keyvalue));
53 if (keyvalue == KEY0VALUE) { /* KEY0 */
54 printf("KEY0 Press, value = %#X\r\n", keyvalue);/* 按下 */
55 }
56 }


```c
/* IMX6ULL */
pinctrl key: keygrp{
        fsl,pins =<
                MX6UL_PAD_UARTI_CTS_B_GPIO1_IO18_0xF080
        >;
}
key{
        compatible = "alientek,key";
        pinctrl-names = "default";
        pinctrl-0 = <&pinctrl key>;
        key-gpios = <&gpi01 18 GPIO ACTIVE HIGH>;
        status ="okay";
}
```

























