#include "RUN_Adc.h"

// 
// 上图展示了逐次逼近型 (SAR) ADC 的内部结构：
// 包括 采样保持电路 (Sample & Hold)、DAC、比较器 和 SAR 逻辑控制。

// ==============================================================================
// 初始化函数 (寄存器版本)
// ==============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      ADC1 模块初始化
// 备注信息      配置为：独立模式、单次转换、软件触发。
//               【关键】ADC 时钟频率不能超过 14MHz。
//-------------------------------------------------------------------------------------------------------------------
void RUN_ADC_Init(void)
{
    // 1. 开启 ADC1 外设时钟
    RCC->APB2ENR |= (1 << 9);

    // 2. 设置 ADC 分频因子 (Div6 = 12MHz)
    RCC->CFGR &= ~(3 << 14);
    RCC->CFGR |=  (2 << 14);

    // 3. 配置 CR1
    ADC1->CR1 = 0; // 独立模式，非扫描

    // 4. 配置 CR2 (关键修改点！)
    // [Bit 20] EXTTRIG = 1 : 必须开启外部触发，SWSTART 才能生效
    // [Bit 19:17] EXTSEL = 111 : 触发源选择 SWSTART
    // [Bit 11] ALIGN = 0 : 右对齐
    ADC1->CR2 = 0; // 先清零
    ADC1->CR2 |= (1 << 20); // <--- 之前漏了这句：开启外部触发
    ADC1->CR2 |= (7 << 17); // EXTSEL = 111 (Software Start)
    
    // 5. 开启 ADC 电源 (ADON)
    ADC1->CR2 |= (1 << 0);

    // === 关键：上电后必须等待至少 2个 ADC 时钟周期才能校准 ===
    // 简单的软延时，防止 ADC 还没稳就开始校准
    for(int i=0; i<1000; i++) __NOP(); 

    // 6. ADC 校准流程
    // 复位校准
    ADC1->CR2 |= (1 << 3);
    while(ADC1->CR2 & (1 << 3));

    // 开始校准
    ADC1->CR2 |= (1 << 2);
    while(ADC1->CR2 & (1 << 2));
}

// 
// 上图展示了 GPIO 的模拟输入模式：
// 施密特触发器 (Schmitt Trigger) 断开，信号直接进入 ADC 模块。

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      自动配置 ADC 通道对应的 GPIO 引脚
// 备注信息      这里的逻辑保持不变，调用底层的 RUN_gpio_init
//-------------------------------------------------------------------------------------------------------------------
void RUN_ADC_ConfigPin(RUN_ADC_Channel_enum ch)
{
    // 这里的 if-else 链用于将用户的 "逻辑通道" 映射到 "物理引脚"
    // 假设 RUN_gpio_init 已经适配好或原本就是兼容的
    
    if(ch == RUN_ADC_CH0_PA0)      RUN_gpio_init(A0, AIN, 0);
    else if(ch == RUN_ADC_CH1_PA1) RUN_gpio_init(A1, AIN, 0);
    else if(ch == RUN_ADC_CH2_PA2) RUN_gpio_init(A2, AIN, 0);
    else if(ch == RUN_ADC_CH3_PA3) RUN_gpio_init(A3, AIN, 0);
    else if(ch == RUN_ADC_CH4_PA4) RUN_gpio_init(A4, AIN, 0);
    else if(ch == RUN_ADC_CH5_PA5) RUN_gpio_init(A5, AIN, 0);
    else if(ch == RUN_ADC_CH6_PA6) RUN_gpio_init(A6, AIN, 0);
    else if(ch == RUN_ADC_CH7_PA7) RUN_gpio_init(A7, AIN, 0);
    
    else if(ch == RUN_ADC_CH8_PB0) RUN_gpio_init(B0, AIN, 0);
    else if(ch == RUN_ADC_CH9_PB1) RUN_gpio_init(B1, AIN, 0);
    
    else if(ch == RUN_ADC_CH10_PC0) RUN_gpio_init(C0, AIN, 0);
    else if(ch == RUN_ADC_CH11_PC1) RUN_gpio_init(C1, AIN, 0);
    else if(ch == RUN_ADC_CH12_PC2) RUN_gpio_init(C2, AIN, 0);
    else if(ch == RUN_ADC_CH13_PC3) RUN_gpio_init(C3, AIN, 0);
    else if(ch == RUN_ADC_CH14_PC4) RUN_gpio_init(C4, AIN, 0);
    else if(ch == RUN_ADC_CH15_PC5) RUN_gpio_init(C5, AIN, 0);
}

// ==============================================================================
// 数据读取函数 (寄存器版本)
// ==============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      读取单个通道的 ADC 转换值 (Polling 方式)
// 参数说明      ch              ADC通道号 (0~17)
// 返回参数      uint16_t        12位原始数据 (0 ~ 4095)
//-------------------------------------------------------------------------------------------------------------------
uint16_t RUN_ADC_Get_Value(RUN_ADC_Channel_enum ch)
{
    // 
    // 上图展示了 ADC 的两个阶段：采样和转换。

    uint8_t channel = (uint8_t)ch;

    // 1. 配置采样时间 (55.5 Cycles)
    // 对应寄存器值: 000(1.5), 001(7.5), 010(13.5), 011(28.5), 100(41.5), 101(55.5)
    // 我们需要写入 5 (即二进制 101)
    
    // SMPR2 控制通道 0-9, SMPR1 控制通道 10-17
    if (channel < 10)
    {
        ADC1->SMPR2 &= ~(7 << (3 * channel));      // 清除原来的设置 (3位掩码)
        ADC1->SMPR2 |=  (5 << (3 * channel));      // 设置为 55.5 周期
    }
    else
    {
        ADC1->SMPR1 &= ~(7 << (3 * (channel - 10))); 
        ADC1->SMPR1 |=  (5 << (3 * (channel - 10))); 
    }

    // 2. 配置规则组序列 (Rank 1)
    // SQR3 的 4:0 位决定了规则组第1个转换的通道
    ADC1->SQR3 &= ~(0x1F << 0); // 清除 Rank1
    ADC1->SQR3 |= (channel << 0);

    // 3. 软件触发转换 (Software Start)
    // CR2: Bit 22 = SWSTART
    // 注意：如果是单次转换，EXTTRIG 必须有效或者使用 SWSTART 位
    ADC1->CR2 |= (1 << 22);

    // 4. 等待转换结束 (Polling EOC Flag)
    // SR: Bit 1 = EOC (End of Conversion)
    while(!(ADC1->SR & (1 << 1)));

    // 5. 读取数据寄存器 (DR)
    // 读取 DR 寄存器会自动清除 EOC 标志
    return (uint16_t)ADC1->DR;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      多次采样取平均值 (软件滤波)
// 备注信息      纯逻辑函数，无需修改寄存器操作
//-------------------------------------------------------------------------------------------------------------------
uint16_t RUN_ADC_Get_Average(RUN_ADC_Channel_enum ch, uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;
    
    for(t = 0; t < times; t++)
    {
        temp_val += RUN_ADC_Get_Value(ch);
    }
    
    return temp_val / times;
}