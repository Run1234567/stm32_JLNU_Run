#ifndef _RUN_MOTOR_BRUSHED_H_
#define _RUN_MOTOR_BRUSHED_H_

#include "stm32f10x.h"
#include "RUN_PWM.h"  // 引用你的PWM库
#include "RUN_Gpio.h" // 引用你的GPIO库

// ==============================================================================
// 1. 单个电机配置结构体
// ==============================================================================
typedef struct {
    RUN_PWM_enum  pwm_id;   // 速度控制 (如 PWM_TIM1_CH1_PA8)
    RUN_GPIO_enum dir_pin;  // 方向引脚 (直接填枚举，如 C0, A5)
} RUN_Motor_Config_t;

// ==============================================================================
// 2. 整车对象结构体
// ==============================================================================
typedef struct {
    uint8_t motor_count;          // 电机数量 (2 或 4)
    RUN_Motor_Config_t motor[4];  // 电机数组 [0]~[3]
} RUN_Car_t;

// ==============================================================================
// 函数声明
// ==============================================================================

/**
 * @brief  整车初始化 (自动初始化内部的 PWM 和 GPIO)
 * @param  car: 车辆结构体指针
 */
void RUN_Car_Init(RUN_Car_t* car);

/**
 * @brief  双轮控制
 * @param  speed1: 左电机速度 (-10000 ~ +10000)
 * @param  speed2: 右电机速度
 */
void RUN_Car_Set2(RUN_Car_t* car, int16_t speed1, int16_t speed2);

/**
 * @brief  四轮控制
 * @param  s1~s4: 对应 motor[0]~[3] 的速度
 */
void RUN_Car_Set4(RUN_Car_t* car, int16_t s1, int16_t s2, int16_t s3, int16_t s4);

/**
 * @brief  停车 (PWM归零)
 */
void RUN_Car_Stop(RUN_Car_t* car);

#endif