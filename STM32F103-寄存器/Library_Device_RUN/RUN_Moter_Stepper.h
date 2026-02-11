#ifndef _RUN_MOTER_STEPPER_H_
#define _RUN_MOTER_STEPPER_H_

#include "stm32f10x.h"
#include "RUN_PWM.h"  // 必须包含 (提供 PWM 脉冲)
#include "RUN_Gpio.h" // 必须包含 (提供 DIR 方向)

// ==============================================================================
// 步进电机结构体
// ==============================================================================
typedef struct {
    RUN_PWM_enum  pwm_pin;   // 脉冲引脚 (PUL/STEP) -> 接 PWM
    RUN_GPIO_enum dir_pin;   // 方向引脚 (DIR)      -> 接 GPIO
} RUN_Stepper_t;

// ==============================================================================
// 函数声明
// ==============================================================================

/**
 * @brief  步进电机初始化
 * @param  motor: 电机结构体指针
 */
void RUN_Step_Init(RUN_Stepper_t* motor);

/**
 * @brief  设置步进电机速度和方向
 * @param  motor: 电机结构体指针
 * @param  freq_hz: 脉冲频率 (Hz)，对应转速
 * > 0 : 正转 (DIR = 1)
 * < 0 : 反转 (DIR = 0)
 * = 0 : 停止 (PWM = 0)
 * @note   注意：步进电机启动时频率不能太高(建议从500Hz开始)，否则会堵转。
 */
void RUN_Step_SetSpeed(RUN_Stepper_t* motor, int32_t freq_hz);

#endif