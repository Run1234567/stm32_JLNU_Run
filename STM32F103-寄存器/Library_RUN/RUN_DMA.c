#include "RUN_DMA.h"

// 
// 上图展示了 DMA 控制器的核心架构：
// DMA 作为一个“总线主控 (Bus Master)”，直接挂载在 AHB 总线上。
// 它可以在没有 CPU 干预的情况下，在 外设<->内存 或 内存<->内存 之间高速搬运数据。
// 这大大释放了 CPU 的算力，使其可以专注于逻辑运算。

// ==============================================================================
// 核心配置函数
// ==============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      通用 DMA 配置函数 (寄存器版)
// 参数说明      DMAy_Channelx   具体通道 (如 DMA1_Channel1)
//              periph_addr     外设地址 (如 &USART1->DR 或 &ADC1->DR)
//              memory_addr     内存地址 (如 数组首地址)
//              buffer_size     传输数据量 (CNDTR)
//              direction       传输方向 (P2M, M2P, M2M)
//              width           数据宽度 (8/16/32位)
//              mode            循环模式/单次模式
//-------------------------------------------------------------------------------------------------------------------
void RUN_DMA_Config(DMA_Channel_TypeDef* DMAy_Channelx, 
                    uint32_t periph_addr, 
                    uint32_t memory_addr, 
                    uint16_t buffer_size,
                    RUN_DMA_Dir_t direction,
                    RUN_DMA_Width_t width,
                    RUN_DMA_Mode_t mode)
{
    // 用于暂存 CCR 寄存器的配置值
    uint32_t tmpreg = 0;

    // 1. 开启 DMA 时钟
    // 直接操作 RCC 的 AHBENR 寄存器
    // 为了保持逻辑一致，我们同时开启 DMA1 和 DMA2 的时钟 (若芯片支持)
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
#ifdef RCC_AHBENR_DMA2EN
    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
#endif

    // 2. 复位通道 & 必须先失能
    // 清除 CCR 寄存器中的 EN 位 (Bit 0)
    DMAy_Channelx->CCR &= ~DMA_CCR1_EN;
    
    // 为了达到 DeInit 的效果，我们将核心寄存器清零
    DMAy_Channelx->CCR   = 0;
    DMAy_Channelx->CNDTR = 0;
    DMAy_Channelx->CPAR  = 0;
    DMAy_Channelx->CMAR  = 0;
    
    // 清除该通道可能存在的挂起中断标志 (GIF, TCIF, HTIF, TEIF)
    // 注意：DMA1 和 DMA2 的中断标志清除寄存器地址不同，
    // 这里为了通用性，暂不执行全局清除，仅在使能前依靠硬件复位状态。
    // (在标准寄存器操作中，配置阶段通常不需要特意清除标志位，除非开启了中断)

    // 3. 填充基础地址
    // CPAR: 外设地址寄存器
    // CMAR: 内存地址寄存器
    // CNDTR: 数据传输数量寄存器
    DMAy_Channelx->CPAR  = periph_addr;
    DMAy_Channelx->CMAR  = memory_addr;
    DMAy_Channelx->CNDTR = (uint16_t)buffer_size;

    // 4. 解析数据宽度 (Data Width)
    // 配置 PSIZE (Bit 9:8) 和 MSIZE (Bit 11:10)
    // 00:8-bit, 01:16-bit, 10:32-bit
    // 假设源和目标宽度一致
    if (width == RUN_DMA_WIDTH_16BIT) 
    {
        tmpreg |= (0x01 << 8);  // PSIZE = 16bit
        tmpreg |= (0x01 << 10); // MSIZE = 16bit
    }
    else if (width == RUN_DMA_WIDTH_32BIT) 
    {
        tmpreg |= (0x02 << 8);  // PSIZE = 32bit
        tmpreg |= (0x02 << 10); // MSIZE = 32bit
    }
    // 8BIT 默认为 00，无需操作

    // 
    // 上图展示了 "地址自增" 的关键概念：
    // - 外设通常是单一寄存器 (如 DR)，读写都在同一个地址，所以地址不自增。
    // - 内存通常是数组 (Buffer)，写完一个 byte 需要往后移动，所以地址要自增。

    // 5. 核心逻辑：根据方向配置地址自增规则
    // DIR (Bit 4): 0=从外设读, 1=从内存读
    // PINC (Bit 6): 外设增量模式
    // MINC (Bit 7): 内存增量模式
    // MEM2MEM (Bit 14): 存储器到存储器模式

    if (direction == RUN_DMA_DIR_P2M) 
    {
        // 外设 -> 内存
        // DIR = 0 (PeripheralSRC)
        // PINC = 0 (Disable)
        // MINC = 1 (Enable)
        tmpreg |= DMA_CCR1_MINC; 
    }
    else if (direction == RUN_DMA_DIR_M2P) 
    {
        // 内存 -> 外设
        // DIR = 1 (PeripheralDST)
        // PINC = 0
        // MINC = 1
        tmpreg |= (DMA_CCR1_DIR | DMA_CCR1_MINC);
    }
    else // RUN_DMA_DIR_M2M
    {
        // 内存 -> 内存
        // DIR = 0 (硬件忽略此位，但通常保持默认)
        // PINC = 1 (源地址自增) -> 这里 STM32 把 Flash 视为 "外设"
        // MINC = 1 (目的地址自增)
        // MEM2MEM = 1
        tmpreg |= (DMA_CCR1_MEM2MEM | DMA_CCR1_PINC | DMA_CCR1_MINC);
    }

    // 
    // 上图展示了 "循环模式 (Circular Mode)" 的工作方式：
    // 当 CNDTR 计数减为 0 时，DMA 会自动重装载初值，并回到 buffer 的开头继续传输。

    // 6. 解析循环模式
    // CIRC (Bit 5)
    if (mode == RUN_DMA_MODE_CIRCULAR)
    {
        tmpreg |= DMA_CCR1_CIRC;
    }

    // 7. 优先级
    // PL (Bit 13:12): 00=Low, 01=Medium, 10=High, 11=VeryHigh
    // 这里配置为 High (10)
    tmpreg |= DMA_CCR1_PL_1; // PL_1 对应 Bit 13 置 1
    
    // 8. 写入寄存器
    // 将配置好的值写入 CCR，注意此时 EN 位是 0 (未启动)
    DMAy_Channelx->CCR = tmpreg;
}

// ==============================================================================
// 控制函数
// ==============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      开启并重置 DMA
// 参数说明      DMAy_Channelx   DMA通道
// 返回参数      void
//-------------------------------------------------------------------------------------------------------------------
void RUN_DMA_Enable(DMA_Channel_TypeDef* DMAy_Channelx)
{
    // 1. 必须先失能
    // 清除 CCR 的 EN 位 (Bit 0)
    DMAy_Channelx->CCR &= ~DMA_CCR1_EN;
    
    // 2. 重新设置传输数量
    // 这里的逻辑保留原代码：读取当前的 CNDTR 并写回去。
    // 注意：如果之前 DMA 已完成 (CNDTR=0)，这里写回的也是 0，DMA 将不会启动传输。
    // 如果是用作“暂停后恢复”，这是有效的。
    DMAy_Channelx->CNDTR = DMAy_Channelx->CNDTR; 
    
    // 3. 使能 DMA
    // 置位 CCR 的 EN 位
    DMAy_Channelx->CCR |= DMA_CCR1_EN;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      关闭 DMA
// 参数说明      DMAy_Channelx   DMA通道
// 返回参数      void
//-------------------------------------------------------------------------------------------------------------------
void RUN_DMA_Disable(DMA_Channel_TypeDef* DMAy_Channelx)
{
    // 清除 CCR 的 EN 位
    DMAy_Channelx->CCR &= ~DMA_CCR1_EN;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      获取当前剩余数据量
// 参数说明      DMAy_Channelx   DMA通道
// 返回参数      uint16_t        剩余未传输的数据个数
//-------------------------------------------------------------------------------------------------------------------
uint16_t RUN_DMA_GetCurrDataCounter(DMA_Channel_TypeDef* DMAy_Channelx)
{
    // 直接读取 CNDTR 寄存器
    return (uint16_t)(DMAy_Channelx->CNDTR);
}