#ifndef _RUN_CAN_H_
#define _RUN_CAN_H_

#include "stm32f10x.h"
#include "RUN_Delay.h" // 使用你定义的 uint32 等类型

// --- 1. 枚举定义 ---

// 引脚方案枚举：包含 F103 的所有映射方案
typedef enum {
    CAN1_RX_PA11_TX_PA12,   // 默认引脚 (注意：与 USB 冲突)
    CAN1_RX_PB8_TX_PB9,     // 部分重映射 (野火霸道板常用)
    CAN1_RX_PD0_TX_PD1,     // 完全重映射
    CAN_PIN_MAX
} CAN_PIN_enum;

// 通信模式枚举
typedef enum {
    RUN_CAN_MODE_NORMAL   = CAN_Mode_Normal,   // 正常模式
    RUN_CAN_MODE_LOOPBACK = CAN_Mode_LoopBack  // 回环模式
} RUN_CAN_MODE_enum;

// 过滤器工作模式枚举
typedef enum {
    RUN_CAN_FILTER_ALL,   // 全通模式：接收所有 ID
    RUN_CAN_FILTER_MASK,  // 掩码模式：匹配特定位
    RUN_CAN_FILTER_LIST   // 列表模式：精确匹配特定 ID
} RUN_CAN_FILTER_enum;

// --- 2. 硬件信息结构体 (参考 RUN_UART) ---
typedef struct {
    CAN_TypeDef* can_base;   // CAN 基地址
    uint32_t     can_rcc;    // CAN 时钟
    GPIO_TypeDef* tx_port;    // TX 端口
    uint16_t     tx_pin;     // TX 引脚
    uint32_t     tx_rcc;     // TX 时钟
    GPIO_TypeDef* rx_port;    // RX 端口
    uint16_t     rx_pin;     // RX 引脚
    uint32_t     rx_rcc;     // RX 时钟
    uint32_t     remap;      // 重映射宏定义
} can_info_t;

// --- 3. 全局变量 ---
extern uint8_t  CAN_Rx_Buffer[8];
extern uint8_t  CAN_Rx_Flag;

// --- 4. 函数声明 ---
void    RUN_can_init(CAN_PIN_enum can_pin, uint32 baud_rate, RUN_CAN_MODE_enum can_mode, RUN_CAN_FILTER_enum filter_mode, uint32 f_id, uint32 f_mask);
uint8_t RUN_can_send_msg(CAN_PIN_enum can_pin, uint32 id, uint8* msg, uint8 len);

#endif