#ifndef _RUN_SOFT_I2C_H_
#define _RUN_SOFT_I2C_H_

#include "stm32f10x.h"
#include "RUN_Gpio.h"
#include "RUN_Delay.h"

// I2C 总线结构体
typedef struct {
    RUN_GPIO_enum SCL_Pin; // 时钟脚
    RUN_GPIO_enum SDA_Pin; // 数据脚
} RUN_SoftI2C_Bus_t;

// --- 1. 初始化 ---
void RUN_I2C_Init(RUN_SoftI2C_Bus_t* bus, RUN_GPIO_enum scl, RUN_GPIO_enum sda);

// --- 2. 底层信号 (Start/Stop/Ack) ---
void RUN_I2C_Start(RUN_SoftI2C_Bus_t* bus);
void RUN_I2C_Stop(RUN_SoftI2C_Bus_t* bus);
uint8_t RUN_I2C_WaitAck(RUN_SoftI2C_Bus_t* bus);

// --- 3. 字节读写 ---
void RUN_I2C_SendByte(RUN_SoftI2C_Bus_t* bus, uint8_t byte);
uint8_t RUN_I2C_ReadByte(RUN_SoftI2C_Bus_t* bus, uint8_t ack);

// --- 4. 寄存器读写 (常用接口) ---
void RUN_I2C_WriteReg(RUN_SoftI2C_Bus_t* bus, uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
uint8_t RUN_I2C_ReadReg(RUN_SoftI2C_Bus_t* bus, uint8_t dev_addr, uint8_t reg_addr);

#endif