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

// ==============================================================================
// 硬件映射表 (Lookup Table)
// ------------------------------------------------------------------------------
// 【注意】必须严格对应 RUN_PWM.h 中的 RUN_PWM_enum 枚举顺序！
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
    // Default Pin Mapping
    {TIM2, RCC_APB1Periph_TIM2, 0, GPIOA, GPIO_Pin_0,  RCC_APB2Periph_GPIOA, 1, 0}, // PA0
    {TIM2, RCC_APB1Periph_TIM2, 0, GPIOA, GPIO_Pin_1,  RCC_APB2Periph_GPIOA, 2, 0}, // PA1
    {TIM2, RCC_APB1Periph_TIM2, 0, GPIOA, GPIO_Pin_2,  RCC_APB2Periph_GPIOA, 3, 0}, // PA2
    {TIM2, RCC_APB1Periph_TIM2, 0, GPIOA, GPIO_Pin_3,  RCC_APB2Periph_GPIOA, 4, 0}, // PA3
    
    // Full Remap Pin Mapping (JTAG 冲突)
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
// 函数实现 (Register Direct Access 版)
// ==============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      PWM 初始化函数 (智能自适应版)
// 参数说明      pwm_ch          PWM通道枚举值
// 参数说明      freq            PWM频率 (Hz)
// 参数说明      duty            初始占空比 (0 ~ 10000)
// 返回参数      void
//-------------------------------------------------------------------------------------------------------------------
void RUN_pwm_init(RUN_PWM_enum pwm_ch, uint32_t freq, uint32_t duty)
{
    if (pwm_ch >= PWM_MAX) return;
    if (freq == 0) freq = 1000;

    // =========================================================
    // 1. 频率自适应计算算法
    // =========================================================
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

    // =========================================================
    // 2. 开启外设时钟 (RCC Register)
    // =========================================================
    RCC->APB2ENR |= pwm_cfg[pwm_ch].gpio_rcc;
    RCC->APB2ENR |= RCC_APB2Periph_AFIO; 
    
    if (pwm_cfg[pwm_ch].is_apb2) {
        RCC->APB2ENR |= pwm_cfg[pwm_ch].tim_rcc;
    } else {
        RCC->APB1ENR |= pwm_cfg[pwm_ch].tim_rcc;
    }

    // =========================================================
    // 3. 特殊重映射处理 (JTAG/SWD 冲突保护)
    // =========================================================
    if (pwm_cfg[pwm_ch].remap == GPIO_FullRemap_TIM2) {
        uint32_t tmpreg = AFIO->MAPR;
        tmpreg &= 0xF0FFFFFF; 
        tmpreg |= 0x02000000; 
        AFIO->MAPR = tmpreg;
    }

    // =========================================================
    // 4. 执行引脚重映射 (AFIO Register)
    // =========================================================
    if (pwm_cfg[pwm_ch].remap != 0) {
        AFIO->MAPR |= pwm_cfg[pwm_ch].remap;
    }

    // =========================================================
    // 5. GPIO 模式配置 (GPIO CRH/CRL Register)
    // =========================================================
    uint32_t currentpin = pwm_cfg[pwm_ch].gpio_pin;
    uint32_t pinpos = 0;
    while (!((currentpin >> pinpos) & 0x01)) {
        pinpos++;
    }

    // 【修改点】这里增加了 volatile 关键字
    volatile uint32_t* cr_reg; 
    uint32_t shift;
    
    if (pinpos < 8) {
        cr_reg = &pwm_cfg[pwm_ch].gpio_port->CRL;
        shift = pinpos * 4;
    } else {
        cr_reg = &pwm_cfg[pwm_ch].gpio_port->CRH;
        shift = (pinpos - 8) * 4;
    }

    *cr_reg &= ~(0x0F << shift);
    *cr_reg |= (0x0B << shift); // AF_PP (0xB)

    // =========================================================
    // 6. 定时器时基初始化 (TIM PSC/ARR Register)
    // =========================================================
    pwm_cfg[pwm_ch].tim_base->ARR = arr_val;
    pwm_cfg[pwm_ch].tim_base->PSC = psc_val;
    pwm_cfg[pwm_ch].tim_base->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS); 
    pwm_cfg[pwm_ch].tim_base->EGR = TIM_PSCReloadMode_Immediate; 

    // =========================================================
    // 7. 初始占空比计算
    // =========================================================
    if (duty > 10000) duty = 10000;
    uint16_t ccr_val = (uint32_t)duty * (arr_val + 1) / 10000;

    // =========================================================
    // 8. PWM 通道模式配置 (TIM CCMR/CCER Register)
    // =========================================================
    uint16_t mode_config = (TIM_OCMode_PWM1 | TIM_OCPreload_Enable); 
    
    switch (pwm_cfg[pwm_ch].channel) {
        case 1:
            pwm_cfg[pwm_ch].tim_base->CCMR1 &= 0xFF00; 
            pwm_cfg[pwm_ch].tim_base->CCMR1 |= mode_config; 
            pwm_cfg[pwm_ch].tim_base->CCER |= TIM_CCER_CC1E; 
            pwm_cfg[pwm_ch].tim_base->CCR1 = ccr_val;
            break;
        case 2:
            pwm_cfg[pwm_ch].tim_base->CCMR1 &= 0x00FF;
            pwm_cfg[pwm_ch].tim_base->CCMR1 |= (mode_config << 8);
            pwm_cfg[pwm_ch].tim_base->CCER |= TIM_CCER_CC2E;
            pwm_cfg[pwm_ch].tim_base->CCR2 = ccr_val;
            break;
        case 3:
            pwm_cfg[pwm_ch].tim_base->CCMR2 &= 0xFF00;
            pwm_cfg[pwm_ch].tim_base->CCMR2 |= mode_config;
            pwm_cfg[pwm_ch].tim_base->CCER |= TIM_CCER_CC3E;
            pwm_cfg[pwm_ch].tim_base->CCR3 = ccr_val;
            break;
        case 4:
            pwm_cfg[pwm_ch].tim_base->CCMR2 &= 0x00FF;
            pwm_cfg[pwm_ch].tim_base->CCMR2 |= (mode_config << 8);
            pwm_cfg[pwm_ch].tim_base->CCER |= TIM_CCER_CC4E;
            pwm_cfg[pwm_ch].tim_base->CCR4 = ccr_val;
            break;
    }

    // =========================================================
    // 9. 使能定时器
    // =========================================================
    pwm_cfg[pwm_ch].tim_base->CR1 |= TIM_CR1_CEN;
    
    if (pwm_cfg[pwm_ch].tim_base == TIM1 || pwm_cfg[pwm_ch].tim_base == TIM8) {
        pwm_cfg[pwm_ch].tim_base->BDTR |= TIM_BDTR_MOE;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      动态修改 PWM 占空比 (Register Direct Access)
//-------------------------------------------------------------------------------------------------------------------
void RUN_pwm_set(RUN_PWM_enum pwm_ch, uint32_t duty)
{
    if (pwm_ch >= PWM_MAX) return;
    if (duty > 10000) duty = 10000;

    // 直接读取寄存器 ARR
    uint32_t current_arr = pwm_cfg[pwm_ch].tim_base->ARR;

    // 计算 CCR
    uint16_t ccr_val = (uint32_t)duty * (current_arr + 1) / 10000;

    // 直接写入寄存器 CCRx
    switch (pwm_cfg[pwm_ch].channel) {
        case 1: pwm_cfg[pwm_ch].tim_base->CCR1 = ccr_val; break;
        case 2: pwm_cfg[pwm_ch].tim_base->CCR2 = ccr_val; break;
        case 3: pwm_cfg[pwm_ch].tim_base->CCR3 = ccr_val; break;
        case 4: pwm_cfg[pwm_ch].tim_base->CCR4 = ccr_val; break;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      动态修改 PWM 频率 (Register Direct Access)
//-------------------------------------------------------------------------------------------------------------------
void RUN_pwm_freq(RUN_PWM_enum pwm_ch, uint32_t freq)
{
    if (pwm_ch >= PWM_MAX) return;
    if (freq == 0) return;

    // 计算
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

    // 写入寄存器
    pwm_cfg[pwm_ch].tim_base->PSC = psc_val;
    pwm_cfg[pwm_ch].tim_base->ARR = arr_val;
    
    // 产生更新事件 (Update Generation)，使 PSC 立即生效
    pwm_cfg[pwm_ch].tim_base->EGR = TIM_PSCReloadMode_Immediate;
}