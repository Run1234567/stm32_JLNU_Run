#ifndef _RUN_EXTI_H_
#define _RUN_EXTI_H_

#include "stm32f10x.h"
#include "RUN_Gpio.h" // 必须包含，才能找到 RUN_GPIO_enum 定义

typedef enum
{
    // Line 0
    EXTI_Line0_PA0 = A0, EXTI_Line0_PB0 = B0, EXTI_Line0_PC0 = C0, 
    EXTI_Line0_PE0 = E0, EXTI_Line0_PF0 = F0, EXTI_Line0_PG0 = G0,
    EXTI_Line0_PD0_Disabled = 0xFFFF, // PD0 晶振

    // Line 1
    EXTI_Line1_PA1 = A1, EXTI_Line1_PB1 = B1, EXTI_Line1_PC1 = C1, 
    EXTI_Line1_PE1 = E1, EXTI_Line1_PF1 = F1, EXTI_Line1_PG1 = G1,
    EXTI_Line1_PD1_Disabled = 0xFFFF, // PD1 晶振

    // Line 2
    EXTI_Line2_PA2 = A2, EXTI_Line2_PB2 = B2, EXTI_Line2_PC2 = C2, EXTI_Line2_PD2 = D2,
    EXTI_Line2_PE2 = E2, EXTI_Line2_PF2 = F2, EXTI_Line2_PG2 = G2,

    // Line 3
    EXTI_Line3_PA3 = A3, EXTI_Line3_PB3 = B3, EXTI_Line3_PC3 = C3, EXTI_Line3_PD3 = D3,
    EXTI_Line3_PE3 = E3, EXTI_Line3_PF3 = F3, EXTI_Line3_PG3 = G3,

    // Line 4
    EXTI_Line4_PA4 = A4, EXTI_Line4_PB4 = B4, EXTI_Line4_PC4 = C4, EXTI_Line4_PD4 = D4,
    EXTI_Line4_PE4 = E4, EXTI_Line4_PF4 = F4, EXTI_Line4_PG4 = G4,

    // Line 5-9
    EXTI_Line5_PA5 = A5, EXTI_Line5_PB5 = B5, EXTI_Line5_PC5 = C5, EXTI_Line5_PE5 = E5,
    EXTI_Line6_PA6 = A6, EXTI_Line6_PB6 = B6, EXTI_Line6_PC6 = C6, EXTI_Line6_PE6 = E6,
    EXTI_Line7_PA7 = A7, EXTI_Line7_PB7 = B7, EXTI_Line7_PC7 = C7, EXTI_Line7_PE7 = E7,
    EXTI_Line8_PA8 = A8, EXTI_Line8_PB8 = B8, EXTI_Line8_PC8 = C8, EXTI_Line8_PE8 = E8,
    EXTI_Line9_PA9 = A9, EXTI_Line9_PB9 = B9, EXTI_Line9_PC9 = C9, EXTI_Line9_PE9 = E9,

    // Line 10-12
    EXTI_Line10_PA10 = A10, EXTI_Line10_PB10 = B10, EXTI_Line10_PC10 = C10,
    EXTI_Line11_PA11 = A11, EXTI_Line11_PB11 = B11, EXTI_Line11_PC11 = C11,
    EXTI_Line12_PA12 = A12, EXTI_Line12_PB12 = B12, EXTI_Line12_PC12 = C12,

    // Line 13 (PC13常用)
    EXTI_Line13_PC13 = C13, EXTI_Line13_PB13 = B13, EXTI_Line13_PE13 = E13,
    EXTI_Line13_PA13_Disabled = 0xFFFF, // SWDIO

    // Line 14-15
    EXTI_Line14_PB14 = B14, EXTI_Line14_PC14 = C14, EXTI_Line14_PE14 = E14,
    EXTI_Line14_PA14_Disabled = 0xFFFF, // SWCLK
    EXTI_Line15_PA15 = A15, EXTI_Line15_PB15 = B15, EXTI_Line15_PC15 = C15, EXTI_Line15_PE15 = E15

} RUN_EXTI_Pin_enum;

typedef void (*ExtiCallback_t)(void);

void RUN_exti_init(RUN_EXTI_Pin_enum exti_pin, 
                   EXTITrigger_TypeDef trigger_mode, 
                   uint8_t pre_priority, 
                   uint8_t sub_priority,
                   ExtiCallback_t callback);

#endif