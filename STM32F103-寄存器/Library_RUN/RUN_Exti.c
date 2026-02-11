#include "RUN_Exti.h"

// ==============================================================================
// 全局变量管理
// ==============================================================================
static ExtiCallback_t exti_callbacks[16] = {0};

// ==============================================================================
// 内部辅助函数
// ==============================================================================

static uint8_t RUN_get_pin_source(uint16_t gpio_pin)
{
    uint8_t source = 0;
    while ((gpio_pin >> source) != 1) {
        source++;
        if(source > 15) return 0;
    }
    return source;
}

static uint8_t RUN_get_port_source(GPIO_TypeDef* port)
{
    if (port == GPIOA) return 0;
    if (port == GPIOB) return 1;
    if (port == GPIOC) return 2;
    if (port == GPIOD) return 3;
    if (port == GPIOE) return 4;
    if (port == GPIOF) return 5;
    if (port == GPIOG) return 6;
    return 0;
}

// ==============================================================================
// 初始化函数 (寄存器版本)
// ==============================================================================

void RUN_exti_init(RUN_EXTI_Pin_enum exti_pin, 
                   EXTITrigger_TypeDef trigger_mode, 
                   uint8_t pre_priority, 
                   uint8_t sub_priority,
                   ExtiCallback_t callback)
{
    uint8_t pin_source;
    uint8_t port_source;
    IRQn_Type irq_channel;

    if((uint16_t)exti_pin == 0xFFFF) return;

    // 1. 硬件资源转换
    RUN_GPIO_enum pin_enum = (RUN_GPIO_enum)exti_pin;
    GPIO_TypeDef* port = gpio_cfg[pin_enum].port;
    uint16_t pin = gpio_cfg[pin_enum].pin;
    
    // 2. 开启 AFIO 时钟
    // RCC_APB2ENR 寄存器第 0 位是 AFIOEN
    RCC->APB2ENR |= (1 << 0);
    
    // 初始化 GPIO (调用你原有的底层函数)
    RUN_gpio_init(pin_enum, GPI, 0);

    // 3. 计算索引
    pin_source = RUN_get_pin_source(pin);
    port_source = RUN_get_port_source(port);

    // 4. 注册回调函数
    if(pin_source < 16) exti_callbacks[pin_source] = callback;

    // 5. AFIO 线路映射 (GPIO_EXTILineConfig 的寄存器实现)
    // AFIO->EXTICR[0-3] 每 4 位控制一个引脚映射
    AFIO->EXTICR[pin_source >> 2] &= ~(0x0F << (4 * (pin_source & 0x03)));
    AFIO->EXTICR[pin_source >> 2] |= (port_source << (4 * (pin_source & 0x03)));

    // 6. EXTI 外设配置
    // IMR: 中断屏蔽寄存器 (1: 开放中断)
    EXTI->IMR |= pin;
    
    // 先清除边沿配置
    EXTI->RTSR &= ~pin;
    EXTI->FTSR &= ~pin;
    
    // 根据触发模式设置上升沿或下降沿寄存器
    if (trigger_mode == EXTI_Trigger_Rising) {
        EXTI->RTSR |= pin;
    } else if (trigger_mode == EXTI_Trigger_Falling) {
        EXTI->FTSR |= pin;
    } else if (trigger_mode == EXTI_Trigger_Rising_Falling) {
        EXTI->RTSR |= pin;
        EXTI->FTSR |= pin;
    }

    // 7. 匹配 NVIC 中断通道
    if (pin_source == 0)       irq_channel = EXTI0_IRQn;
    else if (pin_source == 1) irq_channel = EXTI1_IRQn;
    else if (pin_source == 2) irq_channel = EXTI2_IRQn;
    else if (pin_source == 3) irq_channel = EXTI3_IRQn;
    else if (pin_source == 4) irq_channel = EXTI4_IRQn;
    else if (pin_source >= 5 && pin_source <= 9)   irq_channel = EXTI9_5_IRQn;
    else if (pin_source >= 10 && pin_source <= 15) irq_channel = EXTI15_10_IRQn;
    else return;

    // 8. NVIC 配置 (寄存器操作)
    // 计算优先级 (针对 STM32F10x 4位优先级分组)
    uint32_t priority = (pre_priority << 4) | (sub_priority & 0x0F);
    priority = (priority << 4) & 0xFF; // 仅高4位有效
    
    NVIC->IP[(uint32_t)irq_channel] = (uint8_t)priority;
    // 使能中断通道 (ISER)
    NVIC->ISER[(uint32_t)irq_channel >> 5] = (1 << ((uint32_t)irq_channel & 0x1F));
}

// ==============================================================================
// 中断服务函数
// ==============================================================================

/**
 * @brief  通用中断处理分发器
 */
static void RUN_EXTI_Handler(uint32_t line, uint8_t index)
{
    // PR (Pending Register): 挂起寄存器。写 1 清除
    if ((EXTI->PR & line) != 0)
    {
        if (exti_callbacks[index]) exti_callbacks[index]();
        
        // 清除挂起标志位 (往 PR 对应位写 1)
        EXTI->PR = line;
    }
}

void EXTI0_IRQHandler(void) { RUN_EXTI_Handler(0x0001, 0); }
void EXTI1_IRQHandler(void) { RUN_EXTI_Handler(0x0002, 1); }
void EXTI2_IRQHandler(void) { RUN_EXTI_Handler(0x0004, 2); }
void EXTI3_IRQHandler(void) { RUN_EXTI_Handler(0x0008, 3); }
void EXTI4_IRQHandler(void) { RUN_EXTI_Handler(0x0010, 4); }

void EXTI9_5_IRQHandler(void)
{
    for(int i=5; i<=9; i++) {
        RUN_EXTI_Handler((1 << i), i);
    }
}

void EXTI15_10_IRQHandler(void)
{
    for(int i=10; i<=15; i++) {
        RUN_EXTI_Handler((1 << i), i);
    }
}