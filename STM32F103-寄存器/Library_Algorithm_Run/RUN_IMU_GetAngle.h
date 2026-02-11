#ifndef __RUN_IMU_GETANGLE_H
#define __RUN_IMU_GETANGLE_H

#include <stdint.h>

/* ================= 参数配置 ================= */
// 采样周期 (单位: ms)
// 注意：必须确保 main 循环或定时器调用本函数的频率与此一致！
// 例如: 设置为 5ms，则必须每 5ms 调用一次 RUN_IMU_Update
#define RUN_IMU_SAMPLE_TIME_MS  5.0f

/* ================= 函数声明 ================= */

/**
 * @brief  Mahony 姿态解算更新函数
 * @param  ax, ay, az : 三轴加速度实际值 (单位: g 或 m/s^2 均可，无需归一化)
 * @param  gx, gy, gz : 三轴角速度实际值 (单位: 度/秒, deg/s)
 * @param  roll       : [输出] 横滚角 (单位: 度)
 * @param  pitch      : [输出] 俯仰角 (单位: 度)
 * @param  yaw        : [输出] 航向角 (单位: 度)
 */
void RUN_IMU_Update(float ax, float ay, float az, 
                    float gx, float gy, float gz, 
                    float *roll, float *pitch, float *yaw);

/**
 * @brief  重置姿态解算算法状态
 * @note   如果发现角度发散或需要重新校准起始位置时调用
 */
void RUN_IMU_Reset(void);

										
void RUN_CF_Update(float ax, float ay, float az, float gx, float gy, float gz, float *roll, float *pitch, float *yaw);
										
#endif