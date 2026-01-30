/**
  ******************************************************************************
  * @file    RUN_IMU_GetAngle.c
  * @author  RUN Library
  * @brief   IMU 姿态解算模块 (基于 Mahony 互补滤波算法)
  * @note    
  * 1. 本模块用于将 6轴传感器 (加速度+陀螺仪) 的原始物理值融合为欧拉角。
  * 2. 算法核心为 Mahony 互补滤波，利用加速度计修正陀螺仪的积分漂移。
  * 3. 必须保证函数调用的频率与 RUN_IMU_SAMPLE_TIME_MS 定义的时间一致。
  ******************************************************************************
  */

#include "RUN_IMU_GetAngle.h"
#include <math.h>

/* ================= 内部参数定义 ================= */

/**
 * @brief 比例增益 (Kp)
 * @details 控制加速度计修正陀螺仪的速度。
 * - Kp 越大，姿态跟随加速度计越快，但抗震动干扰能力变差。
 * - Kp 越小，姿态越平滑，但在剧烈运动后恢复水平的时间变长。
 */
#define Kp 10.0f             

/**
 * @brief 积分增益 (Ki)
 * @details 用于消除陀螺仪的稳态零点漂移。
 * - 必须设置得很小，否则会导致姿态缓慢震荡。
 */
#define Ki 0.005f            

/* 常量定义 */
#define DEG_TO_RAD 0.0174532925f  // 度 -> 弧度 (PI / 180)
#define RAD_TO_DEG 57.295779513f  // 弧度 -> 度 (180 / PI)

/* ================= 内部静态变量 (保持算法状态) ================= */

// 四元数 (Quaternion)，描述当前的 3D 姿态
// 初始状态 w=1, x=0, y=0, z=0 (代表水平静止状态)
static float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;

// 积分误差累计 (Integral Error)，用于保存陀螺仪的零偏
static float exInt = 0.0f, eyInt = 0.0f, ezInt = 0.0f;

/**
 * @brief  Mahony 姿态解算更新函数 (6轴融合)
 * @note   该函数实现了互补滤波算法，将加速度计和陀螺仪数据融合。
 * **必须周期性调用**，且调用周期需与 RUN_IMU_SAMPLE_TIME_MS 一致。
 * * @param  ax    X轴加速度实际值 (单位: g 或 m/s^2，内部会自动归一化，数值大小不影响方向)
 * @param  ay    Y轴加速度实际值
 * @param  az    Z轴加速度实际值
 * @param  gx    X轴角速度实际值 (单位: 度/秒 deg/s)
 * @param  gy    Y轴角速度实际值 (单位: 度/秒 deg/s)
 * @param  gz    Z轴角速度实际值 (单位: 度/秒 deg/s)
 * @param  roll  [输出参数] 指向存储 横滚角(Roll) 的变量指针 (单位: 度)
 * @param  pitch [输出参数] 指向存储 俯仰角(Pitch) 的变量指针 (单位: 度)
 * @param  yaw   [输出参数] 指向存储 航向角(Yaw) 的变量指针 (单位: 度)
 * @note 纯6轴算法无法利用地磁修正 Yaw，因此 Yaw 会随时间缓慢漂移，
 * 只反映相对角度变化。
 * @retval None
 */
void RUN_IMU_Update(float ax, float ay, float az, 
                    float gx, float gy, float gz, 
                    float *roll, float *pitch, float *yaw)
{
    float norm;
    float vx, vy, vz;
    float ex, ey, ez;
    
    // 计算采样时间 (秒)
    // 注意：RUN_IMU_SAMPLE_TIME_MS 必须在头文件中定义，例如 5.0f
    float dt = RUN_IMU_SAMPLE_TIME_MS / 1000.0f;

    // 1. 单位转换：将角速度从 [度/秒] 转换为 [弧度/秒]
    // Mahony 算法内部计算依赖弧度制
    gx *= DEG_TO_RAD;
    gy *= DEG_TO_RAD;
    gz *= DEG_TO_RAD;

    // 2. 加速度计数据归一化
    // 我们只关心重力的方向，不关心大小
    norm = sqrtf(ax*ax + ay*ay + az*az);
    
    // 只有在加速度数据有效(非零)时才进行重力修正
    // 处于自由落体时 norm 可能接近 0，此时跳过修正，完全信任陀螺仪
    if(norm > 0.0f)
    {
        norm = 1.0f / norm;
        ax *= norm;
        ay *= norm;
        az *= norm;

        // 3. 提取重力方向 (理论推算值)
        // 将当前姿态(四元数)转换成旋转矩阵的第三行，即推算出重力应该在的分量
        vx = 2.0f * (q1*q3 - q0*q2);
        vy = 2.0f * (q0*q1 + q2*q3);
        vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;

        // 4. 计算误差 (向量叉积)
        // 测量得到的重力方向(ax,ay,az) 与 推算的重力方向(vx,vy,vz) 的偏差
        // 叉积的结果向量方向代表了旋转轴，模长代表了误差大小
        ex = (ay*vz - az*vy);
        ey = (az*vx - ax*vz);
        ez = (ax*vy - ay*vx);

        // 5. 误差积分 (PI控制器的 I 项)
        // 累积误差用于自动估算并消除陀螺仪的静态零偏
        exInt += Ki * ex * dt;
        eyInt += Ki * ey * dt;
        ezInt += Ki * ez * dt;

        // 6. 应用反馈 (PI控制器的 P 项)
        // 将误差补偿到陀螺仪数据上
        gx += Kp * ex + exInt;
        gy += Kp * ey + eyInt;
        gz += Kp * ez + ezInt;
    }

    // 7. 四元数微分方程求解 (一阶龙格库塔法 / 欧拉积分)
    // 利用修正后的角速度更新四元数
    float q0_last = q0;
    float q1_last = q1;
    float q2_last = q2;
    float q3_last = q3;

    q0 += (-q1_last*gx - q2_last*gy - q3_last*gz) * 0.5f * dt;
    q1 += ( q0_last*gx - q3_last*gy + q2_last*gz) * 0.5f * dt;
    q2 += ( q3_last*gx + q0_last*gy - q1_last*gz) * 0.5f * dt;
    q3 += (-q2_last*gx + q1_last*gy + q0_last*gz) * 0.5f * dt;

    // 8. 四元数归一化
    // 计算机浮点运算会有误差，长时间积分会导致四元数模长不为1，需强制归一化
    norm = sqrtf(q0*q0 + q1*q1 + q2*q2 + q3*q3);
    if(norm > 0.0f)
    {
        norm = 1.0f / norm;
        q0 *= norm;
        q1 *= norm;
        q2 *= norm;
        q3 *= norm;
    }

    // 9. 将四元数转换为欧拉角 (输出单位：度)
    // 使用标准航空转换公式 Z-Y-X 顺序
    
    // Pitch (俯仰角): 绕 Y 轴旋转
    // asin 函数定义域为 [-1, 1]，增加限幅防止数值越界导致 NaN
    float pitch_val = 2.0f * (q0*q2 - q1*q3);
    if (pitch_val > 1.0f) pitch_val = 1.0f;
    if (pitch_val < -1.0f) pitch_val = -1.0f;
    *pitch = asinf(pitch_val) * RAD_TO_DEG;

    // Roll (横滚角): 绕 X 轴旋转
    *roll  = atan2f(2.0f * (q0*q1 + q2*q3), 1.0f - 2.0f * (q1*q1 + q2*q2)) * RAD_TO_DEG;

    // Yaw (航向角): 绕 Z 轴旋转
    *yaw   = atan2f(2.0f * (q0*q3 + q1*q2), 1.0f - 2.0f * (q2*q2 + q3*q3)) * RAD_TO_DEG;
}

/**
 * @brief  重置算法状态
 * @note   通常在模块初始化时，或检测到姿态发散/异常时调用。
 * 重置后，四元数恢复到水平初始值，积分误差清零。
 * @retval None
 */
void RUN_IMU_Reset(void)
{
    // 重置四元数 (w=1 表示无旋转)
    q0 = 1.0f; 
    q1 = 0.0f; 
    q2 = 0.0f; 
    q3 = 0.0f;
    
    // 清除积分误差
    exInt = 0.0f; 
    eyInt = 0.0f; 
    ezInt = 0.0f;
}