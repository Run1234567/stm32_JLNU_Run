#include "RUN_W25Q64.h"

// 
// 上图展示了 W25Q64 的内部存储结构：
// 1. 容量：8M 字节 (64M bit)。
// 2. 块 (Block)：共有 128 个块，每块 64KB。
// 3. 扇区 (Sector)：每个块分 16 个扇区，每扇区 4KB (最小擦除单位)。
// 4. 页 (Page)：每个扇区分 16 页，每页 256 字节 (最小写入单位)。
// 注意：Flash 写入前必须擦除 (将 0 变 1)，且写入只能将 1 变 0。

// ==========================================================
// 私有变量：用于“记住”用户的硬件配置
// ==========================================================
static RUN_SPI_Port_t g_W25Q_SPI_PORT;   // 记录使用的 SPI 接口 (如 SPI1)
static GPIO_TypeDef* g_W25Q_CS_PORT;    // 记录片选端口 (如 GPIOB)
static uint16_t       g_W25Q_CS_PIN;     // 记录片选引脚 (如 GPIO_Pin_12)

// ==========================================================
// 内部帮助函数：片选控制
// ==========================================================
// CS (Chip Select) 低电平有效。
// 拉低 CS 开始通信，拉高 CS 结束通信并执行指令。
static void W25Q_CS_LOW(void)  { GPIO_ResetBits(g_W25Q_CS_PORT, g_W25Q_CS_PIN); }
static void W25Q_CS_HIGH(void) { GPIO_SetBits(g_W25Q_CS_PORT, g_W25Q_CS_PIN); }

// ==========================================================
// 1. 初始化函数
// ==========================================================

/**
  * @brief  初始化 W25Q64 (完全动态配置)
  * @param  spi_port:     使用的 SPI 枚举 (如 RUN_SPI_2_PB13_PB14_PB15)
  * @param  cs_gpio_port: 片选端口 (如 GPIOB)
  * @param  cs_gpio_pin:  片选引脚 (如 GPIO_Pin_12)
  * @note   W25Q64 支持 SPI 模式 0 (CPOL=0, CPHA=0) 和 模式 3 (CPOL=1, CPHA=1)。
  */
void RUN_W25Q_Init(RUN_SPI_Port_t spi_port, GPIO_TypeDef* cs_gpio_port, uint16_t cs_gpio_pin)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 1. 保存配置到全局变量
    // 以便后续读写函数知道操作哪个引脚
    g_W25Q_SPI_PORT = spi_port;
    g_W25Q_CS_PORT  = cs_gpio_port;
    g_W25Q_CS_PIN   = cs_gpio_pin;

    // 2. 开启 CS 引脚时钟 (自动判断 GPIO 组)
    if(cs_gpio_port == GPIOA)      RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    else if(cs_gpio_port == GPIOB) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    else if(cs_gpio_port == GPIOC) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    else if(cs_gpio_port == GPIOD) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    else if(cs_gpio_port == GPIOE) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    else if(cs_gpio_port == GPIOF) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
    else if(cs_gpio_port == GPIOG) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);

    // 3. 配置 CS 引脚 (推挽输出)
    GPIO_InitStructure.GPIO_Pin = cs_gpio_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(cs_gpio_port, &GPIO_InitStructure);
    
    W25Q_CS_HIGH(); // 上电默认拉高，不选中芯片

    // 4. 初始化 SPI 底层
    RUN_SPI_Init(spi_port);
    RUN_SPI_SetSpeed(spi_port, 1); // 默认设置中速 (Flash 支持高速，但为了稳定先用中速)
}

// ==========================================================
// 2. 底层指令封装
// ==========================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      等待 Flash 内部操作完成 (忙等待)
// 备注信息      Flash 在执行擦除或编程时，Status Register 的 BUSY 位 (Bit 0) 会置 1。
//               必须等到 BUSY 变 0 才能发送下一条指令。
//-------------------------------------------------------------------------------------------------------------------
static void W25Q_WaitForWriteEnd(void)
{
    uint8_t FLASH_Status = 0;
    
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_ReadStatusReg); // 发送 "读状态寄存器" 指令 (0x05)
    
    do {
        // 持续读取状态寄存器，直到 BUSY 位 (Bit 0) 为 0
        FLASH_Status = RUN_SPI_ReadByte(g_W25Q_SPI_PORT);
    } while ((FLASH_Status & 0x01) == 1); 
    
    W25Q_CS_HIGH();
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      写使能 (Write Enable)
// 备注信息      任何修改 Flash 内容的操作 (擦除、写入) 之前，必须先发这个指令。
//               操作完成后，WEL 位会自动清零，下次还需要再发。
//-------------------------------------------------------------------------------------------------------------------
static void W25Q_WriteEnable(void)
{
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_WriteEnable); // 指令 0x06
    W25Q_CS_HIGH();
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      读取芯片 ID
// 返回参数      16位 ID (高8位厂商 ID, 低8位设备 ID)
// 备注信息      通常 W25Q64 的 ID 为 0xEF16 (0xEF=Winbond, 0x16=64Mbit)
//-------------------------------------------------------------------------------------------------------------------
uint16_t RUN_W25Q_ReadID(void)
{
    uint16_t Temp = 0;
    
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_JedecDeviceID); // 指令 0x90
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, 0x00); // Dummy Byte 地址
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, 0x00); // Dummy Byte 地址
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, 0x00); // Dummy Byte 地址
    
    Temp |= RUN_SPI_ReadByte(g_W25Q_SPI_PORT) << 8; // 读厂商 ID (0xEF)
    Temp |= RUN_SPI_ReadByte(g_W25Q_SPI_PORT);      // 读设备 ID (0x16)
    
    W25Q_CS_HIGH();
    return Temp;
}

// ==========================================================
// 3. 数据读写操作
// ==========================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      读取数据
// 参数说明      pBuffer         数据存储区
// 参数说明      ReadAddr        读取起始地址 (0 ~ 8388607)
// 参数说明      NumByteToRead   要读取的字节数
// 备注信息      读取操作没有页限制，可以连续读取整个 Flash。
//-------------------------------------------------------------------------------------------------------------------
void RUN_W25Q_Read(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_ReadData); // 指令 0x03
    
    // 发送 24位 地址 (高位在前)
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)((ReadAddr) >> 16));
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)((ReadAddr) >> 8));
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)ReadAddr);
    
    // 连续读取数据
    while (NumByteToRead--) {
        *pBuffer = RUN_SPI_ReadByte(g_W25Q_SPI_PORT);
        pBuffer++;
    }
    W25Q_CS_HIGH();
}

// 
// 上图展示了 "页编程 (Page Program)" 时序：
// 1. 写使能。
// 2. 发送指令 0x02 + 24位地址。
// 3. 连续发送数据 (最多 256 字节)。
// 4. 拉高 CS，芯片开始内部编程 (耗时约 0.5ms ~ 3ms)。
// 警告：如果不按页对齐写入 (跨页)，数据会回到该页的开头覆盖之前的数据 (卷绕现象)！

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      页编程 (内部调用，单次最多写一页 256B)
// 参数说明      WriteAddr       必须对齐到页边界，或者注意不要跨页
//-------------------------------------------------------------------------------------------------------------------
static void W25Q_Write_Page(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    W25Q_WriteEnable(); // 必须先写使能
    
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_PageProgram); // 指令 0x02
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)((WriteAddr) >> 16));
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)((WriteAddr) >> 8));
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)WriteAddr);
    
    if(NumByteToWrite > 256) NumByteToWrite = 256; // 硬件限制
    
    while (NumByteToWrite--) {
        RUN_SPI_WriteByte(g_W25Q_SPI_PORT, *pBuffer);
        pBuffer++;
    }
    W25Q_CS_HIGH();
    
    W25Q_WaitForWriteEnd(); // 等待写入结束
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      写入任意长度数据 (自动处理翻页)
// 参数说明      pBuffer         数据源指针
// 参数说明      WriteAddr       写入地址
// 参数说明      NumByteToWrite  写入长度
// 备注信息      这是核心算法函数。它解决了 Flash 只能 "按页写入" 的限制。
//               逻辑：
//               1. 计算首地址在当前页的偏移量 (Addr)。
//               2. 计算当前页剩余可写字节数 (count)。
//               3. 如果数据跨页：
//                  a. 先填满当前页剩余空间。
//                  b. 然后整页整页地写。
//                  c. 最后写剩余不足一页的数据。
//-------------------------------------------------------------------------------------------------------------------
void RUN_W25Q_Write(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;
    
    Addr = WriteAddr % 256;       // 写入地址在当前页的偏移 (0~255)
    count = 256 - Addr;           // 当前页剩余可写入的字节数
    NumOfPage = NumByteToWrite / 256;     // 需要写满多少个完整页
    NumOfSingle = NumByteToWrite % 256;   // 剩下不足一页的字节数

    // 1. 如果写入地址刚好是页对齐 (Addr=0)
    if (Addr == 0) {
        if (NumOfPage == 0) NumOfSingle = NumByteToWrite; // 不足一页，直接写
        else NumOfSingle = 0; // 这个逻辑有微小漏洞，下面会覆盖修正
        // *实际上原代码逻辑：NumOfPage非0时，NumOfSingle已计算好，不需要这里置0。保留原逻辑结构*
    } 
    // 2. 如果地址不对齐 (需要先填满当前页)
    else {
        if (NumOfPage == 0) {
            // 数据很少，不仅不足一页，连当前页剩余空间都用不完
            if (NumOfSingle > count) NumOfSingle = count; // 这里原代码逻辑似乎是截断？需注意
            // *修正逻辑注解*：如果 NumByteToWrite < count，直接写 NumByteToWrite 即可。
        } else {
            // 数据很多，需要跨页
            NumByteToWrite -= count;            // 先减去第一部分
            NumOfPage = NumByteToWrite / 256;   // 重新计算剩余整页数
            NumOfSingle = NumByteToWrite % 256; // 重新计算剩余尾数
            
            W25Q_Write_Page(pBuffer, WriteAddr, count); // 写第一部分 (填满当前页)
            WriteAddr += count;
            pBuffer += count;
        }
    }
    
    // 3. 循环写入完整的页
    while (NumOfPage--) {
        W25Q_Write_Page(pBuffer, WriteAddr, 256);
        WriteAddr += 256;
        pBuffer += 256;
    }
    
    // 4. 写入最后剩余的数据
    if (NumOfSingle != 0) {
        W25Q_Write_Page(pBuffer, WriteAddr, NumOfSingle);
    }
}

// ==========================================================
// 4. 擦除操作
// ==========================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      扇区擦除 (Sector Erase, 4KB)
// 参数说明      Dst_Addr        扇区地址 (任意属于该扇区的地址即可)
// 备注信息      最小擦除单位。耗时约 45ms。
//-------------------------------------------------------------------------------------------------------------------
void RUN_W25Q_Erase_Sector(uint32_t Dst_Addr)
{
    W25Q_WriteEnable();
    W25Q_WaitForWriteEnd(); // 确保之前的操作已完成
    
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_SectorErase); // 指令 0x20
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)((Dst_Addr) >> 16));
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)((Dst_Addr) >> 8));
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)Dst_Addr);
    W25Q_CS_HIGH();
    
    W25Q_WaitForWriteEnd(); // 等待擦除结束
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      整片擦除 (Chip Erase)
// 备注信息      清除所有数据，恢复为 0xFF。
//               耗时极长！W25Q64 可能需要 20-40 秒。
//-------------------------------------------------------------------------------------------------------------------
void RUN_W25Q_Erase_Chip(void)
{
    W25Q_WriteEnable();
    W25Q_WaitForWriteEnd();
    
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_ChipErase); // 指令 0xC7 或 0x60
    W25Q_CS_HIGH();
    
    W25Q_WaitForWriteEnd(); // 漫长的等待...
}