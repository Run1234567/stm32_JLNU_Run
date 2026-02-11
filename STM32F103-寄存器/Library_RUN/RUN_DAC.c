#include "RUN_DAC.h"

/**
  * @brief  DAC 模块初始化 (寄存器版)
  * @param  channel: 通道选择 (RUN_DAC_CH1_PA4 或 RUN_DAC_CH2_PA5)
  * @retval None
  * @note   配置为 12 位右对齐、开启输出缓冲、关闭触发 (软件模式)。
  */
void RUN_DAC_Init(RUN_DAC_Channel_t channel)
{
    // 1. 开启时钟
    RCC->APB2ENR |= RCC_APB2Periph_GPIOA; // 开启 GPIOA 时钟
    RCC->APB1ENR |= RCC_APB1Periph_DAC;   // 开启 DAC 时钟

    // 2. 配置 GPIO 为模拟输入 (AIN)
    // STM32F10x CRL/CRH 寄存器配置：模拟输入模式为 0x0
    if (channel == RUN_DAC_CH1_PA4)
    {
        GPIOA->CRL &= ~(0x0F << (4 * 4)); // 清除 PA4 配置
        GPIOA->CRL |= (0x00 << (4 * 4));  // 设为模拟输入
    }
    else if (channel == RUN_DAC_CH2_PA5)
    {
        GPIOA->CRL &= ~(0x0F << (5 * 4)); // 清除 PA5 配置
        GPIOA->CRL |= (0x00 << (5 * 4));  // 设为模拟输入
    }
    else return;

    // 3. 配置 DAC 控制寄存器 (DAC_CR)
    if (channel == RUN_DAC_CH1_PA4)
    {
        // 通道 1 配置：
        // EN1=1 (开启), BOFF1=0 (开启缓冲), TEN1=0 (软件模式/触发禁用)
        DAC->CR |= (1 << 0);  // 使能通道 1 (EN1)
        DAC->CR &= ~(1 << 1); // 开启输出缓冲 (BOFF1=0 代表开启)
        DAC->CR &= ~(1 << 2); // 禁用触发 (TEN1=0)
        
        DAC->DHR12R1 = 0;     // 初始输出 0V
    }
    else if (channel == RUN_DAC_CH2_PA5)
    {
        // 通道 2 配置：
        // EN2=1 (开启), BOFF2=0 (开启缓冲), TEN2=0 (软件模式/触发禁用)
        DAC->CR |= (1 << 16);  // 使能通道 2 (EN2)
        DAC->CR &= ~(1 << 17); // 开启输出缓冲 (BOFF2=0)
        DAC->CR &= ~(1 << 18); // 禁用触发 (TEN2=0)
        
        DAC->DHR12R2 = 0;      // 初始输出 0V
    }
}

/**
  * @brief  直接设置 DAC 寄存器值 (寄存器版)
  * @param  channel: 通道选择
  * @param  val: 12位 DAC 数值 (0 ~ 4095)
  * @retval None
  */
void RUN_DAC_Set_Value(RUN_DAC_Channel_t channel, uint16_t val)
{
    if(val > 4095) val = 4095; // 硬件限制为 12 bit
    
    if (channel == RUN_DAC_CH1_PA4)
    {
        // 写入通道 1 的 12 位右对齐数据保持寄存器
        DAC->DHR12R1 = val;
    }
    else if (channel == RUN_DAC_CH2_PA5)
    {
        // 写入通道 2 的 12 位右对齐数据保持寄存器
        DAC->DHR12R2 = val;
    }
}

/**
  * @brief  设置 DAC 输出电压 (寄存器版)
  * @param  channel: 通道选择
  * @param  vol: 目标电压 (0.0f ~ 3.3f)
  * @retval None
  * @note   公式: DOR = (Vol / 3.3) * 4096
  */
void RUN_DAC_Set_Vol(RUN_DAC_Channel_t channel, float vol)
{
    uint16_t temp_val;
    
    if(vol > 3.3f) vol = 3.3f;
    if(vol < 0.0f) vol = 0.0f;
    
    // 计算 12 位数值
    temp_val = (uint16_t)(vol * 4096 / 3.3f);
    
    RUN_DAC_Set_Value(channel, temp_val);
}