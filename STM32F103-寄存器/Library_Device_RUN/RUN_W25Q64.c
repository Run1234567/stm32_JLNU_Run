#include "RUN_W25Q64.h"
#include "RUN_Gpio.h"
static RUN_SPI_Port_t g_W25Q_SPI_PORT;
static GPIO_TypeDef* g_W25Q_CS_PORT;
static uint16_t       g_W25Q_CS_PIN;

static void W25Q_CS_LOW(void)  { GPIO_ResetBits(g_W25Q_CS_PORT, g_W25Q_CS_PIN); }
static void W25Q_CS_HIGH(void) { GPIO_SetBits(g_W25Q_CS_PORT, g_W25Q_CS_PIN); }

/**
  * @brief  初始化 W25Q64 存储器 (使用 RUN_Gpio 库简化版)
  * @param  spi_port: 使用的 SPI 硬件接口枚举
  * @param  cs_pin: 片选引脚枚举 (例如：B12, A4 等，对应 RUN_GPIO_enum)
  * @retval None
  */
void RUN_W25Q_Init(RUN_SPI_Port_t spi_port, RUN_GPIO_enum cs_pin)
{
    // 1. 保存配置到全局变量
    // 注意：此处需要修改全局变量类型或通过 gpio_cfg 获取底层指针以适配后续的 W25Q_CS_LOW/HIGH
    g_W25Q_SPI_PORT = spi_port;
    g_W25Q_CS_PORT  = gpio_cfg[cs_pin].port; 
    g_W25Q_CS_PIN   = gpio_cfg[cs_pin].pin;

    // 2. 使用 RUN_Gpio 库初始化 CS 引脚
    // 配置为：推挽输出 (GPO)，默认电平为高 (1) 不选中芯片
    RUN_gpio_init(cs_pin, GPO, 1);

    // 3. 初始化 SPI 底层硬件
    RUN_SPI_Init(spi_port);
    RUN_SPI_SetSpeed(spi_port, 1); 
}

/**
  * @brief  内部阻塞等待 Flash 操作完成
  * @retval None
  * @note   通过轮询状态寄存器 BUSY 位实现，确保擦除或编程执行完毕
  */
static void W25Q_WaitForWriteEnd(void)
{
    uint8_t FLASH_Status = 0;
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_ReadStatusReg); 
    do {
        FLASH_Status = RUN_SPI_ReadByte(g_W25Q_SPI_PORT);
    } while ((FLASH_Status & 0x01) == 1); 
    W25Q_CS_HIGH();
}

/**
  * @brief  发送写使能指令
  * @retval None
  * @note   在执行擦除、页编程等写操作前必须调用
  */
static void W25Q_WriteEnable(void)
{
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_WriteEnable); 
    W25Q_CS_HIGH();
}

/**
  * @brief  读取 Flash 芯片的 JEDEC ID
  * @retval 16位芯片ID (高8位厂商ID，低8位设备ID)
  * @note   常见返回值 W25Q64 为 0xEF16
  */
uint16_t RUN_W25Q_ReadID(void)
{
    uint16_t Temp = 0;
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_JedecDeviceID); 
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, 0x00); 
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, 0x00); 
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, 0x00); 
    Temp |= RUN_SPI_ReadByte(g_W25Q_SPI_PORT) << 8; 
    Temp |= RUN_SPI_ReadByte(g_W25Q_SPI_PORT);      
    W25Q_CS_HIGH();
    return Temp;
}

/**
  * @brief  从指定地址开始读取数据
  * @param  pBuffer: 存储读取内容的缓冲区指针
  * @param  ReadAddr: 起始读取地址 (0 ~ 0x7FFFFF)
  * @param  NumByteToRead: 要读取的字节长度
  * @retval None
  */
void RUN_W25Q_Read(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_ReadData); 
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)((ReadAddr) >> 16));
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)((ReadAddr) >> 8));
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)ReadAddr);
    while (NumByteToRead--) {
        *pBuffer = RUN_SPI_ReadByte(g_W25Q_SPI_PORT);
        pBuffer++;
    }
    W25Q_CS_HIGH();
}

/**
  * @brief  内部页编程函数 (单次最多写入256字节)
  * @param  pBuffer: 源数据缓冲区指针
  * @param  WriteAddr: 写入起始地址
  * @param  NumByteToWrite: 写入长度 (最大256)
  * @retval None
  * @note   需确保不跨越页边界 (256字节对齐)
  */
static void W25Q_Write_Page(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    W25Q_WriteEnable(); 
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_PageProgram); 
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)((WriteAddr) >> 16));
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)((WriteAddr) >> 8));
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)WriteAddr);
    if(NumByteToWrite > 256) NumByteToWrite = 256; 
    while (NumByteToWrite--) {
        RUN_SPI_WriteByte(g_W25Q_SPI_PORT, *pBuffer);
        pBuffer++;
    }
    W25Q_CS_HIGH();
    W25Q_WaitForWriteEnd(); 
}

/**
  * @brief  向 Flash 写入任意长度数据 (带自动换页逻辑)
  * @param  pBuffer: 源数据缓冲区指针
  * @param  WriteAddr: 写入起始地址
  * @param  NumByteToWrite: 写入字节长度
  * @retval None
  * @note   该函数仅负责写入，调用前必须确保目标区域已擦除
  */
void RUN_W25Q_Write(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;
    Addr = WriteAddr % 256;
    count = 256 - Addr;
    NumOfPage = NumByteToWrite / 256;
    NumOfSingle = NumByteToWrite % 256;

    if (Addr == 0) {
        if (NumOfPage == 0) NumOfSingle = NumByteToWrite;
    } else {
        if (NumOfPage == 0) {
            if (NumOfSingle > count) NumOfSingle = count;
        } else {
            NumByteToWrite -= count;
            NumOfPage = NumByteToWrite / 256;
            NumOfSingle = NumByteToWrite % 256;
            W25Q_Write_Page(pBuffer, WriteAddr, count);
            WriteAddr += count;
            pBuffer += count;
        }
    }
    while (NumOfPage--) {
        W25Q_Write_Page(pBuffer, WriteAddr, 256);
        WriteAddr += 256;
        pBuffer += 256;
    }
    if (NumOfSingle != 0) {
        W25Q_Write_Page(pBuffer, WriteAddr, NumOfSingle);
    }
}

/**
  * @brief  扇区擦除 (4KB 擦除)
  * @param  Dst_Addr: 目标扇区内的任意地址
  * @retval None
  * @note   Flash 写入前必须擦除，一个扇区为 4KB
  */
void RUN_W25Q_Erase_Sector(uint32_t Dst_Addr)
{
    W25Q_WriteEnable();
    W25Q_WaitForWriteEnd(); 
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_SectorErase); 
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)((Dst_Addr) >> 16));
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)((Dst_Addr) >> 8));
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, (uint8_t)Dst_Addr);
    W25Q_CS_HIGH();
    W25Q_WaitForWriteEnd(); 
}

/**
  * @brief  全片擦除
  * @retval None
  * @note   耗时极长，执行期间不可断电，擦除后所有位恢复为 0xFF
  */
void RUN_W25Q_Erase_Chip(void)
{
    W25Q_WriteEnable();
    W25Q_WaitForWriteEnd();
    W25Q_CS_LOW();
    RUN_SPI_WriteByte(g_W25Q_SPI_PORT, W25X_ChipErase); 
    W25Q_CS_HIGH();
    W25Q_WaitForWriteEnd(); 
}