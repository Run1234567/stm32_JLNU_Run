#ifndef _RUN_FLASH_H_
#define _RUN_FLASH_H_

#include "stm32f10x.h"
#include <string.h>

// ==========================================================
// ZET6 配置 (512KB, Page=2KB)
// ==========================================================
#define FLASH_START_ADDR  0x0807F800  // 最后一页的起始地址
#define FLASH_PAGE_SIZE   2048        // 一页的大小 (字节)

// ==========================================================
// 任意写、任意读函数
// ==========================================================

// 读数据
// offset: 相对起始地址的偏移 (0 ~ 2047)
// pData:  变量的地址
// len:    变量的长度 (sizeof)
void RUN_Flash_Read(uint16_t offset, void *pData, uint16_t len);

// 写数据 (支持随机写入，不影响其他数据)
// 警告：这个函数比较耗时(几十毫秒)，不要在中断里调用
void RUN_Flash_Write(uint16_t offset, void *pData, uint16_t len);

#endif