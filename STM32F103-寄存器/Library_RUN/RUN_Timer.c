#include "RUN_Timer.h" // 假设你的头文件名为这个
// 必须包含 stm32f10x.h 才能使用寄存器定义 (如 TIM2->CR1)
#include "stm32f10x.h" 

// --- 内部硬件配置结构体 (保持不变) ---
typedef struct {
    TIM_TypeDef* tim_base;   // 定时器硬件基地址
    uint32_t     rcc;        // 时钟位掩码 (如 RCC_APB1ENR_TIM2EN)
    uint8_t      is_apb2;    // 总线标志位 1:APB2, 0:APB1
    IRQn_Type    irqn;       // 中断通道号
} timer_info_t;

// ============================================================================
// 硬件映射表 (保持不变)
// ============================================================================
const timer_info_t timer_cfg[RUN_TIM_MAX] = {
    // --- 1. 高级定时器 (APB2) ---
    {TIM1, RCC_APB2Periph_TIM1, 1, TIM1_UP_IRQn}, 
    {TIM8, RCC_APB2Periph_TIM8, 1, TIM8_UP_IRQn}, 

    // --- 2. 通用定时器 (APB1) ---
    {TIM2, RCC_APB1Periph_TIM2, 0, TIM2_IRQn},
    {TIM3, RCC_APB1Periph_TIM3, 0, TIM3_IRQn},
    {TIM4, RCC_APB1Periph_TIM4, 0, TIM4_IRQn},
    {TIM5, RCC_APB1Periph_TIM5, 0, TIM5_IRQn}, 

    // --- 3. 基本定时器 (APB1) ---
    {TIM6, RCC_APB1Periph_TIM6, 0, TIM6_IRQn},
    {TIM7, RCC_APB1Periph_TIM7, 0, TIM7_IRQn}
};

/*
 * 函数简介: 初始化通用/高级定时器 (寄存器直操版)
 * 函数参数: tim_n   - 定时器枚举号 (RUN_TIM1 ~ RUN_TIM8)
 * time_ms - 定时周期 (单位: ms，最大值 6553)
 * 返回值  : 无
 * 示例    : RUN_timer_init(RUN_TIM2, 500); // 初始化TIM2，500ms中断一次
 * 备注    : 1. 基于72MHz主频，固定分频7200 (计数频率10KHz)
 * 2. 内部自动配置NVIC (优先级分组2: 抢占2, 子1)
 */
void RUN_timer_init(RUN_TIM_enum tim_n, uint16_t time_ms)
{
    if (tim_n >= RUN_TIM_MAX) return;

    // 获取配置指针，减少重复代码
    const timer_info_t *cfg = &timer_cfg[tim_n];
    TIM_TypeDef *TIMx = cfg->tim_base;

    // =========================================================
    // 1. 开启外设时钟 (直接写 RCC 寄存器)
    // =========================================================
    if (cfg->is_apb2)
    {
        // APB2ENR: APB2 外设时钟使能寄存器
        RCC->APB2ENR |= cfg->rcc; 
    }
    else
    {
        // APB1ENR: APB1 外设时钟使能寄存器
        RCC->APB1ENR |= cfg->rcc;
    }

    // =========================================================
    // 2. 时基单元配置 (直接写 TIMx 寄存器)
    // =========================================================
    
    // PSC: 预分频器 (16位)
    // 写入值 = 分频系数 - 1
    TIMx->PSC = 7199; 

    // ARR: 自动重装载寄存器 (16位)
    // 决定定时周期
    TIMx->ARR = (time_ms * 10) - 1;

    // CR1: 控制寄存器 1
    // 这是一个重要的寄存器，我们通常需要先清零再设置，或者只设置需要的位
    // Bit 4 (DIR): 0 = 向上计数 (默认)
    // Bit 8,9 (CKD): 00 = 1分频 (默认)
    // 这里我们只复位 CR1 的关键设置，保持默认向上计数
    TIMx->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS | TIM_CR1_CKD); 

    // =========================================================
    // 3. 触发更新事件 (影子寄存器生效)
    // =========================================================
    // EGR: 事件产生寄存器
    // Bit 0 (UG): Update Generation. 置 1 产生更新事件，重新加载 PSC 和 ARR
    TIMx->EGR |= TIM_EGR_UG;

    // =========================================================
    // 4. 清除标志位
    // =========================================================
    // SR: 状态寄存器
    // 上一步写 EGR 会硬件置位 SR 的 Bit 0 (UIF)，必须手动清除
    TIMx->SR &= ~TIM_SR_UIF;

    // =========================================================
    // 5. 开启定时器更新中断
    // =========================================================
    // DIER: DMA/中断使能寄存器
    // Bit 0 (UIE): Update Interrupt Enable
    TIMx->DIER |= TIM_DIER_UIE;

    // =========================================================
    // 6. NVIC 中断控制器配置 (Cortex-M3 核心寄存器)
    // =========================================================
    // 注意：寄存器操作通常使用 CMSIS 提供的内联函数来操作 NVIC，
    // 因为 NVIC 属于内核外设，直接算地址偏移比较麻烦且易错。
    
    // 计算优先级编码 (假设主函数中已经设为 Group 2)
    // Preemption=2, Sub=1
    // 这是一个辅助计算，如果不确定 Group，可以用简易方式
    uint32_t prioritygroup = NVIC_GetPriorityGrouping();
    uint32_t priority_encode = NVIC_EncodePriority(prioritygroup, 2, 1);
    
    NVIC_SetPriority(cfg->irqn, priority_encode); // 设置优先级
    NVIC_EnableIRQ(cfg->irqn);                    // 使能中断通道

    // =========================================================
    // 7. 使能定时器
    // =========================================================
    // CR1: 控制寄存器
    // Bit 0 (CEN): Counter Enable
    TIMx->CR1 |= TIM_CR1_CEN;
}

/*
 * 函数简介: 定时器计数器开关控制
 * 函数参数: tim_n - 定时器枚举号
 * state - 运行状态 (ENABLE: 启动计数, DISABLE: 暂停计数)
 * 返回值  : 无
 * 示例    : RUN_timer_cmd(RUN_TIM2, DISABLE); // 暂停TIM2计数
 * 备注    : 仅操作 CR1寄存器的 CEN位，暂停时不清除当前计数值
 */
void RUN_timer_cmd(RUN_TIM_enum tim_n, FunctionalState state)
{
    if (tim_n >= RUN_TIM_MAX) return;

    if (state != DISABLE)
    {
        // 置位 CEN (Bit 0)
        timer_cfg[tim_n].tim_base->CR1 |= TIM_CR1_CEN;
    }
    else
    {
        // 清零 CEN (Bit 0)
        timer_cfg[tim_n].tim_base->CR1 &= ~TIM_CR1_CEN;
    }
}