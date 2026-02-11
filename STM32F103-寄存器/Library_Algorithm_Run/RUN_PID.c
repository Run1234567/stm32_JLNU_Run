#include "RUN_PID.h"
#include <math.h> 

/**
  * @brief  内部通用函数：数值限幅
  * @note   如果 value 超过范围，强制将其限制在 [min, max] 之间
  * @param  value: 待限幅的输入值
  * @param  min:   允许的最小值
  * @param  max:   允许的最大值
  * @retval float: 限幅后的结果
  */
static float Limit_Range(float value, float min, float max) {
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

/* * =================================================================================
 * [ PART I ] 位置式 PID 实现 (Positional PID)
 * 适用场景: 直立环、位置控制、温控 (需要直接对应输出量的场景)
 * =================================================================================
 */

/**
  * @brief  位置式 PID 初始化函数
  * @param  pid:     PID 结构体句柄 (指针)
  * @param  kp:      比例系数 (Proportional)
  * @param  ki:      积分系数 (Integral)
  * @param  kd:      微分系数 (Derivative)
  * @param  max_out: 输出限幅 (绝对值)，例如 PWM 最大值 1000
  * @param  max_i:   积分限幅 (绝对值)，防止积分过大导致难以恢复 (Anti-Windup)
  * @retval None
  */
void RUN_PID_Pos_Init(RUN_PID_Pos_t *pid, float kp, float ki, float kd, float max_out, float max_i)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->MaxOut = max_out;
    pid->MaxI   = max_i;
    RUN_PID_Pos_Reset(pid);
}

/**
  * @brief  位置式 PID 状态复位
  * @note   通常在系统启动、电机停止或PID重新介入控制前调用。
  * 作用是清除历史误差和积分累计值，防止系统带着旧的错误状态启动。
  * @param  pid: PID 结构体句柄
  * @retval None
  */
void RUN_PID_Pos_Reset(RUN_PID_Pos_t *pid)
{
    pid->Error     = 0.0f;
    pid->LastError = 0.0f;
    pid->Integral  = 0.0f;
    pid->Output    = 0.0f;
}

/**
  * @brief  位置式 PID 计算核心函数
  * @note   公式: Output = Kp*E + Ki*Sum(E) + Kd*(E - LastE)
  * @param  pid:     PID 结构体句柄
  * @param  target:  目标值 (例如期望角度 0.0)
  * @param  measure: 实际测量值 (例如传感器读回的角度)
  * @retval float:   计算后的控制输出量 (已限幅)
  */
float RUN_PID_Pos_Calc(RUN_PID_Pos_t *pid, float target, float measure)
{
    // 1. 计算误差
    pid->Error = target - measure;

    // 2. 积分项计算 (带抗饱和处理)
    // 积分分离逻辑可在此添加 (例如误差很大时不积分)
    pid->Integral += pid->Ki * pid->Error;
    
    // 积分限幅: 防止电机卡死或大误差时积分项无限累积
    pid->Integral = Limit_Range(pid->Integral, -pid->MaxI, pid->MaxI);

    // 3. 核心公式: 总输出 = 比例项 + 积分项 + 微分项
    pid->Output = pid->Kp * pid->Error 
                + pid->Integral 
                + pid->Kd * (pid->Error - pid->LastError);

    // 4. 总输出限幅 (例如限制 PWM 在 -1000 到 1000 之间)
    pid->Output = Limit_Range(pid->Output, -pid->MaxOut, pid->MaxOut);

    // 5. 更新历史误差 (供下一次微分计算使用)
    pid->LastError = pid->Error;

    return pid->Output;
}


/* * =================================================================================
 * [ PART II ] 增量式 PID 实现 (Incremental PID)
 * 适用场景: 速度环、步进电机、执行机构带积分特性的系统
 * =================================================================================
 */

/**
  * @brief  增量式 PID 初始化函数
  * @param  pid:     PID 结构体句柄
  * @param  kp, ki, kd: PID 系数
  * @param  max_out: 总输出限幅 (绝对值)
  * @retval None
  */
void RUN_PID_Inc_Init(RUN_PID_Inc_t *pid, float kp, float ki, float kd, float max_out)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->MaxOut = max_out;
    RUN_PID_Inc_Reset(pid);
}

/**
  * @brief  增量式 PID 状态复位
  * @note   清除最近三次误差记录和当前的累计输出值
  * @param  pid: PID 结构体句柄
  * @retval None
  */
void RUN_PID_Inc_Reset(RUN_PID_Inc_t *pid)
{
    pid->Error     = 0.0f;
    pid->LastError = 0.0f;
    pid->PrevError = 0.0f;
    pid->Output    = 0.0f;
}

/**
  * @brief  增量式 PID 计算核心函数
  * @note   公式: ΔOut = Kp*(E-E1) + Ki*E + Kd*(E - 2E1 + E2)
  * 本函数会自动将增量 ΔOut 累加到 Output 中返回。
  * @param  pid:     PID 结构体句柄
  * @param  target:  目标值 (例如期望速度)
  * @param  measure: 实际测量值 (例如编码器测得的速度)
  * @retval float:   最终控制输出量 (已累加并限幅)
  */
float RUN_PID_Inc_Calc(RUN_PID_Inc_t *pid, float target, float measure)
{
    float inc_val; // 本次计算出的增量值 (Delta Output)

    // 1. 计算当前误差
    pid->Error = target - measure;

    // 2. 增量式核心公式
    // Kp * (本次误差 - 上次误差)
    // Ki * 本次误差
    // Kd * (本次误差 - 2*上次误差 + 上上次误差)
    inc_val = pid->Kp * (pid->Error - pid->LastError)
            + pid->Ki * pid->Error
            + pid->Kd * (pid->Error - 2.0f * pid->LastError + pid->PrevError);

    // 3. 累加输出 (增量式PID计算的是变化量，所以要累加到当前值上)
    pid->Output += inc_val;

    // 4. 输出限幅
    pid->Output = Limit_Range(pid->Output, -pid->MaxOut, pid->MaxOut);

    // 5. 更新历史误差 (FIFO 队列移动)
    pid->PrevError = pid->LastError; // e(k-2) = e(k-1)
    pid->LastError = pid->Error;     // e(k-1) = e(k)

    return pid->Output;
}