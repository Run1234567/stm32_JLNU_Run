#include "RUN_SPI.h"

// 
// 上图展示了 SPI 的物理拓扑：
// 1. 主机 (Master) 通过 SCK 产生时钟。
// 2. MOSI (Master Out Slave In) 发送数据。
// 3. MISO (Master In Slave Out) 接收数据。
// 4. NSS (片选) 用于选中从机 (本驱动使用软件控制 GPIO 作为片选)。

// ==============================================================================
// 内部辅助函数
// ==============================================================================

/**
 * @brief  根据枚举值获取 STM32 硬件 SPI 外设地址
 * @return SPI1, SPI2 或 SPI3
 */
static SPI_TypeDef* Get_SPIx(RUN_SPI_Port_t port)
{
    if(port == RUN_SPI_1_PA5_PA6_PA7 || port == RUN_SPI_1_PB3_PB4_PB5_REMAP)
        return SPI1; // APB2 总线 (最高 72MHz)
    else if(port == RUN_SPI_2_PB13_PB14_PB15)
        return SPI2; // APB1 总线 (最高 36MHz)
    else
        return SPI3; // APB1 总线
}

// ==============================================================================
// 初始化函数
// ==============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      SPI 模块初始化
// 参数说明      port_group      端口组合枚举 (指定使用哪一组 IO 口)
// 返回参数      void
// 使用示例      RUN_SPI_Init(RUN_SPI_1_PA5_PA6_PA7);
// 备注信息      默认配置：主机模式, 模式0 (CPOL=0, CPHA=1Edge), 8位数据, 软件片选
//-------------------------------------------------------------------------------------------------------------------
void RUN_SPI_Init(RUN_SPI_Port_t port_group)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
    SPI_TypeDef* SPIx = Get_SPIx(port_group);

    // 1. 通用 GPIO 配置
    // SCK (时钟) 和 MOSI (输出) 需要配置为 "复用推挽输出" (AF_PP)
    // 这样引脚的控制权才会被转交给 SPI 硬件外设
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    // 2. 差异化引脚配置 (根据 port_group 选择)
    
    // --- 情况 A: SPI1 标准引脚 (PA5/6/7) ---
    if(port_group == RUN_SPI_1_PA5_PA6_PA7)
    {
        // 开启 SPI1 和 GPIOA 时钟
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

        // SCK=PA5, MOSI=PA7 (复用推挽)
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        // MISO=PA6 (浮空输入)
        // 从机发回的数据通过此引脚读入
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }
    // --- 情况 B: SPI1 重映射引脚 (PB3/4/5) ---
    else if(port_group == RUN_SPI_1_PB3_PB4_PB5_REMAP)
    {
        // 【关键】必须开启 AFIO 时钟用于重映射
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
        
        // 
        // 上图展示了 JTAG/SWD 引脚分布。
        // PB3 是 JTDO，PB4 是 JNTRST。默认上电后它们是 JTAG 功能，不能当普通 IO 或 SPI 使用。
        // 必须执行 "JTAGDisable" 才能释放这两个引脚给 SPI 用。
        GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
        
        // 开启 SPI1 重映射
        GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);            

        // SCK=PB3, MOSI=PB5
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        // MISO=PB4
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    // --- 情况 C: SPI2 标准引脚 (PB13/14/15) ---
    else if(port_group == RUN_SPI_2_PB13_PB14_PB15)
    {
        // SPI2 挂载在 APB1 总线上
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

        // SCK=PB13, MOSI=PB15
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        // MISO=PB14
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    // --- 情况 D: SPI3 标准引脚 (PB3/4/5) ---
    // 注意：这里也是 PB3/4/5，但对应的是 SPI3，且不需要重映射，只需要关 JTAG
    else if(port_group == RUN_SPI_3_PB3_PB4_PB5)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
        
        // 同样需要释放 JTAG 引脚
        GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 

        // SCK=PB3, MOSI=PB5
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        // MISO=PB4
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    // --- 情况 E: SPI3 重映射引脚 (PC10/11/12) ---
    else if(port_group == RUN_SPI_3_PC10_PC11_PC12_REMAP)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
        
        // 开启 SPI3 重映射
        GPIO_PinRemapConfig(GPIO_Remap_SPI3, ENABLE);

        // SCK=PC10, MOSI=PC12
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_12;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        // MISO=PC11
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
    }

    // 
    // 上图展示了 SPI 的 4 种工作模式。本驱动配置为：
    // CPOL=0 (空闲时 SCK 为低电平)
    // CPHA=1 (在第 2 个边沿采样，即上升沿发送，下降沿接收，视具体情况而定)
    // 注意：如果是 W25Qxx Flash，通常支持模式 0 (0,0) 和模式 3 (1,1)。

    // 3. SPI 参数配置
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // 全双工 (同时收发)
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;       // 主机模式
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;   // 8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;          // 时钟极性：空闲低
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;        // 时钟相位：第2个边沿采样 (注意：您的原代码是 1Edge，这里建议核对从机手册，通常通用是 2Edge 或 1Edge)
    // *修正注*：根据您的原代码保留 `SPI_CPHA_1Edge`，对应模式0或1，需确保从机匹配。
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; 
    
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;           // 软件控制片选 (不使用硬件 NSS 引脚)
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // 预分频 8
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;  // 高位先出
    SPI_InitStructure.SPI_CRCPolynomial = 7;            // CRC 多项式 (不开启 CRC 时无效)

    SPI_Init(SPIx, &SPI_InitStructure);
    SPI_Cmd(SPIx, ENABLE); // 使能 SPI
    
    // 启动传输：发送一个 Dummy Byte 激活时钟，确保总线空闲
    RUN_SPI_ReadWriteByte(port_group, 0xff); 
}

// ==============================================================================
// 数据传输函数
// ==============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      SPI 底层字节交换
// 参数说明      port_group      端口组
// 参数说明      TxData          要发送的数据
// 返回参数      uint8_t         接收到的数据
// 备注信息      SPI 是基于 "移位寄存器" 的环形结构。
//               发送一个字节的同时，必然会移入(接收)一个字节。
//-------------------------------------------------------------------------------------------------------------------
uint8_t RUN_SPI_ReadWriteByte(RUN_SPI_Port_t port_group, uint8_t TxData)
{
    SPI_TypeDef* SPIx = Get_SPIx(port_group);
    uint8_t retry = 0;

    // 1. 等待发送缓冲区为空 (TXE: Transmit Buffer Empty)
    // 只有 TXE 置 1，才能把数据写入 DR 寄存器
    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)
    {
        if(++retry > 200) return 0; // 超时保护
    }
    
    // 2. 写入数据，触发时钟信号
    SPI_I2S_SendData(SPIx, TxData);

    // 3. 等待接收缓冲区非空 (RXNE: Receive Buffer Not Empty)
    // 数据发送完成后，接收到的数据会被移入 DR 寄存器，RXNE 置 1
    retry = 0;
    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET)
    {
        if(++retry > 200) return 0; // 超时保护
    }
    
    // 4. 读取接收到的数据
    return SPI_I2S_ReceiveData(SPIx);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      SPI 仅发送
// 备注信息      本质上还是调用交换函数，只是忽略了返回值
//-------------------------------------------------------------------------------------------------------------------
void RUN_SPI_WriteByte(RUN_SPI_Port_t port_group, uint8_t data)
{
    RUN_SPI_ReadWriteByte(port_group, data);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      SPI 仅读取
// 备注信息      发送 0xFF (Dummy Byte) 以产生时钟信号，从而读取从机数据
//-------------------------------------------------------------------------------------------------------------------
uint8_t RUN_SPI_ReadByte(RUN_SPI_Port_t port_group)
{
    return RUN_SPI_ReadWriteByte(port_group, 0xFF);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      动态调整 SPI 速度
// 参数说明      speed           0=低速(256分频), 1=中速(16分频), 其他=高速(2分频)
// 返回参数      void
// 备注信息      用于初始化时低速通信，初始化完成后切换高速通信 (如 SD 卡驱动)
//-------------------------------------------------------------------------------------------------------------------
void RUN_SPI_SetSpeed(RUN_SPI_Port_t port_group, uint8_t speed)
{
    SPI_TypeDef* SPIx = Get_SPIx(port_group);
    
    // 修改寄存器前必须先失能 SPI
    SPI_Cmd(SPIx, DISABLE);
    
    // 清除 CR1 寄存器中的 BR[2:0] 位 (Bit 5:3)
    // 0xFFC7 = 1111 1111 1100 0111
    SPIx->CR1 &= 0xFFC7; 
    
    if(speed == 0)      SPIx->CR1 |= SPI_BaudRatePrescaler_256; // 最慢
    else if(speed == 1) SPIx->CR1 |= SPI_BaudRatePrescaler_16;  // 中等
    else                SPIx->CR1 |= SPI_BaudRatePrescaler_2;   // 最快 (APB/2)
    
    SPI_Cmd(SPIx, ENABLE);
}