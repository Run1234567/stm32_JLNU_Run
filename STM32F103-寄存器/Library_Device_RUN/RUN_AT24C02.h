#ifndef _RUN_AT24C02_H_
#define _RUN_AT24C02_H_

#include "stm32f10x.h"
#include "RUN_Gpio.h"
#include "RUN_SoftI2C.h"

// AT24C02 的默认设备地址 (A0/A1/A2 接地时)
// 如果你的 A0/A1/A2 接了 VCC，需要修改这里的低3位
#define AT24C02_ADDR_DEFAULT  0xA0 

// ==========================================================
// AT24C02 对象结构体
// ==========================================================
typedef struct {
    RUN_SoftI2C_Bus_t I2C_Bus; // I2C 总线引脚对象
    uint8_t Dev_Addr;          // 设备地址
} RUN_AT24C02_t;

// ==========================================================
// 函数声明
// ==========================================================

// 初始化
void AT24C02_Init(RUN_AT24C02_t *dev, RUN_GPIO_enum scl, RUN_GPIO_enum sda, uint8_t addr);

// 写一个字节 (地址 0~255)
void AT24C02_WriteByte(RUN_AT24C02_t *dev, uint8_t word_addr, uint8_t data);

// 读一个字节 (地址 0~255)
uint8_t AT24C02_ReadByte(RUN_AT24C02_t *dev, uint8_t word_addr);

#endif