#include "RUN_DAC.h"

// 
// 上图展示了 DAC 的核心工作流程：
// 1. 数据写入 "数据保持寄存器 (DHR)"。
// 2. 经过一段时间 (或触发信号)，数据转移到 "数据输出寄存器 (DOR)"。
// 3. 模拟电路根据 DOR 的值输出对应电压。
// 注意：STM32F103 的 DAC 是 12位的，输出范围 0 ~ 4095。

// ==============================================================================
// 初始化函数
// ==============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      DAC 模块初始化
// 参数说明      channel         通道选择 (RUN_DAC_CH1_PA4 或 RUN_DAC_CH2_PA5)
// 返回参数      void
// 使用示例      RUN_DAC_Init(RUN_DAC_CH1_PA4);
// 备注信息      默认配置：12位右对齐、开启输出缓冲、软件触发。
//-------------------------------------------------------------------------------------------------------------------
void RUN_DAC_Init(RUN_DAC_Channel_t channel)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    DAC_InitTypeDef DAC_InitType;

    // 1. 开启时钟
    // GPIOA 挂载在 APB2，DAC 挂载在 APB1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    // 
    // 上图展示了 "GPIO_Mode_AIN" (模拟输入) 的内部结构。
    // 关键点：这一步会断开 GPIO 的施密特触发器和数字输出驱动电路。
    // 虽然名字叫 "模拟输入"，但对于 DAC 而言，这是为了让引脚处于 "浮空/无干扰" 状态，
    // 从而让 DAC 模块内部的模拟信号能直接通过该引脚输出，不受数字逻辑干扰。

    // 2. 配置 GPIO
    if (channel == RUN_DAC_CH1_PA4)
    {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; // DAC_OUT1
    }
    else if (channel == RUN_DAC_CH2_PA5)
    {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; // DAC_OUT2
    }
    else
    {
        return; // 参数非法
    }

    // 【重要】必须设为模拟输入，否则电压出不来或不准
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 
    // 上图展示了 DAC 的 "输出缓冲 (Output Buffer)"：本质上是一个电压跟随器 (运放)。
    // 开启 (Enable): 驱动能力强，可以直接驱动负载，但无法输出绝对的 0V 和 3.3V (有零点漂移)。
    // 关闭 (Disable): 驱动能力弱，只能接高阻抗负载，但能实现 0V~3.3V 轨对轨输出。
    
    // 3. 配置 DAC 参数
    DAC_InitType.DAC_Trigger = DAC_Trigger_None;       // 不使用定时器触发，手动写入即输出
    DAC_InitType.DAC_WaveGeneration = DAC_WaveGeneration_None; // 不产生噪声/三角波
    DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0; // 屏蔽位(不产生波形时无效)
    DAC_InitType.DAC_OutputBuffer = DAC_OutputBuffer_Enable; // 开启缓冲，提高带载能力

    // 4. 初始化并使能具体的通道
    if (channel == RUN_DAC_CH1_PA4)
    {
        DAC_Init(DAC_Channel_1, &DAC_InitType); 
        DAC_Cmd(DAC_Channel_1, ENABLE);          // 开启 DAC 模块
        DAC_SetChannel1Data(DAC_Align_12b_R, 0); // 初始状态输出 0V
    }
    else if (channel == RUN_DAC_CH2_PA5)
    {
        DAC_Init(DAC_Channel_2, &DAC_InitType); 
        DAC_Cmd(DAC_Channel_2, ENABLE);          
        DAC_SetChannel2Data(DAC_Align_12b_R, 0); 
    }
}

// ==============================================================================
// 控制函数
// ==============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      设置 DAC 输出电压
// 参数说明      channel         通道选择
// 参数说明      vol             目标电压 (0.0f ~ 3.3f)
// 返回参数      void
// 备注信息      计算公式: Digital = (Vol / Vref) * 4096
//               假设 Vref+ 接的是 3.3V。如果 Vref 接的是 2.5V，这里要由 3.3f 改为 2.5f。
//-------------------------------------------------------------------------------------------------------------------
void RUN_DAC_Set_Vol(RUN_DAC_Channel_t channel, float vol)
{
    uint16_t temp_val;
    
    // 软件限幅，防止计算溢出
    if(vol > 3.3f) vol = 3.3f;
    if(vol < 0.0f) vol = 0.0f;
    
    // 线性变换：电压 -> 寄存器数值
    // 公式推导：
    // $V_{out} = V_{REF+} \times \frac{DOR}{4095}$
    // $\therefore DOR = \frac{V_{out}}{3.3} \times 4096$
    temp_val = (uint16_t)(vol * 4096 / 3.3f);
    
    RUN_DAC_Set_Value(channel, temp_val);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      直接设置 DAC 寄存器值
// 参数说明      channel         通道选择
// 参数说明      val             12位 DAC 数值 (0 ~ 4095)
// 返回参数      void
// 备注信息      0 对应 0V，4095 对应 3.3V (Vref)
//-------------------------------------------------------------------------------------------------------------------
void RUN_DAC_Set_Value(RUN_DAC_Channel_t channel, uint16_t val)
{
    // 硬件限制为 12 bit
    if(val > 4095) val = 4095;
    
    // 写入 "数据保持寄存器 (DHR)"
    // 12b_R 表示 12位右对齐 (通常都用这个)
    if (channel == RUN_DAC_CH1_PA4)
    {
        DAC_SetChannel1Data(DAC_Align_12b_R, val);
    }
    else if (channel == RUN_DAC_CH2_PA5)
    {
        DAC_SetChannel2Data(DAC_Align_12b_R, val);
    }
    
    // 由于配置为 DAC_Trigger_None (软件触发)，
    // 数据写入 DHR 后，会在 1 个时钟周期后自动转入 DOR，引脚电压随即改变。
}