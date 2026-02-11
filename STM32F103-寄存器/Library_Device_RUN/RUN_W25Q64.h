#ifndef _RUN_W25Q64_H_
#define _RUN_W25Q64_H_

#include "stm32f10x.h"
#include "RUN_SPI.h"
#include "RUN_Gpio.h"
// ==========================================================
//  指令集 (无需修改)
// ==========================================================
#define W25X_WriteEnable    0x06 
#define W25X_ReadStatusReg  0x05 
#define W25X_ReadData       0x03 
#define W25X_PageProgram    0x02 
#define W25X_SectorErase    0x20 
#define W25X_ChipErase      0xC7 
#define W25X_JedecDeviceID  0x90 

// ==========================================================
//  函数声明
// ==========================================================

void RUN_W25Q_Init(RUN_SPI_Port_t spi_port, RUN_GPIO_enum cs_pin);

uint16_t RUN_W25Q_ReadID(void);
void RUN_W25Q_Read(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
void RUN_W25Q_Write(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void RUN_W25Q_Erase_Sector(uint32_t Dst_Addr);
void RUN_W25Q_Erase_Chip(void);

#endif