#ifndef _RUN_DS18B20_H_
#define _RUN_DS18B20_H_

#include "RUN_OneWire.h"

// 1. 初始化 (其实就是透传给 OneWire)
void RUN_DS18B20_Init(GPIO_TypeDef* port, uint16_t pin);

// 2. 读取温度
// 返回 float 类型，比如 25.62
float RUN_DS18B20_GetTemp(void);

#endif