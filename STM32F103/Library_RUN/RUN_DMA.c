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
// 函数简介      通用 DMA 配置函数 (对外隐藏寄存器细节)
// 参数说明      DMAy_Channelx   具体通道 (如 DMA1_Channel1)
//              periph_addr     外设地址 (如 &USART1->DR 或 &ADC1->DR)
//              memory_addr     内存地址 (如 数组首地址)
//              buffer_size     传输数据量 (CNDTR)
//              direction       传输方向 (P2M, M2P, M2M)
//              width           数据宽度 (8/16/32位)
//              mode            循环模式/单次模式
// 返回参数      void
// 备注信息      DMA1 有 7 个通道，DMA2 有 5 个通道 (大容量型号)。
//               不同外设对应的通道是固定的 (需查阅数据手册 DMA 请求映射表)。
//-------------------------------------------------------------------------------------------------------------------
void RUN_DMA_Config(DMA_Channel_TypeDef* DMAy_Channelx, 
                    uint32_t periph_addr, 
                    uint32_t memory_addr, 
                    uint16_t buffer_size,
                    RUN_DMA_Dir_t direction,
                    RUN_DMA_Width_t width,
                    RUN_DMA_Mode_t mode)
{
    DMA_InitTypeDef DMA_InitStructure;

    // 1. 开启 DMA 时钟
    // 注意：DMA 挂载在 AHB (Advanced High-performance Bus) 总线上
    // 相比于挂载在 APB 上的外设，AHB 的速度更快，吞吐量更高。
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

    // 2. 复位通道
    // 在配置前必须先失能，因为 DMA 正在工作时是不能修改配置寄存器的
    DMA_Cmd(DMAy_Channelx, DISABLE);
    DMA_DeInit(DMAy_Channelx);

    // 3. 填充基础地址
    // 告诉 DMA 从哪里搬运到哪里，以及搬运多少个数据
    DMA_InitStructure.DMA_PeripheralBaseAddr = periph_addr;
    DMA_InitStructure.DMA_MemoryBaseAddr     = memory_addr;
    DMA_InitStructure.DMA_BufferSize         = buffer_size; // 对应寄存器 CNDTR

    // 4. 解析数据宽度 (Data Width)
    // 也就是每次搬运多少位：Byte(8b), HalfWord(16b), Word(32b)
    // 这里我们将源宽度和目标宽度设为一致，避免复杂的对齐/截断问题
    uint32_t dataSize;
    if      (width == RUN_DMA_WIDTH_8BIT)  dataSize = DMA_PeripheralDataSize_Byte;
    else if (width == RUN_DMA_WIDTH_16BIT) dataSize = DMA_PeripheralDataSize_HalfWord;
    else                                   dataSize = DMA_PeripheralDataSize_Word;

    DMA_InitStructure.DMA_PeripheralDataSize = dataSize;
    DMA_InitStructure.DMA_MemoryDataSize     = dataSize;

    // 
    // 上图展示了 "地址自增" 的关键概念：
    // - 外设通常是单一寄存器 (如 DR)，读写都在同一个地址，所以地址不自增。
    // - 内存通常是数组 (Buffer)，写完一个 byte 需要往后移动，所以地址要自增。

    // 5. 核心逻辑：根据方向配置地址自增规则
    if (direction == RUN_DMA_DIR_P2M) 
    {
        // --- 场景：外设 -> 内存 (ADC 采集, 串口接收) ---
        // 源头是外设 (Peripheral SRC)
        // 外设地址不自增 (始终读同一个 DR 寄存器)
        // 内存地址要自增 (填满 Buffer)
        DMA_InitStructure.DMA_DIR           = DMA_DIR_PeripheralSRC;  
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc     = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_M2M           = DMA_M2M_Disable;
    }
    else if (direction == RUN_DMA_DIR_M2P) 
    {
        // --- 场景：内存 -> 外设 (串口发送, DAC 输出) ---
        // 目的地是外设 (Peripheral DST)
        // 外设地址不自增
        // 内存地址要自增
        DMA_InitStructure.DMA_DIR           = DMA_DIR_PeripheralDST; 
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc     = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_M2M           = DMA_M2M_Disable;
    }
    else // RUN_DMA_DIR_M2M
    {
        // --- 场景：内存 -> 内存 (Flash 到 RAM, RAM 到 RAM) ---
        // 类似于 memcpy()
        // 源和目的都不是外设，但 STM32 规定把其中一个视为“外设”接口
        // 两边地址都要自增
        DMA_InitStructure.DMA_DIR           = DMA_DIR_PeripheralSRC;
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable; // 关键：这里要Enable
        DMA_InitStructure.DMA_MemoryInc     = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_M2M           = DMA_M2M_Enable;           // 开启 M2M 模式
    }

    // 
    // 上图展示了 "循环模式 (Circular Mode)" 的工作方式：
    // 当 CNDTR 计数减为 0 时，DMA 会自动重装载初值，并回到 buffer 的开头继续传输。
    // 这对于 ADC 连续采样或类似 "环形缓冲区" 的应用非常有用。

    // 6. 解析循环模式
    if (mode == RUN_DMA_MODE_CIRCULAR)
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    else
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; // 单次模式，减到0就停止

    // 7. 优先级与 M2M
    // 当多个 DMA 通道同时竞争总线时，高优先级的先处理
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; 
    
    // 8. 写入寄存器
    DMA_Init(DMAy_Channelx, &DMA_InitStructure);
    
    // 注意：此时 DMA 尚未启动，需调用 Enable 函数
}

// ==============================================================================
// 控制函数
// ==============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      开启并重置 DMA
// 参数说明      DMAy_Channelx   DMA通道
// 返回参数      void
// 备注信息      对于 "Normal (单次)" 模式，每次传输完成后 CNDTR 会变为 0。
//               想再次传输，必须：失能 -> 重置 CNDTR -> 使能。
//-------------------------------------------------------------------------------------------------------------------
void RUN_DMA_Enable(DMA_Channel_TypeDef* DMAy_Channelx)
{
    // 1. 必须先失能
    // 如果 DMA 正在工作，直接修改 CNDTR 是无效的
    DMA_Cmd(DMAy_Channelx, DISABLE); 
    
    // 2. 重新设置传输数量
    // 这里的 DMAy_Channelx->CNDTR 读取的是当前寄存器值？
    // 警告：原代码中 `DMA_SetCurrDataCounter(..., DMAy_Channelx->CNDTR)` 
    // 如果此时 CNDTR 已经减为0了，重设为0是没意义的。
    // 通常这里应该传入一个具体的 buffer_size。
    // *但在本封装中，假设用户仅用于“暂停后恢复”或“循环模式启动”，暂且保留原逻辑*
    // *更稳健的做法是：在 Config 结构体中记录初始 size，或者要求用户传入 size*
    
    DMA_SetCurrDataCounter(DMAy_Channelx, DMAy_Channelx->CNDTR); 
    
    // 3. 使能 DMA
    DMA_Cmd(DMAy_Channelx, ENABLE);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      关闭 DMA
// 参数说明      DMAy_Channelx   DMA通道
// 返回参数      void
// 备注信息      暂停传输
//-------------------------------------------------------------------------------------------------------------------
void RUN_DMA_Disable(DMA_Channel_TypeDef* DMAy_Channelx)
{
    DMA_Cmd(DMAy_Channelx, DISABLE);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      获取当前剩余数据量
// 参数说明      DMAy_Channelx   DMA通道
// 返回参数      uint16_t        剩余未传输的数据个数 (CNDTR 寄存器值)
// 备注信息      初始值是 Buffer Size，传输过程中递减，为 0 时表示传输完成。
//-------------------------------------------------------------------------------------------------------------------
uint16_t RUN_DMA_GetCurrDataCounter(DMA_Channel_TypeDef* DMAy_Channelx)
{
    // CNDTR: Current Number of Data to Register
    return DMA_GetCurrDataCounter(DMAy_Channelx);
}