#include "RUN_MPU6050.h"

/**
  * @brief  内部静态函数：I2C 连续读取 (Burst Read)
  * @note   这是私有函数，不对外开放。用于一次性读取多个连续的寄存器，效率比读单个快一倍。
  * @param  mpu:      MPU6050 对象句柄
  * @param  reg_addr: 起始寄存器地址
  * @param  buffer:   数据存储缓冲区指针
  * @param  count:    要读取的字节数量
  * @retval None
  */
static void MPU_Read_Block(RUN_MPU6050_t *mpu, uint8_t reg_addr, uint8_t *buffer, uint8_t count)
{
    uint8_t i;
    // 使用 mpu->I2C_Bus 操作引脚
    // 使用 mpu->Dev_Addr 作为写地址
    
    RUN_I2C_Start(&mpu->I2C_Bus);
    RUN_I2C_SendByte(&mpu->I2C_Bus, mpu->Dev_Addr);     // 写设备地址 (Write)
    RUN_I2C_WaitAck(&mpu->I2C_Bus);
    RUN_I2C_SendByte(&mpu->I2C_Bus, reg_addr);          // 写寄存器地址
    RUN_I2C_WaitAck(&mpu->I2C_Bus);
    
    RUN_I2C_Start(&mpu->I2C_Bus);
    RUN_I2C_SendByte(&mpu->I2C_Bus, mpu->Dev_Addr | 1); // 读设备地址 (Write | 1 -> Read)
    RUN_I2C_WaitAck(&mpu->I2C_Bus);
    
    for(i = 0; i < count; i++)
    {
        // 读字节。如果是最后一个字节，发NACK(0)表示结束，否则发ACK(1)继续
        buffer[i] = RUN_I2C_ReadByte(&mpu->I2C_Bus, (i == (count-1)) ? 0 : 1);
    }
    RUN_I2C_Stop(&mpu->I2C_Bus);
}

/**
  * @brief  MPU6050 初始化函数
  * @note   配置详情：
  * 1. 电源管理：解除休眠模式
  * 2. 采样率分频：125Hz (1kHz / (1+7))
  * 3. 低通滤波(DLPF)：5Hz (使数据更平滑，减少机械震动干扰)
  * 4. 陀螺仪量程：±2000 dps (检测快速旋转)
  * 5. 加速度量程：±2g (检测常规重力和运动)
  * @param  mpu:  MPU6050 对象句柄 (需定义一个全局变量传入)
  * @param  scl:  I2C 时钟引脚 (如 B6, B10...)
  * @param  sda:  I2C 数据引脚 (如 B7, B11...)
  * @param  addr: 设备地址 (0xD0: AD0接GND, 0xD2: AD0接VCC)
  * @retval None
  */
void MPU6050_Init(RUN_MPU6050_t *mpu, RUN_GPIO_enum scl, RUN_GPIO_enum sda, uint8_t addr)
{
    // 1. 记录地址
    mpu->Dev_Addr = addr;

    // 2. 初始化底层的 I2C 引脚
    RUN_I2C_Init(&mpu->I2C_Bus, scl, sda);

    // 3. 配置芯片 (注意所有操作都带 mpu 指针)
    // 复位电源管理寄存器 (解除休眠)
    RUN_I2C_WriteReg(&mpu->I2C_Bus, mpu->Dev_Addr, MPU_PWR_MGMT_1, 0x00);
    
    // 设置采样率分频 (Sample Rate = 1KHz / (1 + 7) = 125Hz)
    RUN_I2C_WriteReg(&mpu->I2C_Bus, mpu->Dev_Addr, MPU_SMPLRT_DIV, 0x07);
    
    // 设置低通滤波 (DLPF = 5Hz)
    RUN_I2C_WriteReg(&mpu->I2C_Bus, mpu->Dev_Addr, MPU_CONFIG, 0x06);
    
    // 设置陀螺仪量程 (±2000 dps) -> 对应寄存器值 0x18 (00011000)
    RUN_I2C_WriteReg(&mpu->I2C_Bus, mpu->Dev_Addr, MPU_GYRO_CONFIG, 0x18);
    
    // 设置加速度量程 (±2g) -> 对应寄存器值 0x00
    RUN_I2C_WriteReg(&mpu->I2C_Bus, mpu->Dev_Addr, MPU_ACCEL_CONFIG, 0x00);
}

/**
  * @brief  读取三轴加速度原始数据
  * @param  mpu: MPU6050 对象句柄
  * @param  ax:  返回 X 轴加速度指针 (原始 ADC 值)
  * @param  ay:  返回 Y 轴加速度指针
  * @param  az:  返回 Z 轴加速度指针
  * @retval None
  * @note   量程 ±2g 时，1g ≈ 16384
  */
void MPU6050_Get_Accel(RUN_MPU6050_t *mpu, int16_t *ax, int16_t *ay, int16_t *az)
{
    uint8_t buf[6];
    // 加速度寄存器从 0x3B (MPU_ACCEL_XOUT_H) 开始，连续 6 个字节
    MPU_Read_Block(mpu, MPU_ACCEL_XOUT_H, buf, 6); 
    
    // 合成 16 位数据 (高8位<<8 | 低8位)
    *ax = (int16_t)((buf[0] << 8) | buf[1]);
    *ay = (int16_t)((buf[2] << 8) | buf[3]);
    *az = (int16_t)((buf[4] << 8) | buf[5]);
}

/**
  * @brief  读取三轴角速度(陀螺仪)原始数据
  * @param  mpu: MPU6050 对象句柄
  * @param  gx:  返回 X 轴角速度指针
  * @param  gy:  返回 Y 轴角速度指针
  * @param  gz:  返回 Z 轴角速度指针
  * @retval None
  * @note   量程 ±2000dps 时，1度/秒 ≈ 16.4
  */
void MPU6050_Get_Gyro(RUN_MPU6050_t *mpu, int16_t *gx, int16_t *gy, int16_t *gz)
{
    uint8_t buf[6];
    // 陀螺仪寄存器从 0x43 开始，连续 6 个字节
    MPU_Read_Block(mpu, 0x43, buf, 6);
    
    *gx = (int16_t)((buf[0] << 8) | buf[1]);
    *gy = (int16_t)((buf[2] << 8) | buf[3]);
    *gz = (int16_t)((buf[4] << 8) | buf[5]);
}

/**
  * @brief  读取三轴加速度实际值 (单位: g)
  * @param  mpu: MPU6050 对象句柄
  * @param  val_x: 返回 X 轴加速度 (float)
  * @param  val_y: 返回 Y 轴加速度 (float)
  * @param  val_z: 返回 Z 轴加速度 (float)
  */
void MPU6050_Get_Accel_Real(RUN_MPU6050_t *mpu, float *val_x, float *val_y, float *val_z)
{
    int16_t raw_x, raw_y, raw_z;
    
    // 1. 先读取原始 16位 整数数据
    MPU6050_Get_Accel(mpu, &raw_x, &raw_y, &raw_z);
    
    // 2. 转化为实际物理量 (除以灵敏度)
    *val_x = (float)raw_x / MPU_ACCEL_SENSITIVITY;
    *val_y = (float)raw_y / MPU_ACCEL_SENSITIVITY;
    *val_z = (float)raw_z / MPU_ACCEL_SENSITIVITY;
}

/**
  * @brief  读取三轴角速度实际值 (单位: 度/秒)
  * @param  mpu: MPU6050 对象句柄
  * @param  val_x: 返回 X 轴角速度 (float)
  * @param  val_y: 返回 Y 轴角速度 (float)
  * @param  val_z: 返回 Z 轴角速度 (float)
  */
void MPU6050_Get_Gyro_Real(RUN_MPU6050_t *mpu, float *val_x, float *val_y, float *val_z)
{
    int16_t raw_x, raw_y, raw_z;
    
    // 1. 先读取原始 16位 整数数据
    MPU6050_Get_Gyro(mpu, &raw_x, &raw_y, &raw_z);
    
    // 2. 转化为实际物理量 (除以灵敏度)
    *val_x = (float)raw_x / MPU_GYRO_SENSITIVITY;
    *val_y = (float)raw_y / MPU_GYRO_SENSITIVITY;
    *val_z = (float)raw_z / MPU_GYRO_SENSITIVITY;
}

/**
  * @brief  读取芯片内部温度
  * @param  mpu: MPU6050 对象句柄
  * @retval float: 温度值 (单位：摄氏度 ℃)
  * @note   计算公式: Temp = (Raw / 340.0) + 36.53
  */
float MPU6050_Get_Temp(RUN_MPU6050_t *mpu)
{
    uint8_t buf[2];
    int16_t raw_temp;
    
    // 温度寄存器从 0x41 开始，2个字节
    MPU_Read_Block(mpu, 0x41, buf, 2);
    raw_temp = (int16_t)((buf[0] << 8) | buf[1]);
    
    return ((float)raw_temp / 340.0f) + 36.53f;
}