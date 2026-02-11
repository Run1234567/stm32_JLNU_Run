#include "RUN_header_file.h"

// --- 内部硬件配置结构体 ---
typedef struct {
    TIM_TypeDef* tim_base;   // 定时器硬件基地址
    uint32_t     rcc;        // 时钟控制宏 (RCC_APB1... or RCC_APB2...)
    uint8_t      is_apb2;    // 总线标志位 1:APB2(高速, TIM1/8), 0:APB1(低速, 其他)
    IRQn_Type    irqn;       // 中断通道号 (NVIC)
} timer_info_t;

// ============================================================================
// 硬件映射表 (Lookup Table)
// ----------------------------------------------------------------------------
// 【警告】必须严格对应 RUN_Timer.h 中的 RUN_TIM_enum 枚举顺序！
// 作用：将简单的枚举值映射到底层复杂的硬件资源参数
// 顺序：TIM1 -> TIM8 -> TIM2 -> TIM3 -> TIM4 -> TIM5 -> TIM6 -> TIM7
// ============================================================================
const timer_info_t timer_cfg[RUN_TIM_MAX] = {
    // --- 1. 高级定时器 (APB2 72MHz) ---
    {TIM1, RCC_APB2Periph_TIM1, 1, TIM1_UP_IRQn}, 
    {TIM8, RCC_APB2Periph_TIM8, 1, TIM8_UP_IRQn}, // TIM8 是 APB2 的！

    // --- 2. 通用定时器 (APB1 36MHz*2 = 72MHz) ---
    {TIM2, RCC_APB1Periph_TIM2, 0, TIM2_IRQn},
    {TIM3, RCC_APB1Periph_TIM3, 0, TIM3_IRQn},
    {TIM4, RCC_APB1Periph_TIM4, 0, TIM4_IRQn},
    {TIM5, RCC_APB1Periph_TIM5, 0, TIM5_IRQn}, // TIM5 是 APB1 的

    // --- 3. 基本定时器 (APB1 36MHz*2 = 72MHz) ---
    {TIM6, RCC_APB1Periph_TIM6, 0, TIM6_IRQn},
    {TIM7, RCC_APB1Periph_TIM7, 0, TIM7_IRQn}
};

// ==========================================================
// 初始化函数
// ==========================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      通用定时器初始化函数
// 参数说明      tim_n           选择定时器 (枚举值, 如 RUN_TIM6)
// 参数说明      time_ms         定时周期，单位：毫秒 (ms)
// 返回参数      void
// 使用示例      RUN_timer_init(RUN_TIM6, 100); // 初始化定时器6，周期100ms
// 备注信息      基于 72MHz 主频计算：
//               1. 预分频(PSC) = 7199 -> 计数频率 10kHz (0.1ms/跳)
//               2. 自动重装(ARR) = time_ms * 10 -> 达到指定毫秒数
//-------------------------------------------------------------------------------------------------------------------
void RUN_timer_init(RUN_TIM_enum tim_n, uint16_t time_ms)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    if (tim_n >= RUN_TIM_MAX) return;

    // =========================================================
    // 1. 开启外设时钟
    // =========================================================
    // 根据配置表判断是 APB1 还是 APB2 总线，分别调用不同的函数
    if (timer_cfg[tim_n].is_apb2)
        RCC_APB2PeriphClockCmd(timer_cfg[tim_n].rcc, ENABLE);
    else
        RCC_APB1PeriphClockCmd(timer_cfg[tim_n].rcc, ENABLE);

    // =========================================================
    // 2. 时基单元配置 (TimeBase Configuration)
    // =========================================================
    // 目标：计数器频率设为 10kHz (即每 0.1ms 计数一次)
    // 公式：CK_CNT = f_CK_PSC / (PSC + 1)
    // 计算：10000 = 72,000,000 / (7199 + 1)
    TIM_TimeBaseStructure.TIM_Prescaler = 7199;
    
    // 目标：设置溢出周期
    // 因为频率是 10kHz (0.1ms)，所以 1ms 需要计数 10 次
    TIM_TimeBaseStructure.TIM_Period = (time_ms * 10) - 1;
    
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0; // 高级定时器专用，通用定时器忽略
    TIM_TimeBaseInit(timer_cfg[tim_n].tim_base, &TIM_TimeBaseStructure);

    // =========================================================
    // 【关键步骤】手动触发一次更新事件 (Update Event)
    // =========================================================
    // 作用：STM32的预分频器(PSC)有缓冲机制(影子寄存器)。
    // 只有在发生更新事件时，写入的值才会真正起作用。
    // 如果不手动触发，第一次计时的周期可能是错误的（通常是极大或极小）。
    TIM_GenerateEvent(timer_cfg[tim_n].tim_base, TIM_EventSource_Update);

    // =========================================================
    // 3. 清除标志位
    // =========================================================
    // 上一步的 GenerateEvent 会导致硬件置位 "更新中断标志位"。
    // 必须手动清除，否则开启中断的瞬间会立即进入一次中断服务函数。
    TIM_ClearFlag(timer_cfg[tim_n].tim_base, TIM_FLAG_Update);

    // =========================================================
    // 4. 开启中断
    // =========================================================
    TIM_ITConfig(timer_cfg[tim_n].tim_base, TIM_IT_Update, ENABLE);

    // =========================================================
    // 5. NVIC 中断控制器配置
    // =========================================================
    NVIC_InitStructure.NVIC_IRQChannel = timer_cfg[tim_n].irqn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // =========================================================
    // 6. 使能定时器
    // =========================================================
    TIM_Cmd(timer_cfg[tim_n].tim_base, ENABLE);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      定时器开关控制
// 参数说明      tim_n           选择定时器 (枚举值)
// 参数说明      state           ENABLE (开启) / DISABLE (停止)
// 返回参数      void
// 使用示例      RUN_timer_cmd(RUN_TIM6, DISABLE); // 暂停定时器6
// 备注信息      仅控制计数器运行，不关闭时钟
//-------------------------------------------------------------------------------------------------------------------
void RUN_timer_cmd(RUN_TIM_enum tim_n, FunctionalState state)
{
    if (tim_n >= RUN_TIM_MAX) return;
    TIM_Cmd(timer_cfg[tim_n].tim_base, state);
}