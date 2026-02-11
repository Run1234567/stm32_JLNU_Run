#ifndef __RUN_DMA_H
#define __RUN_DMA_H

#include "stm32f10x.h" // 根据你的芯片型号可能是 stm32f4xx.h

// =============================================================
//  用户配置枚举 (用这些词代替难记的宏定义)
// =============================================================

// 1. 搬运方向
typedef enum {
    RUN_DMA_DIR_P2M = 0, // 外设 -> 内存 (例: ADC采集, 串口接收)
    RUN_DMA_DIR_M2P = 1, // 内存 -> 外设 (例: 串口发送, DAC输出)
    RUN_DMA_DIR_M2M = 2  // 内存 -> 内存 (例: 数组拷贝)
} RUN_DMA_Dir_t;

// 2. 数据宽度 (搬运工手里的箱子大小)
typedef enum {
    RUN_DMA_WIDTH_8BIT  = 0, // 字节 Byte (串口, I2C)
    RUN_DMA_WIDTH_16BIT = 1, // 半字 HalfWord (ADC, DAC)
    RUN_DMA_WIDTH_32BIT = 2  // 全字 Word (int拷贝)
} RUN_DMA_Width_t;

// 3. 循环模式
typedef enum {
    RUN_DMA_MODE_NORMAL   = 0, // 单次模式 (干完活就停，适合串口发送)
    RUN_DMA_MODE_CIRCULAR = 1  // 循环模式 (一直干，适合ADC示波器)
} RUN_DMA_Mode_t;


// =============================================================
//  函数声明
// =============================================================

/**
 * @brief  万能 DMA 配置函数
 * @param  DMAy_Channelx: 硬件通道 (查表! 如 DMA1_Channel1)
 * @param  periph_addr:   外设地址 (源头或目的地，如 &ADC1->DR)
 * @param  memory_addr:   内存地址 (数组首地址)
 * @param  buffer_size:   搬运数据的个数
 * @param  direction:     搬运方向 (P2M / M2P / M2M)
 * @param  width:         数据宽度 (8 / 16 / 32)
 * @param  mode:          模式 (单次 / 循环)
 */
void RUN_DMA_Config(DMA_Channel_TypeDef* DMAy_Channelx, 
                    uint32_t periph_addr, 
                    uint32_t memory_addr, 
                    uint16_t buffer_size,
                    RUN_DMA_Dir_t direction,
                    RUN_DMA_Width_t width,
                    RUN_DMA_Mode_t mode);

// 开启 DMA
void RUN_DMA_Enable(DMA_Channel_TypeDef* DMAy_Channelx);

// 关闭 DMA (修改参数前必须关)
void RUN_DMA_Disable(DMA_Channel_TypeDef* DMAy_Channelx);

// 获取剩余数据量 (用于计算最新数据位置)
uint16_t RUN_DMA_GetCurrDataCounter(DMA_Channel_TypeDef* DMAy_Channelx);

#endif