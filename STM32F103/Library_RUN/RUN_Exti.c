#include "RUN_Exti.h"

// ==============================================================================
// 全局变量管理
// ==============================================================================
// 回调函数指针数组
// 作用：保存用户注册的中断处理函数。
// 索引 0~15 对应 EXTI_Line0 ~ EXTI_Line15
static ExtiCallback_t exti_callbacks[16] = {0};

// 
// 上图展示了外部中断的信号流向：GPIO引脚 -> AFIO选择器 -> EXTI边缘检测 -> NVIC中断控制器 -> CPU响应

// ==============================================================================
// 内部辅助函数
// ==============================================================================

/**
 * @brief  根据 GPIO_Pin 计算引脚源索引 (0~15)
 * @note   标准库的 GPIO_Pin_x 是位掩码 (如 0x0020)，AFIO 需要的是索引值 (如 5)
 */
static uint8_t RUN_get_pin_source(uint16_t gpio_pin)
{
    uint8_t source = 0;
    // 通过移位寻找 '1' 所在的位置
    while ((gpio_pin >> source) != 1) {
        source++;
        if(source > 15) return 0; // 防止死循环
    }
    return source;
}

/**
 * @brief  根据 GPIO_Port 地址计算端口源索引
 * @return 0:GPIOA, 1:GPIOB, ...
 */
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
// 初始化函数
// ==============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      外部中断初始化 (支持回调函数注册)
// 参数说明      exti_pin        引脚枚举 (如 RUN_PIN_A0)
// 参数说明      trigger_mode    触发模式 (EXTI_Trigger_Rising, Falling, Rising_Falling)
// 参数说明      pre_priority    抢占优先级
// 参数说明      sub_priority    子优先级
// 参数说明      callback        中断触发时调用的函数指针
// 返回参数      void
// 使用示例      RUN_exti_init(RUN_PIN_A0, EXTI_Trigger_Falling, 1, 1, my_key_handler);
// 备注信息      【重要】必须开启 AFIO 时钟，否则无法将 GPIO 映射到 EXTI 线。
//-------------------------------------------------------------------------------------------------------------------
void RUN_exti_init(RUN_EXTI_Pin_enum exti_pin, 
                   EXTITrigger_TypeDef trigger_mode, 
                   uint8_t pre_priority, 
                   uint8_t sub_priority,
                   ExtiCallback_t callback)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    uint8_t pin_source;
    uint8_t port_source;
    IRQn_Type irq_channel;

    if((uint16_t)exti_pin == 0xFFFF) return;

    // 1. 硬件资源转换
    // 将通用的 RUN_EXTI_Pin_enum 强制转换为 RUN_GPIO_enum 以利用现有的查找表
    RUN_GPIO_enum pin_enum = (RUN_GPIO_enum)exti_pin;
    GPIO_TypeDef* port = gpio_cfg[pin_enum].port;
    uint16_t pin = gpio_cfg[pin_enum].pin;
    
    // 2. 开启时钟
    // 【关键】使用 EXTI 必须开启 AFIO (复用功能IO) 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
    // 初始化 GPIO 为输入模式 (浮空输入或上拉输入，取决于 RUN_gpio_init 的内部实现)
    RUN_gpio_init(pin_enum, GPI, 0);

    // 3. 计算 AFIO 配置所需的索引
    pin_source = RUN_get_pin_source(pin);
    port_source = RUN_get_port_source(port);

    // 4. 注册回调函数
    if(pin_source < 16) exti_callbacks[pin_source] = callback;

    // 5. AFIO 线路映射
    // 作用：告诉 STM32，当前的 EXTI_LineX 是连接到 PAx, PBx 还是 PCx...
    // 注意：同一时刻，PA0, PB0, PC0 只能选一个连接到 EXTI_Line0
    GPIO_EXTILineConfig(port_source, pin_source);

    // 6. EXTI 外设配置
    EXTI_InitStructure.EXTI_Line = pin;             // 指定中断线 (如 GPIO_Pin_0)
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = trigger_mode; // 上升沿/下降沿
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // 7. 匹配 NVIC 中断通道
    // STM32 的中断通道是分组的：
    // Line 0~4   : 每个线有独立的通道 (EXTI0_IRQn ... EXTI4_IRQn)
    // Line 5~9   : 共用一个通道 (EXTI9_5_IRQn)
    // Line 10~15 : 共用一个通道 (EXTI15_10_IRQn)
    if (pin_source == 0)       irq_channel = EXTI0_IRQn;
    else if (pin_source == 1) irq_channel = EXTI1_IRQn;
    else if (pin_source == 2) irq_channel = EXTI2_IRQn;
    else if (pin_source == 3) irq_channel = EXTI3_IRQn;
    else if (pin_source == 4) irq_channel = EXTI4_IRQn;
    else if (pin_source >= 5 && pin_source <= 9)   irq_channel = EXTI9_5_IRQn;
    else if (pin_source >= 10 && pin_source <= 15) irq_channel = EXTI15_10_IRQn;
    else return;

    // 8. NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = irq_channel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pre_priority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = sub_priority;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// ==============================================================================
// 中断服务函数 (ISR Handlers)
// ==============================================================================

// 
// 上图展示了中断向量表。STM32 硬件检测到中断时，会直接跳转到对应的 IRQHandler 函数执行。

/**
 * @brief  通用中断处理分发器
 * @param  line   EXTI线路宏 (如 EXTI_Line0)
 * @param  index  回调数组索引 (0~15)
 */
static void RUN_EXTI_Handler(uint32_t line, uint8_t index)
{
    // 1. 检查标志位 (确认该线是否真的产生了中断)
    if (EXTI_GetITStatus(line) != RESET)
    {
        // 2. 执行用户注册的回调函数 (如果存在)
        if (exti_callbacks[index]) exti_callbacks[index]();
        
        // 3. 清除挂起标志位 (必须清除，否则会无限进入中断)
        EXTI_ClearITPendingBit(line);
    }
}

// --- 独立中断处理 (Line 0 ~ 4) ---
void EXTI0_IRQHandler(void) { RUN_EXTI_Handler(EXTI_Line0, 0); }
void EXTI1_IRQHandler(void) { RUN_EXTI_Handler(EXTI_Line1, 1); }
void EXTI2_IRQHandler(void) { RUN_EXTI_Handler(EXTI_Line2, 2); }
void EXTI3_IRQHandler(void) { RUN_EXTI_Handler(EXTI_Line3, 3); }
void EXTI4_IRQHandler(void) { RUN_EXTI_Handler(EXTI_Line4, 4); }

// --- 共享中断处理 (Line 5 ~ 9) ---
// 任何 Line 5 到 Line 9 的中断都会进入此函数
// RUN_EXTI_Handler 内部的 GetITStatus 会判断具体是哪根线触发的
void EXTI9_5_IRQHandler(void)
{
    RUN_EXTI_Handler(EXTI_Line5, 5); 
    RUN_EXTI_Handler(EXTI_Line6, 6);
    RUN_EXTI_Handler(EXTI_Line7, 7); 
    RUN_EXTI_Handler(EXTI_Line8, 8);
    RUN_EXTI_Handler(EXTI_Line9, 9);
}

// --- 共享中断处理 (Line 10 ~ 15) ---
void EXTI15_10_IRQHandler(void)
{
    RUN_EXTI_Handler(EXTI_Line10, 10); 
    RUN_EXTI_Handler(EXTI_Line11, 11);
    RUN_EXTI_Handler(EXTI_Line12, 12); 
    RUN_EXTI_Handler(EXTI_Line13, 13);
    RUN_EXTI_Handler(EXTI_Line14, 14); 
    RUN_EXTI_Handler(EXTI_Line15, 15);
}