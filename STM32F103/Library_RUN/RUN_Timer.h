#ifndef _RUN_TIMER_H_
#define _RUN_TIMER_H_

#include "stm32f10x.h"

// ==========================================================
// 定时器枚举 (ZET6 全套)
// ==========================================================
typedef enum {
    // 高级定时器 (APB2) - 通常用于 PWM，但也能做定时
    RUN_TIM1,
    RUN_TIM8,

    // 通用定时器 (APB1) - 资源丰富
    RUN_TIM2,
    RUN_TIM3,
    RUN_TIM4,
    RUN_TIM5,

    // 基本定时器 (APB1) - ZET6 独有，强烈推荐用于纯定时！
    // 它们没有 GPIO 引脚，不占用任何 IO 资源
    RUN_TIM6,
    RUN_TIM7,

    RUN_TIM_MAX
} RUN_TIM_enum;

// ==========================================================
// 函数声明
// ==========================================================

/**
 * @brief  定时器中断初始化
 * @param  tim_n:   定时器枚举 (如 RUN_TIM6)
 * @param  time_ms: 中断周期 (单位: 毫秒)
 * 注意：16位定时器最大计数约为 6500ms (6.5秒)
 */
void RUN_timer_init(RUN_TIM_enum tim_n, uint16_t time_ms);

/**
 * @brief  定时器开关
 */
void RUN_timer_cmd(RUN_TIM_enum tim_n, FunctionalState state);

#endif