#include "RUN_header_file.h"	
RUN_MPU6050_t my_mpu;
float Roll, Pitch, Yaw;float ax, ay, az;
float gx, gy, gz;	float a=0;
int main(void)
{	
	// 变量存储原始数据

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	RUN_delay_init(72);
	RUN_uart_init(UART1_TX_PA9_RX_PA10,115200,1);
	MPU6050_Init(&my_mpu, B6, B7, MPU_ADDR_DEFAULT);

	RUN_timer_init(RUN_TIM6,5);
	while (1)
	{
RUN_delay_ms(5);		printf("Accel: %.3f, %.3f, %.3f,", ax, ay, az);
printf("%.3f, %.3f, %.3f,", gx, gy, gz);		printf("%.2f,%.2f,%.2f\n", Roll, Pitch, Yaw);
	}
}
void TIM6_Callback()
{
  		// 3. 读取加速度 (原始值)
MPU6050_Get_Accel_Real(&my_mpu, &ax, &ay, &az);

// 4. 读取角速度 (原始值)
MPU6050_Get_Gyro_Real(&my_mpu, &gx, &gy, &gz);
RUN_CF_Update(ax, ay, az, gx, gy, gz, &Roll, &Pitch, &Yaw);

}