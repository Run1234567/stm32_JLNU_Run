#include "RUN_UART.h" 
#include <stdio.h>  // <--- 新增：必须包含此头文件，否则不认识 FILE 类型

// 声明系统时钟变量 (通常在 system_stm32f10x.c 中定义)
extern uint32_t SystemCoreClock;

// ===============================================================================
// 核心配置表 (完全保留原定义，利用其中的基地址和掩码)
// ===============================================================================
const uart_info_t uart_cfg[UART_PIN_MAX] = {
    // --- UART1 (APB2) ---
    {USART1, RCC_APB2Periph_USART1, 1, GPIOA, GPIO_Pin_9,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_10, RCC_APB2Periph_GPIOA, 0}, 
    {USART1, RCC_APB2Periph_USART1, 1, GPIOB, GPIO_Pin_6,  RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_7,  RCC_APB2Periph_GPIOB, GPIO_Remap_USART1}, 
    
    // --- UART2 (APB1) ---
    {USART2, RCC_APB1Periph_USART2, 0, GPIOA, GPIO_Pin_2,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_3,  RCC_APB2Periph_GPIOA, 0},
    {USART2, RCC_APB1Periph_USART2, 0, GPIOD, GPIO_Pin_5,  RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_6,  RCC_APB2Periph_GPIOD, GPIO_Remap_USART2},
    
    // --- UART3 (APB1) ---
    {USART3, RCC_APB1Periph_USART3, 0, GPIOB, GPIO_Pin_10, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_11, RCC_APB2Periph_GPIOB, 0},
    {USART3, RCC_APB1Periph_USART3, 0, GPIOC, GPIO_Pin_10, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_11, RCC_APB2Periph_GPIOC, GPIO_PartialRemap_USART3},
    {USART3, RCC_APB1Periph_USART3, 0, GPIOD, GPIO_Pin_8,  RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_9,  RCC_APB2Periph_GPIOD, GPIO_FullRemap_USART3},
    
    // --- UART4/5 (APB1) ---
    {UART4,  RCC_APB1Periph_UART4,  0, GPIOC, GPIO_Pin_10, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_11, RCC_APB2Periph_GPIOC, 0},
    {UART5,  RCC_APB1Periph_UART5,  0, GPIOC, GPIO_Pin_12, RCC_APB2Periph_GPIOC, GPIOD, GPIO_Pin_2,  RCC_APB2Periph_GPIOD, 0}
};

//-------------------------------------------------------------------------------------------------------------------
// 内部辅助：获取中断通道号
//-------------------------------------------------------------------------------------------------------------------
static uint8_t get_uart_irqn(USART_TypeDef* uart_base)
{
    if (uart_base == USART1) return USART1_IRQn;
    if (uart_base == USART2) return USART2_IRQn;
    if (uart_base == USART3) return USART3_IRQn;
    if (uart_base == UART4)  return UART4_IRQn;
    if (uart_base == UART5)  return UART5_IRQn;
    return 0; 
}

//-------------------------------------------------------------------------------------------------------------------
// 内部辅助：配置 GPIO 模式 (寄存器版)
//-------------------------------------------------------------------------------------------------------------------
static void uart_gpio_config(GPIO_TypeDef* port, uint16_t pin_mask, uint8_t mode_config)
{
    // 寻找引脚索引 (0~15)
    uint32_t pin_idx = 0;
    while (!((pin_mask >> pin_idx) & 0x01)) 
    {
        pin_idx++;
        if (pin_idx > 15) return;
    }

    // 配置寄存器
    if (pin_idx < 8) {
        port->CRL &= ~(0xF << (pin_idx * 4));      // 清除
        port->CRL |=  (mode_config << (pin_idx * 4)); // 写入
    } else {
        port->CRH &= ~(0xF << ((pin_idx - 8) * 4));
        port->CRH |=  (mode_config << ((pin_idx - 8) * 4));
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      通用串口初始化配置 (寄存器版)
//-------------------------------------------------------------------------------------------------------------------
void RUN_uart_init(UART_PIN_enum uart_pin, uint32_t baud_rate, uint8_t enable_it)
{
    if (uart_pin >= UART_PIN_MAX) return;

    USART_TypeDef* UARTx = uart_cfg[uart_pin].uart_base;

    // 1. 开启时钟 
    RCC->APB2ENR |= (uart_cfg[uart_pin].tx_rcc | uart_cfg[uart_pin].rx_rcc);
    if (uart_cfg[uart_pin].is_apb2)
        RCC->APB2ENR |= uart_cfg[uart_pin].uart_rcc;
    else
        RCC->APB1ENR |= uart_cfg[uart_pin].uart_rcc;

    // 2. 引脚重映射
    if (uart_cfg[uart_pin].remap_val != 0) {
        RCC->APB2ENR |= RCC_APB2Periph_AFIO; 
        AFIO->MAPR   |= uart_cfg[uart_pin].remap_val;
    }

    // 3. GPIO 配置
    // TX: 复用推挽输出 (AF_PP) -> 0xB
    uart_gpio_config(uart_cfg[uart_pin].tx_port, uart_cfg[uart_pin].tx_pin, 0xB);
    // RX: 浮空输入 (IN_FLOATING) -> 0x4
    uart_gpio_config(uart_cfg[uart_pin].rx_port, uart_cfg[uart_pin].rx_pin, 0x4);

    // 4. USART 参数配置
    uint32_t pclk = (uart_cfg[uart_pin].is_apb2) ? SystemCoreClock : (SystemCoreClock / 2);
    UARTx->BRR = (pclk + baud_rate / 2) / baud_rate;

    uint32_t cr1_val = 0x200C; // UE, TE, RE
    
    // 5. 中断配置
    if (enable_it)
    {
        cr1_val |= 0x0020; // RXNEIE
        uint8_t irq_channel = get_uart_irqn(UARTx);
        NVIC->IP[irq_channel] = 0x10; 
        NVIC->ISER[irq_channel >> 5] = (1 << (irq_channel & 0x1F));
    }
    
    UARTx->CR1 = cr1_val; 
}

// -----------------------------------------------------------
// 发送与查询函数
// -----------------------------------------------------------

void RUN_uart_putchar(UART_PIN_enum uart_pin, uint8_t dat) {
    if (uart_pin >= UART_PIN_MAX) return;
    USART_TypeDef* UARTx = uart_cfg[uart_pin].uart_base;
    while ((UARTx->SR & (1<<7)) == 0); 
    UARTx->DR = dat;
}

void RUN_uart_putstr(UART_PIN_enum uart_pin, char *str) {
    while (*str) RUN_uart_putchar(uart_pin, *str++);
}

void RUN_uart_putbuff(UART_PIN_enum uart_pin, uint8_t *buff, uint32_t len) {
    for(uint32_t i = 0; i < len; i++) RUN_uart_putchar(uart_pin, buff[i]);
}

uint8_t RUN_uart_getchar(UART_PIN_enum uart_pin) {
    if (uart_pin >= UART_PIN_MAX) return 0;
    USART_TypeDef* UARTx = uart_cfg[uart_pin].uart_base;
    while ((UARTx->SR & (1<<5)) == 0);
    return (uint8_t)(UARTx->DR & 0xFF);
}

uint8_t RUN_uart_query(UART_PIN_enum uart_pin) {
    if (uart_pin >= UART_PIN_MAX) return 0;
    USART_TypeDef* UARTx = uart_cfg[uart_pin].uart_base;
    return (UARTx->SR & (1<<5)) ? 1 : 0;
}

// -----------------------------------------------------------
// Printf 重定向
// -----------------------------------------------------------
#if 1
#pragma import(__use_no_semihosting)
struct __FILE { int handle; };
FILE __stdout;       // 需要 <stdio.h>
void _sys_exit(int x) { x = x; }

int fputc(int ch, FILE *f) {  // 需要 <stdio.h>
    // 默认使用枚举值为 0 的串口 (通常是 UART1)
    RUN_uart_putchar((UART_PIN_enum)0, (uint8_t)ch);
    return ch;
}
#endif