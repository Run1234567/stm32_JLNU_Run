#ifndef _RUN_HEADER_FILE_H_
#define _RUN_HEADER_FILE_H_

/* 1. 先包含标准库和系统库 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"
#include "misc.h" // NVIC配置必需
#include "math.h"

/* 2. 再包含自己的模块库 */
#include "RUN_Gpio.h"
#include "RUN_UART.h"
#include "RUN_Isr.h" 
#include "RUN_Timer.h"
#include "RUN_PWM.h"
#include "RUN_Delay.h"
#include "RUN_Exti.h"
#include "RUN_SoftI2C.h"
#include "RUN_ADC.h"
#include "RUN_DAC.h"
#include "RUN_Flash.h"
#include "RUN_SPI.h"
#include "RUN_OneWire.h"
#include "RUN_DMA.h"


#include "RUN_MPU6050.h"
#include "RUN_AT24C02.h"
#include "RUN_W25Q64.h"
#include "RUN_DS18B20.h"
#include "RUN_Moter_Brushed.h"
#include "RUN_Moter_Stepper.h"


#include "RUN_IMU_GetAngle.h"
#include "RUN_PID.h"
#endif