#include "RUN_Moter_Brushed.h"

// 
// 上图展示了 H桥 (H-Bridge) 驱动电路的基本原理：
// 1. 它由 4 个开关管 (MOSFET) 组成，形状像字母 "H"。
// 2. 通过控制对角线上的开关导通，可以改变流过电机的电流方向，从而控制正反转。
// 3. DIR 引脚控制电流方向，PWM 引脚控制电流导通的时间比例 (即速度)。

// ==============================================================================
// 内部函数：控制单个电机
// ==============================================================================

/**
 * @brief  单电机底层控制函数
 * @param  m:     电机配置结构体指针 (包含 PWM ID 和 方向引脚)
 * @param  speed: 目标速度，范围 -10000 ~ +10000
 * 正数代表正转，负数代表反转，0 代表停止
 */
static void _Motor_Run_Single(RUN_Motor_Config_t* m, int16_t speed) {
    // 1. 速度限幅 (Input Saturation)
    // 防止传入过大的数值导致 PWM 寄存器溢出或不可控
    if (speed > 10000) speed = 10000;
    if (speed < -10000) speed = -10000;

    uint32_t duty = 0;
    
    // 2. 逻辑控制 (Direction & Duty Cycle)
    // 根据 H桥 驱动芯片 (如 TB6612, L298N, A4950) 的逻辑：
    // 通常需要一个方向信号 (DIR) 和一个使能/速度信号 (PWM)
    
    if (speed > 0) {
        // --- 正转 (Forward) ---
        RUN_gpio_set(m->dir_pin, 1); // 逻辑高电平设置方向
        duty = speed;                // 占空比直接对应速度值
    } 
    else if (speed < 0) {
        // --- 反转 (Reverse) ---
        RUN_gpio_set(m->dir_pin, 0); // 逻辑低电平切换方向
        duty = -speed;               // PWM 寄存器只认正数，取绝对值
    } 
    else {
        // --- 停止 (Stop) ---
        // PWM 为 0 时，MOS管截止，电机失去动力自由滑行 (Coasting)
        // 若需要“刹车 (Brake)”，通常需要将两个输入端同时拉低或拉高 (取决于驱动芯片)
        duty = 0;
        // 停止时方向引脚保持原样或拉低均可，不影响结果
    }

    // 3. 设置 PWM 占空比
    // 假设底层 RUN_pwm_set 接受 0~10000 的数值
    // 公式: $V_{motor} = V_{bat} \times \frac{Duty}{Period}$
    RUN_pwm_set(m->pwm_id, duty);
}

// 
// 上图展示了 PWM (脉宽调制) 的概念：
// 频率 (Frequency): 脉冲重复的速度 (本代码设置为 20kHz)。
// 占空比 (Duty Cycle): 高电平持续时间占整个周期的百分比。
// 50% 占空比意味着电机只能得到电池电压的一半有效电压。

// ==============================================================================
// 用户接口函数
// ==============================================================================

/**
 * @brief  初始化小车底盘电机
 * @param  car: 小车句柄
 */
void RUN_Car_Init(RUN_Car_t* car) {
    for(int i = 0; i < car->motor_count; i++) {
        // 1. 初始化 PWM
        // 频率设定为 20000Hz (20kHz) 的原因：
        // 人耳的听觉范围大约是 20Hz ~ 20kHz。
        // 如果 PWM 频率太低 (如 1kHz)，电机线圈会发出刺耳的电流啸叫声。
        // 设置为 20kHz 可以让声音变成“超声波”，实现静音驱动。
        RUN_pwm_init(car->motor[i].pwm_id, 20000, 0); 
        
        // 2. 初始化 方向引脚
        // 推挽输出 (Push-Pull)，驱动能力强，开关速度快
        RUN_gpio_init(car->motor[i].dir_pin, GPO, 0);
    }
}

/**
 * @brief  控制两轮差速底盘 (2WD)
 * @param  speed1: 左电机速度
 * @param  speed2: 右电机速度
 */
void RUN_Car_Set2(RUN_Car_t* car, int16_t speed1, int16_t speed2) {
    if(car->motor_count < 2) return; // 安全检查
    
    // 分别设置左右电机
    _Motor_Run_Single(&car->motor[0], speed1);
    _Motor_Run_Single(&car->motor[1], speed2);
}

/**
 * @brief  控制四轮底盘 (4WD / 麦克纳姆轮 / 全向轮)
 * @param  s1, s2, s3, s4: 四个电机的速度
 * @note   通常电机顺序为：左前、右前、左后、右后 (具体需参考硬件接线)
 */
void RUN_Car_Set4(RUN_Car_t* car, int16_t s1, int16_t s2, int16_t s3, int16_t s4) {
    if(car->motor_count < 4) return;
    
    _Motor_Run_Single(&car->motor[0], s1);
    _Motor_Run_Single(&car->motor[1], s2);
    _Motor_Run_Single(&car->motor[2], s3);
    _Motor_Run_Single(&car->motor[3], s4);
}

/**
 * @brief  急停
 */
void RUN_Car_Stop(RUN_Car_t* car) {
    for(int i = 0; i < car->motor_count; i++) {
        _Motor_Run_Single(&car->motor[i], 0);
    }
}