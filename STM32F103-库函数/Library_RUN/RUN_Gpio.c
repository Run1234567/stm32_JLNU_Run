#include "RUN_Gpio.h" // 必须包含自己的头文件

// ===============================================================================
// 硬件映射表 (注意：类型必须是 gpio_info_t，顺序必须对应枚举)
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
// 函数简介      GPIO 初始化配置
// 参数说明      pin             选择 GPIO 引脚 (枚举值，如 RUN_PA0)
// 参数说明      mode            配置 GPIO 模式 (GPO, GPO_OD, GPI_PU, GPI_PD, AIN 等)
// 参数说明      default_level   配置输出默认电平 (0:低电平, 1:高电平，仅在输出模式下有效)
// 返回参数      void
// 使用示例      RUN_gpio_init(RUN_PA0, GPO, 1); // 初始化PA0为推挽输出，默认高电平
// 备注信息      函数内部会自动开启对应端口的 RCC 时钟
//-------------------------------------------------------------------------------------------------------------------
void RUN_gpio_init(RUN_GPIO_enum pin, RUN_GPIO_Mode mode, uint8_t default_level)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    if (pin >= RUN_GPIO_MAX) return;

    RCC_APB2PeriphClockCmd(gpio_cfg[pin].rcc, ENABLE);

    GPIO_InitStructure.GPIO_Pin = gpio_cfg[pin].pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    switch (mode)
    {
        case GPO:    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; break;
        case GPO_OD: GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; break;
        case GPI_PU: GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;    break;
        case GPI_PD: GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;    break;
        case AIN:    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;    break;
        default:     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; break;
    }

    GPIO_Init(gpio_cfg[pin].port, &GPIO_InitStructure);

    if (mode == GPO || mode == GPO_OD)
    {
        RUN_gpio_set(pin, default_level);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      GPIO 输出电平设置
// 参数说明      pin             选择 GPIO 引脚 (枚举值)
// 参数说明      level           设置电平状态 (0:低电平, 1:高电平)
// 返回参数      void
// 使用示例      RUN_gpio_set(RUN_PA0, 0); // 设置PA0输出低电平
// 备注信息      使用标准库 GPIO_SetBits / GPIO_ResetBits 实现
//-------------------------------------------------------------------------------------------------------------------
void RUN_gpio_set(RUN_GPIO_enum pin, uint8_t level)
{
    if (pin >= RUN_GPIO_MAX) return;

    if (level)
        GPIO_SetBits(gpio_cfg[pin].port, gpio_cfg[pin].pin);
    else
        GPIO_ResetBits(gpio_cfg[pin].port, gpio_cfg[pin].pin);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      GPIO 输出电平翻转
// 参数说明      pin             选择 GPIO 引脚 (枚举值)
// 返回参数      void
// 使用示例      RUN_gpio_toggle(RUN_PA0); // 翻转PA0的输出电平
// 备注信息      函数读取当前输出寄存器(ODR)状态，并写入相反值
//-------------------------------------------------------------------------------------------------------------------
void RUN_gpio_toggle(RUN_GPIO_enum pin)
{
    if (pin >= RUN_GPIO_MAX) return;
    
    // 注意：使用的是 port (不是 group)
    uint8_t current_val = GPIO_ReadOutputDataBit(gpio_cfg[pin].port, gpio_cfg[pin].pin);
    
    if (current_val)
        GPIO_ResetBits(gpio_cfg[pin].port, gpio_cfg[pin].pin);
    else
        GPIO_SetBits(gpio_cfg[pin].port, gpio_cfg[pin].pin);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      GPIO 输入电平读取
// 参数说明      pin             选择 GPIO 引脚 (枚举值)
// 返回参数      uint8_t         返回引脚电平 (0:低电平, 1:高电平)
// 使用示例      uint8_t status = RUN_gpio_get(RUN_PA0); // 读取PA0的输入状态
// 备注信息      读取的是输入数据寄存器(IDR)
//-------------------------------------------------------------------------------------------------------------------
uint8_t RUN_gpio_get(RUN_GPIO_enum pin)
{
    if (pin >= RUN_GPIO_MAX) return 0;
    return GPIO_ReadInputDataBit(gpio_cfg[pin].port, gpio_cfg[pin].pin);
}