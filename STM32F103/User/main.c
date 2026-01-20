#include "RUN_header_file.h"
int main(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	RUN_delay_init(72);
	RUN_uart_init(UART1_TX_PA9_RX_PA10,115200,1);
RUN_Stepper_t MyStepMotor = {
    .pwm_pin = PWM_TIM3_CH3_PB0, // 脉冲接 PA6
    .dir_pin = B1                // 方向接 PC0
};
RUN_Step_Init(&MyStepMotor);
	while (1)
	{
		RUN_Step_SetSpeed(&MyStepMotor, 1000);
		RUN_delay_ms(2000);
		RUN_Step_SetSpeed(&MyStepMotor, -2);
		RUN_delay_ms(2000);
	}
}
