#include "RUN_header_file.h"
#include "RUN_PWM.h" 

// ==============================================================================
// 内部硬件配置结构体
// ------------------------------------------------------------------------------
// 该结构体用于抽象硬件差异，实现统一的 PWM 控制接口
// ==============================================================================
typedef struct {
    TIM_TypeDef* tim_base;   // 定时器外设基地址 (TIM1 ~ TIM8)
    uint32_t     tim_rcc;    // 定时器时钟控制宏 (RCC_APB1... / RCC_APB2...)
    uint8_t      is_apb2;    // 总线标志位 [1:APB2 (72M), 0:APB1 (36M*2=72M)]
    
    GPIO_TypeDef* gpio_port; // PWM输出对应的 GPIO 端口
    uint16_t      gpio_pin;  // PWM输出对应的 GPIO 引脚
    uint32_t      gpio_rcc;  // GPIO 时钟控制宏
    
    uint8_t       channel;   // 定时器通道号 (1~4)
    uint32_t      remap;     // AFIO重映射参数 (0:默认引脚, 其他:部分或完全重映射)
} pwm_info_t;

// 
// 上图展示了 STM32 定时器的核心结构：时钟源 -> 预分频器(PSC) -> 计数器(CNT) -> 比较寄存器(CCR)

// ==============================================================================
// 硬件映射表 (Lookup Table)
// ------------------------------------------------------------------------------
// 【注意】必须严格对应 RUN_PWM.h 中的 RUN_PWM_enum 枚举顺序！
// 作用：通过查表法，将用户简单的枚举索引转化为复杂的底层硬件配置
// ==============================================================================
const pwm_info_t pwm_cfg[PWM_MAX] = {
    // ---------------- TIM1 (APB2 72MHz) ----------------
    // Default Pin Mapping
    {TIM1, RCC_APB2Periph_TIM1, 1, GPIOA, GPIO_Pin_8,  RCC_APB2Periph_GPIOA, 1, 0}, // PA8
    {TIM1, RCC_APB2Periph_TIM1, 1, GPIOA, GPIO_Pin_9,  RCC_APB2Periph_GPIOA, 2, 0}, // PA9
    {TIM1, RCC_APB2Periph_TIM1, 1, GPIOA, GPIO_Pin_10, RCC_APB2Periph_GPIOA, 3, 0}, // PA10
    {TIM1, RCC_APB2Periph_TIM1, 1, GPIOA, GPIO_Pin_11, RCC_APB2Periph_GPIOA, 4, 0}, // PA11
    
    // Full Remap Pin Mapping
    {TIM1, RCC_APB2Periph_TIM1, 1, GPIOE, GPIO_Pin_9,  RCC_APB2Periph_GPIOE, 1, GPIO_FullRemap_TIM1}, // PE9
    {TIM1, RCC_APB2Periph_TIM1, 1, GPIOE, GPIO_Pin_11, RCC_APB2Periph_GPIOE, 2, GPIO_FullRemap_TIM1}, // PE11
    {TIM1, RCC_APB2Periph_TIM1, 1, GPIOE, GPIO_Pin_13, RCC_APB2Periph_GPIOE, 3, GPIO_FullRemap_TIM1}, // PE13
    {TIM1, RCC_APB2Periph_TIM1, 1, GPIOE, GPIO_Pin_14, RCC_APB2Periph_GPIOE, 4, GPIO_FullRemap_TIM1}, // PE14

    // ---------------- TIM2 (APB1 72MHz) ----------------
    // Default Pin Mapping (注意：PA15/PB3/PB4 默认是 JTAG 引脚)
    {TIM2, RCC_APB1Periph_TIM2, 0, GPIOA, GPIO_Pin_0,  RCC_APB2Periph_GPIOA, 1, 0}, // PA0
    {TIM2, RCC_APB1Periph_TIM2, 0, GPIOA, GPIO_Pin_1,  RCC_APB2Periph_GPIOA, 2, 0}, // PA1
    {TIM2, RCC_APB1Periph_TIM2, 0, GPIOA, GPIO_Pin_2,  RCC_APB2Periph_GPIOA, 3, 0}, // PA2
    {TIM2, RCC_APB1Periph_TIM2, 0, GPIOA, GPIO_Pin_3,  RCC_APB2Periph_GPIOA, 4, 0}, // PA3
    
    // Full Remap Pin Mapping (这里涉及到 JTAG 复用冲突)
    {TIM2, RCC_APB1Periph_TIM2, 0, GPIOA, GPIO_Pin_15, RCC_APB2Periph_GPIOA, 1, GPIO_FullRemap_TIM2}, // PA15 (JTDI)
    {TIM2, RCC_APB1Periph_TIM2, 0, GPIOB, GPIO_Pin_3,  RCC_APB2Periph_GPIOB, 2, GPIO_FullRemap_TIM2}, // PB3  (JTDO)
    {TIM2, RCC_APB1Periph_TIM2, 0, GPIOB, GPIO_Pin_10, RCC_APB2Periph_GPIOB, 3, GPIO_FullRemap_TIM2}, // PB10
    {TIM2, RCC_APB1Periph_TIM2, 0, GPIOB, GPIO_Pin_11, RCC_APB2Periph_GPIOB, 4, GPIO_FullRemap_TIM2}, // PB11

    // ---------------- TIM3 (APB1 72MHz) ----------------
    // Default Pin Mapping
    {TIM3, RCC_APB1Periph_TIM3, 0, GPIOA, GPIO_Pin_6,  RCC_APB2Periph_GPIOA, 1, 0}, // PA6
    {TIM3, RCC_APB1Periph_TIM3, 0, GPIOA, GPIO_Pin_7,  RCC_APB2Periph_GPIOA, 2, 0}, // PA7
    {TIM3, RCC_APB1Periph_TIM3, 0, GPIOB, GPIO_Pin_0,  RCC_APB2Periph_GPIOB, 3, 0}, // PB0
    {TIM3, RCC_APB1Periph_TIM3, 0, GPIOB, GPIO_Pin_1,  RCC_APB2Periph_GPIOB, 4, 0}, // PB1
    
    // Full Remap Pin Mapping
    {TIM3, RCC_APB1Periph_TIM3, 0, GPIOC, GPIO_Pin_6,  RCC_APB2Periph_GPIOC, 1, GPIO_FullRemap_TIM3}, // PC6
    {TIM3, RCC_APB1Periph_TIM3, 0, GPIOC, GPIO_Pin_7,  RCC_APB2Periph_GPIOC, 2, GPIO_FullRemap_TIM3}, // PC7
    {TIM3, RCC_APB1Periph_TIM3, 0, GPIOC, GPIO_Pin_8,  RCC_APB2Periph_GPIOC, 3, GPIO_FullRemap_TIM3}, // PC8
    {TIM3, RCC_APB1Periph_TIM3, 0, GPIOC, GPIO_Pin_9,  RCC_APB2Periph_GPIOC, 4, GPIO_FullRemap_TIM3}, // PC9

    // ---------------- TIM4 (APB1 72MHz) ----------------
    // Default Pin Mapping
    {TIM4, RCC_APB1Periph_TIM4, 0, GPIOB, GPIO_Pin_6,  RCC_APB2Periph_GPIOB, 1, 0}, // PB6
    {TIM4, RCC_APB1Periph_TIM4, 0, GPIOB, GPIO_Pin_7,  RCC_APB2Periph_GPIOB, 2, 0}, // PB7
    {TIM4, RCC_APB1Periph_TIM4, 0, GPIOB, GPIO_Pin_8,  RCC_APB2Periph_GPIOB, 3, 0}, // PB8
    {TIM4, RCC_APB1Periph_TIM4, 0, GPIOB, GPIO_Pin_9,  RCC_APB2Periph_GPIOB, 4, 0}, // PB9
    
    // Remap Pin Mapping
    {TIM4, RCC_APB1Periph_TIM4, 0, GPIOD, GPIO_Pin_12, RCC_APB2Periph_GPIOD, 1, GPIO_Remap_TIM4}, // PD12
    {TIM4, RCC_APB1Periph_TIM4, 0, GPIOD, GPIO_Pin_13, RCC_APB2Periph_GPIOD, 2, GPIO_Remap_TIM4}, // PD13
    {TIM4, RCC_APB1Periph_TIM4, 0, GPIOD, GPIO_Pin_14, RCC_APB2Periph_GPIOD, 3, GPIO_Remap_TIM4}, // PD14
    {TIM4, RCC_APB1Periph_TIM4, 0, GPIOD, GPIO_Pin_15, RCC_APB2Periph_GPIOD, 4, GPIO_Remap_TIM4}, // PD15

    // ---------------- TIM5 (APB1 72MHz) ----------------
    // Default Pin Mapping
    {TIM5, RCC_APB1Periph_TIM5, 0, GPIOA, GPIO_Pin_0,  RCC_APB2Periph_GPIOA, 1, 0}, // PA0
    {TIM5, RCC_APB1Periph_TIM5, 0, GPIOA, GPIO_Pin_1,  RCC_APB2Periph_GPIOA, 2, 0}, // PA1
    {TIM5, RCC_APB1Periph_TIM5, 0, GPIOA, GPIO_Pin_2,  RCC_APB2Periph_GPIOA, 3, 0}, // PA2
    {TIM5, RCC_APB1Periph_TIM5, 0, GPIOA, GPIO_Pin_3,  RCC_APB2Periph_GPIOA, 4, 0}, // PA3

    // ---------------- TIM8 (APB2 72MHz) ----------------
    // Default Pin Mapping
    {TIM8, RCC_APB2Periph_TIM8, 1, GPIOC, GPIO_Pin_6,  RCC_APB2Periph_GPIOC, 1, 0}, // PC6
    {TIM8, RCC_APB2Periph_TIM8, 1, GPIOC, GPIO_Pin_7,  RCC_APB2Periph_GPIOC, 2, 0}, // PC7
    {TIM8, RCC_APB2Periph_TIM8, 1, GPIOC, GPIO_Pin_8,  RCC_APB2Periph_GPIOC, 3, 0}, // PC8
    {TIM8, RCC_APB2Periph_TIM8, 1, GPIOC, GPIO_Pin_9,  RCC_APB2Periph_GPIOC, 4, 0}, // PC9
};

// ==============================================================================
// 函数实现
// ==============================================================================

// 
// 上图展示了 PWM 的基本概念：频率决定周期的长短，占空比(Duty)决定高电平持续的时间比例。

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      PWM 初始化函数 (智能自适应版)
// 参数说明      pwm_ch          PWM通道枚举值 (如 RUN_PWM_TIM1_CH1)
// 参数说明      freq            PWM频率 (Hz)
// 参数说明      duty            初始占空比 (范围 0 ~ 10000，对应 0% ~ 100%)
// 返回参数      void
// 使用示例      RUN_pwm_init(RUN_PWM_TIM1_CH1, 20000, 5000); // 20kHz, 50%占空比
// 备注信息      函数会自动计算最佳的 PSC (预分频) 和 ARR (重装载值) 以匹配目标频率。
//               优先保证 PSC=0 以获得最高的占空比调节分辨率。
//-------------------------------------------------------------------------------------------------------------------
void RUN_pwm_init(RUN_PWM_enum pwm_ch, uint32_t freq, uint32_t duty)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    if (pwm_ch >= PWM_MAX) return;
    if (freq == 0) freq = 1000; // 安全保护：防止除以零

    // =========================================================
    // 1. 频率自适应计算算法
    // =========================================================
    // 公式：PWM频率 = 系统时钟 / ((ARR+1) * (PSC+1))
    // 系统时钟固定为 72MHz (72,000,000)
    // 策略：
    //   - 如果目标频率较高 (周期数 < 65535)，则不分频 (PSC=0)，直接用 ARR 计数，精度最高。
    //   - 如果目标频率极低 (如电机控制的 50Hz)，ARR 不够用，才增加 PSC。
    
    uint32_t period_cycles = 72000000 / freq; 
    uint16_t psc_val = 0;
    uint16_t arr_val = 0;

    if (period_cycles < 65535) {
        // [情况A: 高频] (如 20kHz -> 3600 cycles)
        // 3600 < 65535，可以直接装入 ARR
        psc_val = 0;
        arr_val = period_cycles - 1;
    } else {
        // [情况B: 低频] (如 50Hz -> 1440000 cycles)
        // 1440000 > 65535，必须分频
        // 计算需要的 PSC (粗略计算)
        psc_val = (period_cycles / 65535);
        // 根据算出的 PSC 反推精确的 ARR
        arr_val = (period_cycles / (psc_val + 1)) - 1;
    }
    // ---------------------------

    // =========================================================
    // 2. 开启外设时钟
    // =========================================================
    // 必须同时开启 GPIO、AFIO (用于重映射) 和 TIM 时钟
    RCC_APB2PeriphClockCmd(pwm_cfg[pwm_ch].gpio_rcc | RCC_APB2Periph_AFIO, ENABLE);
    
    if (pwm_cfg[pwm_ch].is_apb2) 
        RCC_APB2PeriphClockCmd(pwm_cfg[pwm_ch].tim_rcc, ENABLE);
    else                             
        RCC_APB1PeriphClockCmd(pwm_cfg[pwm_ch].tim_rcc, ENABLE);

    // =========================================================
    // 3. 特殊重映射处理 (JTAG/SWD 冲突保护)
    // =========================================================
    // STM32 的 PB3, PB4, PA13, PA14, PA15 默认是 JTAG/SWD 调试引脚。
    // 如果要将 PB3 或 PA15 用作普通 PWM，必须禁用 JTAG 功能。
    // 这里选择 "JTAGDisable, SWJ_JTAGDisable" 模式：禁用 JTAG，但保留 SWD (两线仿真)。
    if (pwm_cfg[pwm_ch].remap == GPIO_FullRemap_TIM2) {
        GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    }

    // =========================================================
    // 4. 执行引脚重映射
    // =========================================================
    if (pwm_cfg[pwm_ch].remap != 0) {
        GPIO_PinRemapConfig(pwm_cfg[pwm_ch].remap, ENABLE);
    }

    // =========================================================
    // 5. GPIO 模式配置
    // =========================================================
    // PWM 输出必须配置为 "复用推挽输出" (AF_PP)
    GPIO_InitStructure.GPIO_Pin = pwm_cfg[pwm_ch].gpio_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(pwm_cfg[pwm_ch].gpio_port, &GPIO_InitStructure);

    // =========================================================
    // 6. 定时器时基初始化
    // =========================================================
    TIM_TimeBaseStructure.TIM_Period = arr_val;     // 自动重装载值 (决定频率)
    TIM_TimeBaseStructure.TIM_Prescaler = psc_val;  // 预分频系数
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(pwm_cfg[pwm_ch].tim_base, &TIM_TimeBaseStructure);

    // =========================================================
    // 7. 初始占空比计算
    // =========================================================
    // 逻辑：将用户输入的 0~10000 线性映射到 0~ARR
    if (duty > 10000) duty = 10000;
    uint16_t ccr_val = (uint32_t)duty * (arr_val + 1) / 10000;

    // =========================================================
    // 8. PWM 通道模式配置
    // =========================================================
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; // PWM模式1: CNT<CCR有效
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable; // 互补输出无效(高级定时器用)
    TIM_OCInitStructure.TIM_Pulse = ccr_val; // 比较值 (CCR)
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

    // 根据通道号初始化对应的 OCx
    switch (pwm_cfg[pwm_ch].channel) {
        case 1: TIM_OC1Init(pwm_cfg[pwm_ch].tim_base, &TIM_OCInitStructure); 
                TIM_OC1PreloadConfig(pwm_cfg[pwm_ch].tim_base, TIM_OCPreload_Enable); break;
        case 2: TIM_OC2Init(pwm_cfg[pwm_ch].tim_base, &TIM_OCInitStructure); 
                TIM_OC2PreloadConfig(pwm_cfg[pwm_ch].tim_base, TIM_OCPreload_Enable); break;
        case 3: TIM_OC3Init(pwm_cfg[pwm_ch].tim_base, &TIM_OCInitStructure); 
                TIM_OC3PreloadConfig(pwm_cfg[pwm_ch].tim_base, TIM_OCPreload_Enable); break;
        case 4: TIM_OC4Init(pwm_cfg[pwm_ch].tim_base, &TIM_OCInitStructure); 
                TIM_OC4PreloadConfig(pwm_cfg[pwm_ch].tim_base, TIM_OCPreload_Enable); break;
    }

    // =========================================================
    // 9. 使能定时器
    // =========================================================
    TIM_Cmd(pwm_cfg[pwm_ch].tim_base, ENABLE);
    
    // 【重要】高级定时器 (TIM1, TIM8) 主输出使能 (MOE)
    // 对于 TIM1/TIM8，必须额外开启 MOE (Main Output Enable) 位，否则引脚无波形输出。
    // 普通定时器不需要此步骤。
    if (pwm_cfg[pwm_ch].tim_base == TIM1 || pwm_cfg[pwm_ch].tim_base == TIM8) {
        TIM_CtrlPWMOutputs(pwm_cfg[pwm_ch].tim_base, ENABLE);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      动态修改 PWM 占空比
// 参数说明      pwm_ch          PWM通道枚举
// 参数说明      duty            目标占空比 (0 ~ 10000)
// 返回参数      void
// 使用示例      RUN_pwm_set(RUN_PWM_TIM1_CH1, 2500); // 设置为 25% 占空比
// 备注信息      函数会读取当前的 ARR 值进行重映射，因此修改频率后本函数依然有效。
//-------------------------------------------------------------------------------------------------------------------
void RUN_pwm_set(RUN_PWM_enum pwm_ch, uint32_t duty)
{
    if (pwm_ch >= PWM_MAX) return;
    if (duty > 10000) duty = 10000;

    // 获取当前自动重装载值 (ARR)，确保计算基于当前频率
    uint32_t current_arr = pwm_cfg[pwm_ch].tim_base->ARR;

    // 重新映射：Input(0~10000) -> CCR(0~ARR)
    uint16_t ccr_val = (uint32_t)duty * (current_arr + 1) / 10000;

    switch (pwm_cfg[pwm_ch].channel) {
        case 1: TIM_SetCompare1(pwm_cfg[pwm_ch].tim_base, ccr_val); break;
        case 2: TIM_SetCompare2(pwm_cfg[pwm_ch].tim_base, ccr_val); break;
        case 3: TIM_SetCompare3(pwm_cfg[pwm_ch].tim_base, ccr_val); break;
        case 4: TIM_SetCompare4(pwm_cfg[pwm_ch].tim_base, ccr_val); break;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      动态修改 PWM 频率
// 参数说明      pwm_ch          PWM通道枚举
// 参数说明      freq            新频率 (Hz)
// 返回参数      void
// 使用示例      RUN_pwm_freq(RUN_PWM_TIM1_CH1, 1000); // 运行时改为 1kHz
// 备注信息      注意：修改频率会改变 ARR，因此占空比的 CCR 值也需要跟随调整。
//               如果在修改频率后不调用 RUN_pwm_set，占空比比例可能会发生变化。
//-------------------------------------------------------------------------------------------------------------------
void RUN_pwm_freq(RUN_PWM_enum pwm_ch, uint32_t freq)
{
    if (pwm_ch >= PWM_MAX) return;
    if (freq == 0) return;

    // 重新计算 PSC 和 ARR (逻辑同 Init 函数)
    uint32_t period_cycles = 72000000 / freq; 
    uint16_t psc_val = 0;
    uint16_t arr_val = 0;

    if (period_cycles < 65535) {
        psc_val = 0;
        arr_val = period_cycles - 1;
    } else {
        psc_val = (period_cycles / 65535);
        arr_val = (period_cycles / (psc_val + 1)) - 1;
    }

    // 更新寄存器
    // 使用 TIM_PSCReloadMode_Immediate 确保预分频器立即更新，而不是等到下个周期
    TIM_PrescalerConfig(pwm_cfg[pwm_ch].tim_base, psc_val, TIM_PSCReloadMode_Immediate);
    TIM_SetAutoreload(pwm_cfg[pwm_ch].tim_base, arr_val);
}