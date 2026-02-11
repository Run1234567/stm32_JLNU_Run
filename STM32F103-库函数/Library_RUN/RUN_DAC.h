#ifndef _RUN_DAC_H_
#define _RUN_DAC_H_

#include "stm32f10x.h"

// 定义 DAC 通道枚举
// 强制用户选择，而不是默认
typedef enum {
    RUN_DAC_CH1_PA4 = 1, // 通道1 对应 PA4
    RUN_DAC_CH2_PA5 = 2  // 通道2 对应 PA5
} RUN_DAC_Channel_t;

// ==========================================
// 函数声明
// ==========================================

// 初始化指定的 DAC 通道
// 参数: channel (RUN_DAC_CH1_PA4 或 RUN_DAC_CH2_PA5)
void RUN_DAC_Init(RUN_DAC_Channel_t channel);

// 设置指定通道的电压 (0.0v - 3.3v)
void RUN_DAC_Set_Vol(RUN_DAC_Channel_t channel, float vol);

// 直接设置指定通道的寄存器值 (0 - 4095)
void RUN_DAC_Set_Value(RUN_DAC_Channel_t channel, uint16_t val);

#endif