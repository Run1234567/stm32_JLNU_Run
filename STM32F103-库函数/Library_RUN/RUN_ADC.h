#ifndef _RUN_ADC_H_
#define _RUN_ADC_H_

#include "stm32f10x.h"
#include "RUN_Gpio.h" // 需要用到 GPIO 初始化设为模拟输入

// ==========================================================
// ADC 通道枚举 (对应引脚速查)
// ==========================================================
typedef enum {
    RUN_ADC_CH0_PA0 = ADC_Channel_0,
    RUN_ADC_CH1_PA1 = ADC_Channel_1,
    RUN_ADC_CH2_PA2 = ADC_Channel_2,
    RUN_ADC_CH3_PA3 = ADC_Channel_3,
    RUN_ADC_CH4_PA4 = ADC_Channel_4,
    RUN_ADC_CH5_PA5 = ADC_Channel_5,
    RUN_ADC_CH6_PA6 = ADC_Channel_6,
    RUN_ADC_CH7_PA7 = ADC_Channel_7,
    
    RUN_ADC_CH8_PB0 = ADC_Channel_8,
    RUN_ADC_CH9_PB1 = ADC_Channel_9,
    
    RUN_ADC_CH10_PC0 = ADC_Channel_10,
    RUN_ADC_CH11_PC1 = ADC_Channel_11,
    RUN_ADC_CH12_PC2 = ADC_Channel_12,
    RUN_ADC_CH13_PC3 = ADC_Channel_13,
    RUN_ADC_CH14_PC4 = ADC_Channel_14,
    RUN_ADC_CH15_PC5 = ADC_Channel_15,
    
    // 内部温度传感器 (需要特殊开启)
    RUN_ADC_CH_TEMP  = ADC_Channel_16, 
    // 内部参考电压 (Vrefint)
    RUN_ADC_CH_VREF  = ADC_Channel_17  

} RUN_ADC_Channel_enum;

// ==========================================================
// 函数声明
// ==========================================================

// 1. 初始化 ADC 模块 (全局只需调用一次)
void RUN_ADC_Init(void);

// 2. 配置引脚 (把某个引脚设为模拟输入模式)
//    虽然 RUN_gpio_init 也能做，但这个函数封装了自动查表，更方便
void RUN_ADC_ConfigPin(RUN_ADC_Channel_enum ch);

// 3. 读取原始值 (0 ~ 4095)
uint16_t RUN_ADC_Get_Value(RUN_ADC_Channel_enum ch);

// 4. 读取平均值 (多次采样取平均，更稳)
uint16_t RUN_ADC_Get_Average(RUN_ADC_Channel_enum ch, uint8_t times);

#endif