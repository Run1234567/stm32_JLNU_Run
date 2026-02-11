#ifndef _RUN_UART_H_
#define _RUN_UART_H_

// =================================================================
// 必须包含 STM32 标准头文件，否则编译器不认识 uint32_t 和 USART_TypeDef
// =================================================================
#include "stm32f10x.h" 

// --- 1. 枚举定义 ---
typedef enum {
    UART1_TX_PA9_RX_PA10,
    UART1_TX_PB6_RX_PB7,
    UART2_TX_PA2_RX_PA3,
    UART2_TX_PD5_RX_PD6,
    UART3_TX_PB10_RX_PB11,
    UART3_TX_PC10_RX_PC11,
    UART3_TX_PD8_RX_PD9,
    UART4_TX_PC10_RX_PC11, 
    UART5_TX_PC12_RX_PD2,
    UART_PIN_MAX
} UART_PIN_enum;

// --- 2. 硬件信息结构体 ---
typedef struct {
    USART_TypeDef* uart_base;   // 串口基地址
    uint32_t       uart_rcc;    // 串口时钟
    uint8_t        is_apb2;     // 时钟总线 1:APB2, 0:APB1

    GPIO_TypeDef* tx_port;      // TX 端口
    uint16_t       tx_pin;      // TX 引脚
    uint32_t       tx_rcc;      // TX 时钟

    GPIO_TypeDef* rx_port;      // RX 端口
    uint16_t       rx_pin;      // RX 引脚
    uint32_t       rx_rcc;      // RX 时钟

    uint32_t       remap_val;   // 重映射值
} uart_info_t;

// 声明配置表，方便外部(如中断服务函数)调用
extern const uart_info_t uart_cfg[UART_PIN_MAX];

// --- 3. 函数声明 ---

// 初始化串口
// enable_it: 1=开启接收中断(需要在main里写处理函数), 0=仅轮询发送
void RUN_uart_init(UART_PIN_enum uart_pin, uint32_t baud_rate, uint8_t enable_it);

// 发送相关
void RUN_uart_putchar(UART_PIN_enum uart_pin, uint8_t dat);
void RUN_uart_putstr(UART_PIN_enum uart_pin, char *str);
void RUN_uart_putbuff(UART_PIN_enum uart_pin, uint8_t *buff, uint32_t len);

// 查询相关
uint8_t RUN_uart_getchar(UART_PIN_enum uart_pin);
uint8_t RUN_uart_query(UART_PIN_enum uart_pin);

#endif