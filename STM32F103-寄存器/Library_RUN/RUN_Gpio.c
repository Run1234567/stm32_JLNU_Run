#include "RUN_Gpio.h"

// ===============================================================================
// 硬件映射表 
// (完全保留原定义，用于获取端口基地址 GPIOx 和 引脚掩码 GPIO_Pin_x)
// ===============================================================================
const gpio_info_t gpio_cfg[RUN_GPIO_MAX] = {
    // --- GPIO A (0~15) ---
    {GPIOA, GPIO_Pin_0, RCC_APB2Periph_GPIOA}, {GPIOA, GPIO_Pin_1, RCC_APB2Periph_GPIOA},
    {GPIOA, GPIO_Pin_2, RCC_APB2Periph_GPIOA}, {GPIOA, GPIO_Pin_3, RCC_APB2Periph_GPIOA},
    {GPIOA, GPIO_Pin_4, RCC_APB2Periph_GPIOA}, {GPIOA, GPIO_Pin_5, RCC_APB2Periph_GPIOA},
    {GPIOA, GPIO_Pin_6, RCC_APB2Periph_GPIOA}, {GPIOA, GPIO_Pin_7, RCC_APB2Periph_GPIOA},
    {GPIOA, GPIO_Pin_8, RCC_APB2Periph_GPIOA}, {GPIOA, GPIO_Pin_9, RCC_APB2Periph_GPIOA},
    {GPIOA, GPIO_Pin_10,RCC_APB2Periph_GPIOA}, {GPIOA, GPIO_Pin_11,RCC_APB2Periph_GPIOA},
    {GPIOA, GPIO_Pin_12,RCC_APB2Periph_GPIOA}, {GPIOA, GPIO_Pin_13,RCC_APB2Periph_GPIOA},
    {GPIOA, GPIO_Pin_14,RCC_APB2Periph_GPIOA}, {GPIOA, GPIO_Pin_15,RCC_APB2Periph_GPIOA},

    // --- GPIO B (16~31) ---
    {GPIOB, GPIO_Pin_0, RCC_APB2Periph_GPIOB}, {GPIOB, GPIO_Pin_1, RCC_APB2Periph_GPIOB},
    {GPIOB, GPIO_Pin_2, RCC_APB2Periph_GPIOB}, {GPIOB, GPIO_Pin_3, RCC_APB2Periph_GPIOB},
    {GPIOB, GPIO_Pin_4, RCC_APB2Periph_GPIOB}, {GPIOB, GPIO_Pin_5, RCC_APB2Periph_GPIOB},
    {GPIOB, GPIO_Pin_6, RCC_APB2Periph_GPIOB}, {GPIOB, GPIO_Pin_7, RCC_APB2Periph_GPIOB},
    {GPIOB, GPIO_Pin_8, RCC_APB2Periph_GPIOB}, {GPIOB, GPIO_Pin_9, RCC_APB2Periph_GPIOB},
    {GPIOB, GPIO_Pin_10,RCC_APB2Periph_GPIOB}, {GPIOB, GPIO_Pin_11,RCC_APB2Periph_GPIOB},
    {GPIOB, GPIO_Pin_12,RCC_APB2Periph_GPIOB}, {GPIOB, GPIO_Pin_13,RCC_APB2Periph_GPIOB},
    {GPIOB, GPIO_Pin_14,RCC_APB2Periph_GPIOB}, {GPIOB, GPIO_Pin_15,RCC_APB2Periph_GPIOB},

    // --- GPIO C (32~47) ---
    {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC}, {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC},
    {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC}, {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC},
    {GPIOC, GPIO_Pin_4, RCC_APB2Periph_GPIOC}, {GPIOC, GPIO_Pin_5, RCC_APB2Periph_GPIOC},
    {GPIOC, GPIO_Pin_6, RCC_APB2Periph_GPIOC}, {GPIOC, GPIO_Pin_7, RCC_APB2Periph_GPIOC},
    {GPIOC, GPIO_Pin_8, RCC_APB2Periph_GPIOC}, {GPIOC, GPIO_Pin_9, RCC_APB2Periph_GPIOC},
    {GPIOC, GPIO_Pin_10,RCC_APB2Periph_GPIOC}, {GPIOC, GPIO_Pin_11,RCC_APB2Periph_GPIOC},
    {GPIOC, GPIO_Pin_12,RCC_APB2Periph_GPIOC}, {GPIOC, GPIO_Pin_13,RCC_APB2Periph_GPIOC},
    {GPIOC, GPIO_Pin_14,RCC_APB2Periph_GPIOC}, {GPIOC, GPIO_Pin_15,RCC_APB2Periph_GPIOC},

    // --- GPIO D (48~63) ---
    {GPIOD, GPIO_Pin_0, RCC_APB2Periph_GPIOD}, {GPIOD, GPIO_Pin_1, RCC_APB2Periph_GPIOD},
    {GPIOD, GPIO_Pin_2, RCC_APB2Periph_GPIOD}, {GPIOD, GPIO_Pin_3, RCC_APB2Periph_GPIOD},
    {GPIOD, GPIO_Pin_4, RCC_APB2Periph_GPIOD}, {GPIOD, GPIO_Pin_5, RCC_APB2Periph_GPIOD},
    {GPIOD, GPIO_Pin_6, RCC_APB2Periph_GPIOD}, {GPIOD, GPIO_Pin_7, RCC_APB2Periph_GPIOD},
    {GPIOD, GPIO_Pin_8, RCC_APB2Periph_GPIOD}, {GPIOD, GPIO_Pin_9, RCC_APB2Periph_GPIOD},
    {GPIOD, GPIO_Pin_10,RCC_APB2Periph_GPIOD}, {GPIOD, GPIO_Pin_11,RCC_APB2Periph_GPIOD},
    {GPIOD, GPIO_Pin_12,RCC_APB2Periph_GPIOD}, {GPIOD, GPIO_Pin_13,RCC_APB2Periph_GPIOD},
    {GPIOD, GPIO_Pin_14,RCC_APB2Periph_GPIOD}, {GPIOD, GPIO_Pin_15,RCC_APB2Periph_GPIOD},

    // --- GPIO E (64~79) ---
    {GPIOE, GPIO_Pin_0, RCC_APB2Periph_GPIOE}, {GPIOE, GPIO_Pin_1, RCC_APB2Periph_GPIOE},
    {GPIOE, GPIO_Pin_2, RCC_APB2Periph_GPIOE}, {GPIOE, GPIO_Pin_3, RCC_APB2Periph_GPIOE},
    {GPIOE, GPIO_Pin_4, RCC_APB2Periph_GPIOE}, {GPIOE, GPIO_Pin_5, RCC_APB2Periph_GPIOE},
    {GPIOE, GPIO_Pin_6, RCC_APB2Periph_GPIOE}, {GPIOE, GPIO_Pin_7, RCC_APB2Periph_GPIOE},
    {GPIOE, GPIO_Pin_8, RCC_APB2Periph_GPIOE}, {GPIOE, GPIO_Pin_9, RCC_APB2Periph_GPIOE},
    {GPIOE, GPIO_Pin_10,RCC_APB2Periph_GPIOE}, {GPIOE, GPIO_Pin_11,RCC_APB2Periph_GPIOE},
    {GPIOE, GPIO_Pin_12,RCC_APB2Periph_GPIOE}, {GPIOE, GPIO_Pin_13,RCC_APB2Periph_GPIOE},
    {GPIOE, GPIO_Pin_14,RCC_APB2Periph_GPIOE}, {GPIOE, GPIO_Pin_15,RCC_APB2Periph_GPIOE},

    // --- GPIO F (80~95) ---
    {GPIOF, GPIO_Pin_0, RCC_APB2Periph_GPIOF}, {GPIOF, GPIO_Pin_1, RCC_APB2Periph_GPIOF},
    {GPIOF, GPIO_Pin_2, RCC_APB2Periph_GPIOF}, {GPIOF, GPIO_Pin_3, RCC_APB2Periph_GPIOF},
    {GPIOF, GPIO_Pin_4, RCC_APB2Periph_GPIOF}, {GPIOF, GPIO_Pin_5, RCC_APB2Periph_GPIOF},
    {GPIOF, GPIO_Pin_6, RCC_APB2Periph_GPIOF}, {GPIOF, GPIO_Pin_7, RCC_APB2Periph_GPIOF},
    {GPIOF, GPIO_Pin_8, RCC_APB2Periph_GPIOF}, {GPIOF, GPIO_Pin_9, RCC_APB2Periph_GPIOF},
    {GPIOF, GPIO_Pin_10,RCC_APB2Periph_GPIOF}, {GPIOF, GPIO_Pin_11,RCC_APB2Periph_GPIOF},
    {GPIOF, GPIO_Pin_12,RCC_APB2Periph_GPIOF}, {GPIOF, GPIO_Pin_13,RCC_APB2Periph_GPIOF},
    {GPIOF, GPIO_Pin_14,RCC_APB2Periph_GPIOF}, {GPIOF, GPIO_Pin_15,RCC_APB2Periph_GPIOF},

    // --- GPIO G (96~111) ---
    {GPIOG, GPIO_Pin_0, RCC_APB2Periph_GPIOG}, {GPIOG, GPIO_Pin_1, RCC_APB2Periph_GPIOG},
    {GPIOG, GPIO_Pin_2, RCC_APB2Periph_GPIOG}, {GPIOG, GPIO_Pin_3, RCC_APB2Periph_GPIOG},
    {GPIOG, GPIO_Pin_4, RCC_APB2Periph_GPIOG}, {GPIOG, GPIO_Pin_5, RCC_APB2Periph_GPIOG},
    {GPIOG, GPIO_Pin_6, RCC_APB2Periph_GPIOG}, {GPIOG, GPIO_Pin_7, RCC_APB2Periph_GPIOG},
    {GPIOG, GPIO_Pin_8, RCC_APB2Periph_GPIOG}, {GPIOG, GPIO_Pin_9, RCC_APB2Periph_GPIOG},
    {GPIOG, GPIO_Pin_10,RCC_APB2Periph_GPIOG}, {GPIOG, GPIO_Pin_11,RCC_APB2Periph_GPIOG},
    {GPIOG, GPIO_Pin_12,RCC_APB2Periph_GPIOG}, {GPIOG, GPIO_Pin_13,RCC_APB2Periph_GPIOG},
    {GPIOG, GPIO_Pin_14,RCC_APB2Periph_GPIOG}, {GPIOG, GPIO_Pin_15,RCC_APB2Periph_GPIOG},
};

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      GPIO 初始化配置 (寄存器直接操作版)
// 参数说明      pin             选择 GPIO 引脚
// 参数说明      mode            配置 GPIO 模式
// 参数说明      default_level   配置输出默认电平
// 备注信息      直接操作 APB2ENR, CRL/CRH, ODR, BSRR/BRR 寄存器
//-------------------------------------------------------------------------------------------------------------------
void RUN_gpio_init(RUN_GPIO_enum pin, RUN_GPIO_Mode mode, uint8_t default_level)
{
    if (pin >= RUN_GPIO_MAX) return;

    GPIO_TypeDef* GPIOx = gpio_cfg[pin].port; 
    uint32_t current_mode_conf = 0;
    
    // 1. 开启时钟
    // 直接操作 RCC_APB2ENR 寄存器，通过“或”运算使能对应时钟
    RCC->APB2ENR |= gpio_cfg[pin].rcc;

    // 2. 准备寄存器配置值 (MODE + CNF)
    // STM32F103 的配置位为 4 位： [CNF1 CNF0 MODE1 MODE0]
    // MODE: 00(Input), 11(Output 50MHz)
    switch (mode)
    {
        // 推挽输出 50MHz (MODE=11, CNF=00) -> 0011b = 0x3
        case GPO:    current_mode_conf = 0x3; break; 
        
        // 开漏输出 50MHz (MODE=11, CNF=01) -> 0111b = 0x7
        case GPO_OD: current_mode_conf = 0x7; break; 
        
        // 上拉/下拉输入 (MODE=00, CNF=10) -> 1000b = 0x8
        // 注：上下拉的区别在于 ODR 寄存器的值，配置位本身是一样的
        case GPI_PU: 
        case GPI_PD: current_mode_conf = 0x8; break; 
        
        // 模拟输入 (MODE=00, CNF=00) -> 0000b = 0x0
        case AIN:    current_mode_conf = 0x0; break; 
        
        // 浮空输入 (MODE=00, CNF=01) -> 0100b = 0x4
        default:     current_mode_conf = 0x4; break; 
    }

    // 3. 写入 CRL 或 CRH 寄存器
    // pin_index 计算：0~15
    uint8_t pin_index = pin % 16; 

    if (pin_index < 8)
    {
        // 使用 CRL (Control Register Low) 控制 Pin 0-7
        // 每 4 位控制一个引脚，先清零对应的 4 位，再写入新值
        uint32_t shift = pin_index * 4;
        GPIOx->CRL &= ~(0xF << shift);        // 清除原有配置
        GPIOx->CRL |= (current_mode_conf << shift); // 写入新配置
    }
    else
    {
        // 使用 CRH (Control Register High) 控制 Pin 8-15
        uint32_t shift = (pin_index - 8) * 4;
        GPIOx->CRH &= ~(0xF << shift);
        GPIOx->CRH |= (current_mode_conf << shift);
    }

    // 4. 处理上拉/下拉 (通过 ODR 寄存器)
    if (mode == GPI_PU)
    {
        GPIOx->BSRR = gpio_cfg[pin].pin; // Set Bit -> 上拉
    }
    else if (mode == GPI_PD)
    {
        GPIOx->BRR = gpio_cfg[pin].pin;  // Reset Bit -> 下拉
    }

    // 5. 如果是输出模式，设置初始电平
    if (mode == GPO || mode == GPO_OD)
    {
        RUN_gpio_set(pin, default_level);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      GPIO 输出电平设置 (寄存器版)
// 备注信息      使用 BSRR 和 BRR 寄存器进行原子操作，无需读改写
//-------------------------------------------------------------------------------------------------------------------
void RUN_gpio_set(RUN_GPIO_enum pin, uint8_t level)
{
    if (pin >= RUN_GPIO_MAX) return;

    if (level)
    {
        // BSRR 低 16 位写 1 为置位 (Set)
        gpio_cfg[pin].port->BSRR = gpio_cfg[pin].pin;
    }
    else
    {
        // BRR 低 16 位写 1 为复位 (Reset)
        gpio_cfg[pin].port->BRR = gpio_cfg[pin].pin;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      GPIO 输出电平翻转 (寄存器版)
// 备注信息      通过异或 ODR 寄存器实现
//-------------------------------------------------------------------------------------------------------------------
void RUN_gpio_toggle(RUN_GPIO_enum pin)
{
    if (pin >= RUN_GPIO_MAX) return;
    
    // ^= 异或操作：0变1，1变0
    gpio_cfg[pin].port->ODR ^= gpio_cfg[pin].pin;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      GPIO 输入电平读取 (寄存器版)
// 备注信息      直接读取 IDR 寄存器
//-------------------------------------------------------------------------------------------------------------------
uint8_t RUN_gpio_get(RUN_GPIO_enum pin)
{
    if (pin >= RUN_GPIO_MAX) return 0;

    // 检查 IDR (Input Data Register) 中对应的位
    if ((gpio_cfg[pin].port->IDR & gpio_cfg[pin].pin) != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}