#include "RUN_header_file.h"	

int main(void)
{	
	// 变量存储原始数据
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	RUN_delay_init(72);
	RUN_uart_init(UART1_TX_PA9_RX_PA10,115200,1);
	RUN_pwm_init(PWM_TIM3_CH3_PB0, 20000, 5000); // 20kHz, 50%占空比
    RUN_gpio_init(B1, GPO, 1);
    RUN_gpio_init(B5, GPO, 1);
RUN_timer_init(RUN_TIM2, 500);
RUN_W25Q_Init(RUN_SPI_1_PA5_PA6_PA7,A4);
	uint16_t a=0;
a=RUN_W25Q_ReadID();
	RUN_pwm_init(PWM_TIM3_CH3_PB0,10000,0);
	int b=0;
	while (1)
	{
		b+=2;
		RUN_delay_ms(1);
		if(b>=10000)b=0;
		RUN_pwm_set(PWM_TIM3_CH3_PB0,b);
    if (UART1_RxFlag == 1)
        {
					
            // --- 处理接收到的数据 ---

            // 1. 发送回显 (原样发回去)
            printf("Received: %s\r\n", UART1_RxPacket);
            
            
            // --- 处理完毕，清除标志位，允许接收下一包 ---
            UART1_RxFlag = 0; 
        }
				 printf("%d\r\n", a);
	}
}
