#ifndef _RUN_MPU6050_H_
#define _RUN_MPU6050_H_

#include "stm32f10x.h"
#include "RUN_Gpio.h"
#include "RUN_SoftI2C.h" // 必须包含I2C驱动

// ==========================================================
// 1. MPU6050 寄存器地址定义 (原 Reg.h 内容)
// ==========================================================

// 设备地址 (AD0接地时为0xD0，接VCC时为0xD2)
#define MPU_ADDR_DEFAULT    0xD0 

// 核心功能寄存器
#define MPU_SMPLRT_DIV      0x19  // 采样率分频
#define MPU_CONFIG          0x1A  // 低通滤波配置
#define MPU_GYRO_CONFIG     0x1B  // 陀螺仪量程
#define MPU_ACCEL_CONFIG    0x1C  // 加速度量程
#define MPU_FIFO_EN         0x23  // FIFO使能

// 数据读取寄存器 (从这里开始连读)
#define MPU_ACCEL_XOUT_H    0x3B  
#define MPU_TEMP_OUT_H      0x41
#define MPU_GYRO_XOUT_H     0x43

// 电源与ID
#define MPU_PWR_MGMT_1      0x6B  // 电源管理1 (复位/唤醒)
#define MPU_WHO_AM_I        0x75  // 器件ID (默认0x68)

// 加速度量程 ±2g (默认) -> 16384 LSB/g
#define MPU_ACCEL_SENSITIVITY  16384.0f 

#define MPU_GYRO_SENSITIVITY   16.4f

// ==========================================================
// 2. 对象结构体定义 (支持多设备)
// ==========================================================
typedef struct {
    RUN_SoftI2C_Bus_t I2C_Bus; // 每个对象独立的 I2C 引脚
    uint8_t Dev_Addr;          // 每个对象独立的 设备地址
} RUN_MPU6050_t;

// ==========================================================
// 3. 函数声明
// ==========================================================

// 初始化函数
// addr 参数填 0xD0 (AD0接地) 或 0xD2 (AD0接VCC)
void MPU6050_Init(RUN_MPU6050_t *mpu, RUN_GPIO_enum scl, RUN_GPIO_enum sda, uint8_t addr);

// 数据读取函数
void MPU6050_Get_Accel(RUN_MPU6050_t *mpu, int16_t *ax, int16_t *ay, int16_t *az);
void MPU6050_Get_Gyro (RUN_MPU6050_t *mpu, int16_t *gx, int16_t *gy, int16_t *gz);
float MPU6050_Get_Temp(RUN_MPU6050_t *mpu);
void MPU6050_Get_Accel_Real(RUN_MPU6050_t *mpu, float *val_x, float *val_y, float *val_z);
void MPU6050_Get_Gyro_Real(RUN_MPU6050_t *mpu, float *val_x, float *val_y, float *val_z);

#endif