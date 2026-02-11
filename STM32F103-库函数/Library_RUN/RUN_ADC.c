#include "RUN_Adc.h"

// 
// 上图展示了逐次逼近型 (SAR) ADC 的内部结构：
// 包括 采样保持电路 (Sample & Hold)、DAC、比较器 和 SAR 逻辑控制。

// ==============================================================================
// 初始化函数
// ==============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      ADC1 模块初始化
// 参数说明      void
// 返回参数      void
// 使用示例      RUN_ADC_Init();
// 备注信息      配置为：独立模式、单次转换、软件触发。
//               【关键】ADC 时钟频率不能超过 14MHz，否则会导致读数错乱或死机。
//-------------------------------------------------------------------------------------------------------------------
void RUN_ADC_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;

    // 1. 开启 ADC1 外设时钟
    // ADC 挂载在 APB2 总线上
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // 
    // 上图展示了 ADC 的时钟源：APB2 (PCLK2) -> 分频器 -> ADCCLK

    // 2. 设置 ADC 分频因子 (ADCCLK)
    // STM32F103 的 ADC 最大工作频率为 14MHz。
    // 如果系统时钟(SystemCoreClock) 为 72MHz，APB2也是 72MHz。
    // - Div4: 72/4 = 18MHz (>14MHz, 超频，不稳定)
    // - Div6: 72/6 = 12MHz (<14MHz, 推荐)
    // - Div8: 72/8 = 9MHz  (<14MHz, 较慢但稳)
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); 

    // 3. 配置 ADC 工作参数
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  // 独立模式 (单ADC工作)
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;       // 关闭扫描 (因为我们是单次手动读取)
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // 关闭连续转换 (读一次停一次)
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发 (不使用定时器/外部中断触发)
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;// 数据右对齐 (12位数据存放在低12位)
    ADC_InitStructure.ADC_NbrOfChannel = 1;             // 顺序进行规则转换的 ADC 通道的数目
    ADC_Init(ADC1, &ADC_InitStructure);

    // 4. 使能 ADC 外设
    // 此时 ADC 开始上电，但还没开始转换
    ADC_Cmd(ADC1, ENABLE);

    // ==========================================================
    // 5. ADC 校准流程 (Self-Calibration)
    // ==========================================================
    // 作用：消除芯片制造过程中的内部电容偏差。
    // 建议：每次上电初始化后都执行一次。
    
    // 步骤A: 复位校准寄存器
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1)); // 等待复位完成

    // 步骤B: 开始校准
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));      // 等待校准完成
}

// 
// 上图展示了 GPIO 的模拟输入模式：
// 施密特触发器 (Schmitt Trigger) 断开，信号直接进入 ADC 模块。
// 如果不配置为 AIN，数字输入电路可能会消耗额外电流并引入噪声。

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      自动配置 ADC 通道对应的 GPIO 引脚
// 参数说明      ch              自定义的通道枚举 (如 RUN_ADC_CH0_PA0)
// 返回参数      void
// 备注信息      将引脚配置为 模拟输入 (GPIO_Mode_AIN)
//-------------------------------------------------------------------------------------------------------------------
void RUN_ADC_ConfigPin(RUN_ADC_Channel_enum ch)
{
    // 这里的 if-else 链用于将用户的 "逻辑通道" 映射到 "物理引脚"
    // 必须确保 GPIO 时钟在外部或 GPIO_Init 内部已开启
    
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
// 数据读取函数
// ==============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      读取单个通道的 ADC 转换值 (Polling 方式)
// 参数说明      ch              ADC通道号 (ADC_Channel_x)
// 返回参数      uint16_t        12位原始数据 (0 ~ 4095)
// 备注信息      计算公式：
//               总转换时间 = 采样时间 + 12.5个周期
//               当前配置：55.5 + 12.5 = 68个周期
//               在 12MHz 时钟下，耗时约 5.6us
//-------------------------------------------------------------------------------------------------------------------
uint16_t RUN_ADC_Get_Value(RUN_ADC_Channel_enum ch)
{
    // 
    // 上图展示了 ADC 的两个阶段：
    // 1. 采样阶段 (Sampling): 对内部电容充电，时间越长越准，适合高阻抗信号。
    // 2. 转换阶段 (Conversion): 固定为 12.5 个时钟周期。

    // 1. 配置规则组通道 (Rank 1)
    // 意思是：我马上要转换的第一个通道就是 ch
    // 采样时间选择 55.5 周期，兼顾速度和稳定性
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_55Cycles5);

    // 2. 软件触发转换 (Software Start)
    // 相当于按下了 "开始" 按钮
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    // 3. 等待转换结束 (Polling EOC Flag)
    // 轮询 EOC (End Of Conversion) 标志位
    // 如果卡死在这里，请检查 RCC 时钟是否开启
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

    // 4. 读取数据寄存器 (DR)
    // 读取操作会自动清除 EOC 标志位
    return ADC_GetConversionValue(ADC1);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      多次采样取平均值 (软件滤波)
// 参数说明      ch              ADC通道号
// 参数说明      times           采样次数 (推荐 10~50 次)
// 返回参数      uint16_t        平均后的 ADC 值
// 使用示例      uint16_t val = RUN_ADC_Get_Average(RUN_ADC_CH1_PA1, 20);
// 备注信息      简单的均值滤波，能有效抑制电源纹波或随机噪声。
//               注意：累加变量使用 uint32_t 防止溢出。
//-------------------------------------------------------------------------------------------------------------------
uint16_t RUN_ADC_Get_Average(RUN_ADC_Channel_enum ch, uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;
    
    for(t = 0; t < times; t++)
    {
        temp_val += RUN_ADC_Get_Value(ch);
        // 如果信号变化极快，这里可以加一点微秒级延时
    }
    
    return temp_val / times;
}