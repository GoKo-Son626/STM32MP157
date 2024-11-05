/*
 * @Date: 2024-11-05
 * @LastEditors: GoKo Son626
 * @LastEditTime: 2024-11-05
 * @FilePath: \02-led\led.c
 * @Description: 
 */
#include<>


/* 1.寄存器地址映射 */
led_ioremap();

/* 2.使能GPIOI时钟 */
val = readl(MPU_AHB4_PERIPH_RCC_PI);
val |= ~(0x1 << 8); /* 清除bit8以前的值 */
val |= (0x1 << 8); /* 将bit8设置为1 */
write(val, MPU_AHB4_PERIPH_RCC_PI);

/* 3. 选择GPIOI为复用功能 */
val = readl(GPIO_MODERT_PI);
val &= ~(0x3 << 0); /* 将bit1和bit0清零 */
val |= (0x1 << 0); /* 将bit1和bit0设置为1 */
write(val, GPIO_MODER_PI);

/* 4. 选择GPIOI为通用推��输出 */
val = readl(GPIO_OTYPER_PI);
val &= ~(0x1  << 0); /* 将bit0清零 */
write(val, GPIO_OTYPER_PI);

/* 5.将GPIO_0设置为超高速 */
val = readl(GPIO_OSPEEDR_PI);
val &= ~(0x3 << 0); /* 将bit1和bit0清零 */
val |= (0x3 << 0); /* 将bit1和bit0设置为11 */
write(val, GPIO_OSPEEDR_PI);

/* 6. 将GPIO_0设置为上拉关闭*/
val = readl(GPIO_PUPDR_PI);
val &= ~(0x3 << 0); /* 将bit1和bit0清零 */
val |= (0x1 << 0); /* 将bit1和bit0设置为10 */
write(val, GPIO_PUPDR_PI);

/* 7. 默认输出低电平，打开LED灯 */
val = readl(GPIO_BSRR_PI);
val &= ~(0x1 << 16); /* 将bit16清零 */
val |= (0x1 << 16); /* 将bit16设置为1 */
write(val, GPIO_BSRR_PI);







