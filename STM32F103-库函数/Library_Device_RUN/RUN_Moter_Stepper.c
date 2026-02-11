#include "RUN_Moter_Stepper.h"
#include <stdlib.h> // 需要用到 abs() 取绝对值

// 
// 上图展示了步进电机驱动器 (如 TB6600, A4988) 的核心控制时序：
// 1. PUL (Pulse): 脉冲信号。每接收到一个上升沿，电机转动一步 (或微步)。
// 2. DIR (Direction): 方向信号。高/低电平决定顺时针或逆时针。
// 3. ENA (Enable): 使能信号。通常悬空或低电平为使能 (锁轴)，高电平脱机 (手能转动)。
// 本驱动主要控制 PUL 和 DIR。

// ==============================================================================
// 步进电机初始化函数
// ==============================================================================

/**
 * @brief  步进电机硬件初始化
 * @param  motor: 电机句柄
 */
void RUN_Step_Init(RUN_Stepper_t* motor)
{
    // 1. 初始化 PWM (PUL 引脚)
    // 默认给 1000Hz，但占空比为 0 (停止状态)
    // 此时 PWM 引脚维持低电平，没有上升沿，所以电机不转。
    // 注意：只要驱动器供电且 ENA 有效，电机此时处于 "锁死 (Holding Torque)" 状态，手很难拧动。
    RUN_pwm_init(motor->pwm_pin, 1000, 0);

    // 2. 初始化 GPIO (DIR 引脚)
    // 使用 GPO 推挽输出模式，驱动能力强
    RUN_gpio_init(motor->dir_pin, GPO, 0);
}
// ==============================================================================
// 速度控制函数 (开环控制)
// ==============================================================================

/**
 * @brief  设定电机速度 (通过改变 PWM 频率)
 * @param  motor:   电机句柄
 * @param  freq_hz: 目标频率 (Hz)
 * - 正数: 正转
 * - 负数: 反转
 * - 0:    停止
 * @note   转速公式: RPM = (Frequency * 60) / (StepsPerRev * Microstep)
 */
void RUN_Step_SetSpeed(RUN_Stepper_t* motor, int32_t freq_hz)
{
    // --- 1. 停止处理 ---
    if (freq_hz == 0) {
        // 占空比设为 0，停止发送脉冲
        // 电机保持锁死状态 (Holding)
        RUN_pwm_set(motor->pwm_pin, 0);
        return;
    }

    // --- 2. 方向处理 (设置 DIR 引脚) ---
    // 建立在脉冲发送之前，确保方向信号由于脉冲信号稳定建立 (Setup Time)
    if (freq_hz > 0) {
        // 正转：方向脚拉高
        RUN_gpio_set(motor->dir_pin, 1);
    } else {
        // 反转：方向脚拉低
        RUN_gpio_set(motor->dir_pin, 0);
    }

    // --- 3. 计算频率绝对值 ---
    // PWM 定时器只接受正数的频率值
    uint32_t abs_freq = (freq_hz > 0) ? freq_hz : -freq_hz;

    // --- 4. 修改 PWM 频率 ---
    // 调用底层定时器接口，修改自动重装载寄存器 (ARR) 和 预分频器 (PSC)
    RUN_pwm_freq(motor->pwm_pin, abs_freq);

    // --- 5. 开启脉冲 (设置 50% 占空比) ---
    // 步进驱动器通常是 "边缘触发" (Edge Triggered)，通常是上升沿有效。
    // 只要有上升沿就行，脉冲宽度只要满足最小要求即可。
    // 设置 50% 占空比 (方波) 是最通用、能量分布最均匀的方式。
    // 假设 RUN_pwm_set 的满量程是 10000，那么 5000 就是 50%。
    RUN_pwm_set(motor->pwm_pin, 5000);
}