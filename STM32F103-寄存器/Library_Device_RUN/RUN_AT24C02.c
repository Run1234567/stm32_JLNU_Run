#include "RUN_AT24C02.h"
// 假设你的延时函数在这个头文件里，比如 RUN_delay_ms()
#include "RUN_header_file.h" 

/**
  * @brief  初始化 AT24C02
  * @param  dev:  对象句柄
  * @param  scl:  SCL引脚
  * @param  sda:  SDA引脚
  * @param  addr: 设备地址 (通常是 0xA0)
  */
void AT24C02_Init(RUN_AT24C02_t *dev, RUN_GPIO_enum scl, RUN_GPIO_enum sda, uint8_t addr)
{
    dev->Dev_Addr = addr;
    RUN_I2C_Init(&dev->I2C_Bus, scl, sda);
}

/**
  * @brief  写入一个字节
  * @note   EEPROM 写入很慢，内部搬运电荷需要时间。
  * 写入后必须至少延时 5ms，否则无法进行下一次操作。
  * @param  dev:       对象句柄
  * @param  word_addr: 要写入的内存地址 (0 ~ 255)
  * @param  data:      要写入的数据
  */
void AT24C02_WriteByte(RUN_AT24C02_t *dev, uint8_t word_addr, uint8_t data)
{
    RUN_I2C_Start(&dev->I2C_Bus);
    
    // 1. 发送设备地址 (写模式)
    RUN_I2C_SendByte(&dev->I2C_Bus, dev->Dev_Addr);
    RUN_I2C_WaitAck(&dev->I2C_Bus);
    
    // 2. 发送要写入的内存地址
    RUN_I2C_SendByte(&dev->I2C_Bus, word_addr);
    RUN_I2C_WaitAck(&dev->I2C_Bus);
    
    // 3. 发送数据
    RUN_I2C_SendByte(&dev->I2C_Bus, data);
    RUN_I2C_WaitAck(&dev->I2C_Bus);
    
    RUN_I2C_Stop(&dev->I2C_Bus);
    
    // 【关键】必须延时！
    // AT24C02 的写入周期最大为 5ms
    RUN_delay_ms(5); 
}

/**
  * @brief  读取一个字节 (随机读取模式)
  * @param  dev:       对象句柄
  * @param  word_addr: 要读取的内存地址 (0 ~ 255)
  * @return uint8_t:   读取到的数据
  */
uint8_t AT24C02_ReadByte(RUN_AT24C02_t *dev, uint8_t word_addr)
{
    uint8_t data;

    // 第一步：伪写入 (Dummy Write)，目的是把内部指针指到 word_addr
    RUN_I2C_Start(&dev->I2C_Bus);
    RUN_I2C_SendByte(&dev->I2C_Bus, dev->Dev_Addr); // 写设备地址
    RUN_I2C_WaitAck(&dev->I2C_Bus);
    RUN_I2C_SendByte(&dev->I2C_Bus, word_addr);     // 只要告诉它地址，不要发数据
    RUN_I2C_WaitAck(&dev->I2C_Bus);
    
    // 第二步：重启信号，开始读取
    RUN_I2C_Start(&dev->I2C_Bus);
    RUN_I2C_SendByte(&dev->I2C_Bus, dev->Dev_Addr | 1); // 读设备地址 (地址 | 1)
    RUN_I2C_WaitAck(&dev->I2C_Bus);
    
    // 第三步：读取一个字节，并且发送 NACK (0)，告诉芯片读完了
    data = RUN_I2C_ReadByte(&dev->I2C_Bus, 0); 
    
    RUN_I2C_Stop(&dev->I2C_Bus);
    
    return data;
}