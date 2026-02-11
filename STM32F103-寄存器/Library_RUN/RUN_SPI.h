#ifndef _RUN_SPI_H_
#define _RUN_SPI_H_

#include "stm32f10x.h"

// ==========================================================
// SPI 引脚组合枚举 (ZET6 全覆盖)
// ==========================================================
typedef enum {
    // --- SPI 1 (APB2 高速) ---
    // SCK=PA5, MISO=PA6, MOSI=PA7 (最推荐，默认)
    RUN_SPI_1_PA5_PA6_PA7 = 0,    

    // SCK=PB3, MISO=PB4, MOSI=PB5 (重映射)
    // 警告：占用 JTAG 口，会自动关闭 JTAG 调试，仅保留 SWD
    RUN_SPI_1_PB3_PB4_PB5_REMAP,  

    // --- SPI 2 (APB1) ---
    // SCK=PB13, MISO=PB14, MOSI=PB15 (ZET6 板载 Flash 通常在这里)
    RUN_SPI_2_PB13_PB14_PB15,     

    // --- SPI 3 (APB1) ---
    // SCK=PB3, MISO=PB4, MOSI=PB5 (默认)
    // 警告：占用 JTAG 口，会自动关闭 JTAG
    RUN_SPI_3_PB3_PB4_PB5,        

    // SCK=PC10, MISO=PC11, MOSI=PC12 (部分型号支持重映射)
    RUN_SPI_3_PC10_PC11_PC12_REMAP 

} RUN_SPI_Port_t;

// ==========================================================
// 函数声明
// ==========================================================

// 1. 初始化 (自动处理时钟、GPIO、重映射)
void RUN_SPI_Init(RUN_SPI_Port_t port_group);

// 2. 底层交换一个字节 (发送数据的同时接收数据)
uint8_t RUN_SPI_ReadWriteByte(RUN_SPI_Port_t port_group, uint8_t TxData);

// 3. [封装] 只发送一个字节 (不关心返回值)
void RUN_SPI_WriteByte(RUN_SPI_Port_t port_group, uint8_t data);

// 4. [封装] 只读取一个字节 (自动发送 0xFF 产生时钟)
uint8_t RUN_SPI_ReadByte(RUN_SPI_Port_t port_group);

// 5. 设置速度 (0=慢, 1=中, 2=快)
void RUN_SPI_SetSpeed(RUN_SPI_Port_t port_group, uint8_t speed);

#endif