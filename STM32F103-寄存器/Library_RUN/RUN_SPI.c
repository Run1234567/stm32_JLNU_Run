#include "RUN_SPI.h"

// ==============================================================================
// 内部辅助函数
// ==============================================================================

/**
 * @brief  根据枚举值获取 STM32 硬件 SPI 外设地址
 */
static SPI_TypeDef* Get_SPIx(RUN_SPI_Port_t port)
{
    if(port == RUN_SPI_1_PA5_PA6_PA7 || port == RUN_SPI_1_PB3_PB4_PB5_REMAP)
        return SPI1;
    else if(port == RUN_SPI_2_PB13_PB14_PB15)
        return SPI2;
    else
        return SPI3;
}

/**
 * @brief  简易的 GPIO 寄存器配置函数 (用于替代 GPIO_Init)
 * @param  GPIOx: GPIOA/GPIOB/GPIOC...
 * @param  Pin:   0~15
 * @param  Mode:  对应 CRL/CRH 的 4位配置值 (CNF[1:0] + MODE[1:0])
 * 例如: 
 * 推挽复用 50MHz (AF_PP) -> CNF=10, MODE=11 -> 0xB
 * 浮空输入 (IN_FLOATING) -> CNF=01, MODE=00 -> 0x4
 * 上拉/下拉输入 (IPU/IPD) -> CNF=10, MODE=00 -> 0x8 (需配合 ODR)
 */
// ==============================================================================
// 内部辅助函数
// ==============================================================================

static void RUN_SPI_GPIO_Config(GPIO_TypeDef* GPIOx, uint8_t Pin, uint8_t Mode)
{
    // 修改点：加上 volatile 关键字
    volatile uint32_t *cr_reg; 
    uint8_t shift;

    if (Pin < 8) 
    {
        // CRL 是 volatile uint32_t，所以指针也必须是 volatile 的
        cr_reg = &GPIOx->CRL;
        shift = Pin * 4;
    } 
    else 
    {
        cr_reg = &GPIOx->CRH;
        shift = (Pin - 8) * 4;
    }

    // 清除原来的 4 位配置
    *cr_reg &= ~(0x0F << shift);
    // 写入新的配置
    *cr_reg |= ((uint32_t)Mode << shift);
}

// ==============================================================================
// 初始化函数 (寄存器版本)
// ==============================================================================

void RUN_SPI_Init(RUN_SPI_Port_t port_group)
{
    SPI_TypeDef* SPIx = Get_SPIx(port_group);
    
    // 1. GPIO 与 时钟配置
    // GPIO 模式: 
    // SCK/MOSI -> 复用推挽 50MHz (0xB)
    // MISO     -> 浮空输入 (0x4) 或 上拉输入 (0x8)
    
    if(port_group == RUN_SPI_1_PA5_PA6_PA7)
    {
        // 开启 SPI1(Bit12) 和 GPIOA(Bit2) 时钟
        RCC->APB2ENR |= (1 << 12) | (1 << 2);

        // PA5 (SCK), PA7 (MOSI) -> AF_PP (0xB)
        RUN_SPI_GPIO_Config(GPIOA, 5, 0xB);
        RUN_SPI_GPIO_Config(GPIOA, 7, 0xB);
        // PA6 (MISO) -> Input Floating (0x4)
        RUN_SPI_GPIO_Config(GPIOA, 6, 0x4);
    }
    else if(port_group == RUN_SPI_1_PB3_PB4_PB5_REMAP)
    {
        // 开启 SPI1, GPIOB, AFIO 时钟
        RCC->APB2ENR |= (1 << 12) | (1 << 3) | (1 << 0);

        // 解除 JTAG，保留 SWD (AFIO->MAPR [26:24] = 010)
        AFIO->MAPR &= ~(7 << 24);
        AFIO->MAPR |=  (2 << 24);

        // 开启 SPI1 重映射 (Bit 0)
        AFIO->MAPR |= (1 << 0);

        // PB3 (SCK), PB5 (MOSI) -> AF_PP
        RUN_SPI_GPIO_Config(GPIOB, 3, 0xB);
        RUN_SPI_GPIO_Config(GPIOB, 5, 0xB);
        // PB4 (MISO)
        RUN_SPI_GPIO_Config(GPIOB, 4, 0x4);
    }
    else if(port_group == RUN_SPI_2_PB13_PB14_PB15)
    {
        // 开启 SPI2 (APB1 Bit 14)
        RCC->APB1ENR |= (1 << 14);
        // 开启 GPIOB (APB2 Bit 3)
        RCC->APB2ENR |= (1 << 3);

        // PB13 (SCK), PB15 (MOSI) -> AF_PP
        RUN_SPI_GPIO_Config(GPIOB, 13, 0xB);
        RUN_SPI_GPIO_Config(GPIOB, 15, 0xB);
        // PB14 (MISO)
        RUN_SPI_GPIO_Config(GPIOB, 14, 0x4);
    }
    else if(port_group == RUN_SPI_3_PB3_PB4_PB5)
    {
        // 开启 SPI3 (APB1 Bit 15)
        RCC->APB1ENR |= (1 << 15);
        // 开启 GPIOB, AFIO
        RCC->APB2ENR |= (1 << 3) | (1 << 0);

        // 解除 JTAG
        AFIO->MAPR &= ~(7 << 24);
        AFIO->MAPR |=  (2 << 24);

        // PB3 (SCK), PB5 (MOSI) -> AF_PP
        RUN_SPI_GPIO_Config(GPIOB, 3, 0xB);
        RUN_SPI_GPIO_Config(GPIOB, 5, 0xB);
        // PB4 (MISO)
        RUN_SPI_GPIO_Config(GPIOB, 4, 0x4);
    }
    else if(port_group == RUN_SPI_3_PC10_PC11_PC12_REMAP)
    {
        // 开启 SPI3 (APB1 Bit 15)
        RCC->APB1ENR |= (1 << 15);
        // 开启 GPIOC, AFIO
        RCC->APB2ENR |= (1 << 4) | (1 << 0);

        // SPI3 重映射 (AFIO->MAPR Bit 28)
        AFIO->MAPR |= (1 << 28);

        // PC10 (SCK), PC12 (MOSI) -> AF_PP
        RUN_SPI_GPIO_Config(GPIOC, 10, 0xB);
        RUN_SPI_GPIO_Config(GPIOC, 12, 0xB);
        // PC11 (MISO)
        RUN_SPI_GPIO_Config(GPIOC, 11, 0x4);
    }

    // 2. SPI 寄存器配置 (CR1)
    // -----------------------------------------------------
    // Bit 15-14: BIDIMODE/BIDIOE (默认0, 全双工)
    // Bit 13   : CRCEN (0)
    // Bit 11   : DFF (0: 8-bit)
    // Bit 9    : SSM (1: 软件管理 NSS)
    // Bit 8    : SSI (1: 内部片选信号，必须置1否则Master模式失效)
    // Bit 7    : LSBFIRST (0: MSB First)
    // Bit 6    : SPE (1: 开启 SPI)
    // Bit 5-3  : BR (Baud Rate) -> 设为 111 (256分频, 初始慢速)
    // Bit 2    : MSTR (1: Master)
    // Bit 1    : CPOL (0: 空闲低电平)
    // Bit 0    : CPHA (1: 第二个边沿采样, 1Edge 对应 CPHA=0, 2Edge 对应 CPHA=1)
    // -----------------------------------------------------
    
    // 复位 CR1
    SPIx->CR1 = 0;

    // 配置: Master, SSM=1, SSI=1, BaudRate=Div256 (慢速启动), CPOL=0, CPHA=0 (模式0)
    // 注意：原标准库代码注释写 CPHA=2Edge 但下面赋值写了 CPHA_1Edge (模式0/1)。
    // 这里我配置为模式0 (CPOL=0, CPHA=0)，这是最通用的 Flash 模式。
    // 如果需要模式3，改为 CPOL=1, CPHA=1。
    
    SPIx->CR1 |= (1 << 9); // SSM = 1
    SPIx->CR1 |= (1 << 8); // SSI = 1 (Master 必须置位)
    SPIx->CR1 |= (1 << 2); // MSTR = 1
    
    // 设置分频：初始给 8 分频 (010) 兼容原代码
    // 原代码 SPI_BaudRatePrescaler_8 (0x10) -> Bit 4=1 -> 010
    SPIx->CR1 |= (2 << 3); 

    // CPHA/CPOL 配置 (模式 0)
    SPIx->CR1 &= ~(1 << 1); // CPOL = 0
    SPIx->CR1 &= ~(1 << 0); // CPHA = 0 (1Edge)

    // 3. 使能 SPI
    SPIx->CR1 |= (1 << 6); // SPE = 1

    // 4. 启动传输：发送 Dummy Byte
    RUN_SPI_ReadWriteByte(port_group, 0xFF);
}

// ==============================================================================
// 数据传输函数 (寄存器版本)
// ==============================================================================

uint8_t RUN_SPI_ReadWriteByte(RUN_SPI_Port_t port_group, uint8_t TxData)
{
    SPI_TypeDef* SPIx = Get_SPIx(port_group);
    uint16_t retry = 0;

    // 1. 等待发送缓冲区为空 (TXE, Bit 1)
    while((SPIx->SR & (1 << 1)) == 0)
    {
        if(++retry > 2000) return 0;
    }

    // 2. 写入数据 (DR)
    SPIx->DR = TxData;

    // 3. 等待接收缓冲区非空 (RXNE, Bit 0)
    retry = 0;
    while((SPIx->SR & (1 << 0)) == 0)
    {
        if(++retry > 2000) return 0;
    }

    // 4. 读取数据
    return (uint8_t)SPIx->DR;
}

void RUN_SPI_WriteByte(RUN_SPI_Port_t port_group, uint8_t data)
{
    RUN_SPI_ReadWriteByte(port_group, data);
}

uint8_t RUN_SPI_ReadByte(RUN_SPI_Port_t port_group)
{
    return RUN_SPI_ReadWriteByte(port_group, 0xFF);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      动态调整 SPI 速度 (寄存器操作)
// 参数说明      speed: 0=低速(256分频), 1=中速(16分频), 其他=高速(2分频)
//-------------------------------------------------------------------------------------------------------------------
void RUN_SPI_SetSpeed(RUN_SPI_Port_t port_group, uint8_t speed)
{
    SPI_TypeDef* SPIx = Get_SPIx(port_group);
    
    // 修改 CR1 前建议先关闭 SPE (Bit 6)
    SPIx->CR1 &= ~(1 << 6);

    // 清除 BR[2:0] (Bit 5:3)
    // 掩码: ~(0x7 << 3) = ~(111000)
    SPIx->CR1 &= ~(7 << 3);

    if(speed == 0)
    {
        // 256 分频 (111)
        SPIx->CR1 |= (7 << 3);
    }
    else if(speed == 1)
    {
        // 16 分频 (011)
        SPIx->CR1 |= (3 << 3);
    }
    else
    {
        // 2 分频 (000)
        SPIx->CR1 |= (0 << 3);
    }

    // 重新开启 SPE
    SPIx->CR1 |= (1 << 6);
}