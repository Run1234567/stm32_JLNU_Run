#ifndef __RUN_PID_H
#define __RUN_PID_H

#include <stdint.h>

/* =================================================================================
 * [ TYPE 1 ] 位置式 PID (Positional PID)
 * ---------------------------------------------------------------------------------
 * >> 适用: 直立环、转向环、温控、舵机位置控制
 * >> 特点: 直接计算输出值，必须要有积分限幅(Anti-Windup)防止饱和
 * =================================================================================
 */
typedef struct {
    // 参数
    float Kp;           // 比例
    float Ki;           // 积分
    float Kd;           // 微分

    // 限幅
    float MaxOut;       // 输出限幅 (例如 1000)
    float MaxI;         // 积分限幅 (建议设为 MaxOut 的 30%~50%)

    // 内部状态
    float Error;        // 当前误差
    float LastError;    // 上次误差
    float Integral;     // 积分累计
    float Output;       // 最终输出

} RUN_PID_Pos_t;

/* =================================================================================
 * [ TYPE 2 ] 增量式 PID (Incremental PID)
 * ---------------------------------------------------------------------------------
 * >> 适用: 速度环、步进电机、流量阀
 * >> 特点: 计算输出的变化量，无积分累积饱和问题，抗干扰性强
 * =================================================================================
 */
typedef struct {
    // 参数
    float Kp;
    float Ki;
    float Kd;

    // 限幅
    float MaxOut;       // 输出限幅

    // 内部状态
    float Error;        // e(k)
    float LastError;    // e(k-1)
    float PrevError;    // e(k-2)
    float Output;       // 实际输出累计值

} RUN_PID_Inc_t;


/* ================= API 函数声明 ================= */

/* --- 位置式 PID 接口 --- */
void RUN_PID_Pos_Init(RUN_PID_Pos_t *pid, float kp, float ki, float kd, float max_out, float max_i);
void RUN_PID_Pos_Reset(RUN_PID_Pos_t *pid);
float RUN_PID_Pos_Calc(RUN_PID_Pos_t *pid, float target, float measure);

/* --- 增量式 PID 接口 --- */
void RUN_PID_Inc_Init(RUN_PID_Inc_t *pid, float kp, float ki, float kd, float max_out);
void RUN_PID_Inc_Reset(RUN_PID_Inc_t *pid);
float RUN_PID_Inc_Calc(RUN_PID_Inc_t *pid, float target, float measure);

#endif