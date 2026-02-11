#ifndef _RUN_ONEWIRE_H_
#define _RUN_ONEWIRE_H_

#include "stm32f10x.h"
#include "RUN_delay.h" // 必须依赖我们之前写的微秒延时 delay_us()

// ==========================================================
// 函数声明
// ==========================================================

// 初始化 (动态指定引脚)
void RUN_OneWire_Init(GPIO_TypeDef* port, uint16_t pin);

// 1. 复位总线 (发送复位脉冲，检测是否有从机回应)
// 返回值: 0=有人回应(正常), 1=无人回应(异常)
uint8_t RUN_OneWire_Reset(void);

// 2. 写一个字节
void RUN_OneWire_WriteByte(uint8_t data);

// 3. 读一个字节
uint8_t RUN_OneWire_ReadByte(void);

#endif