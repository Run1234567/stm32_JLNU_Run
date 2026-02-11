#include "RUN_OneWire.h"

// 
// 上图展示了单总线的物理拓扑：
// 1. 只有一根数据线 (DQ) 和一根地线 (GND)。
// 2. 空闲状态必须由外部电阻 (4.7kΩ) 拉高到 VCC。
// 3. 任何设备（主机或从机）想发送 "0"，就将总线拉低；想发送 "1"，就释放总线（由电阻拉高）。

// ==========================================================
// 私有变量：记录当前使用的引脚资源
// ==========================================================
static GPIO_TypeDef* g_OW_PORT;
static uint16_t      g_OW_PIN;
static uint32_t      g_OW_CLK; 

// ==========================================================
// 内部函数：GPIO 模式快速切换
// ==========================================================
// 注意：频繁调用 GPIO_Init 会消耗一定 CPU 时间。
// 如果追求极致性能，建议直接操作寄存器 (CRL/CRH) 来切换输入输出。

/**
 * @brief  切换为推挽输出 (Master控制总线)
 */
static void OW_IO_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = g_OW_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(g_OW_PORT, &GPIO_InitStructure);
}

/**
 * @brief  切换为输入模式 (释放总线，由电阻拉高，准备读取Slave)
 */
static void OW_IO_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = g_OW_PIN;
    
    // 【关键硬件配置】
    // 如果板子上焊接了 4.7k 上拉电阻 -> 使用 IN_FLOATING (浮空输入)
    // 如果板子上没有电阻 -> 必须使用 IPU (内部上拉)，否则读取不稳定
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    
    GPIO_Init(g_OW_PORT, &GPIO_InitStructure);
}

// -----------------------------------------------------------
// 电平控制宏 (简化代码可读性)
// -----------------------------------------------------------
#define OW_DQ_1     GPIO_SetBits(g_OW_PORT, g_OW_PIN)       // 拉高 (释放)
#define OW_DQ_0     GPIO_ResetBits(g_OW_PORT, g_OW_PIN)     // 拉低 (占用)
#define OW_DQ_READ  GPIO_ReadInputDataBit(g_OW_PORT, g_OW_PIN) // 读取

// ==========================================================
// 1. 初始化函数
// ==========================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      单总线初始化
// 参数说明      port            GPIO端口 (如 GPIOA)
// 参数说明      pin             GPIO引脚 (如 GPIO_Pin_0)
// 返回参数      void
// 备注信息      配置 GPIO 时钟并释放总线。
//               初始化后，总线应处于高电平状态。
//-------------------------------------------------------------------------------------------------------------------
void RUN_OneWire_Init(GPIO_TypeDef* port, uint16_t pin)
{
    g_OW_PORT = port;
    g_OW_PIN = pin;
    
    // 自动匹配时钟源
    if(port == GPIOA)      g_OW_CLK = RCC_APB2Periph_GPIOA;
    else if(port == GPIOB) g_OW_CLK = RCC_APB2Periph_GPIOB;
    else if(port == GPIOC) g_OW_CLK = RCC_APB2Periph_GPIOC;
    else if(port == GPIOD) g_OW_CLK = RCC_APB2Periph_GPIOD;
    else if(port == GPIOE) g_OW_CLK = RCC_APB2Periph_GPIOE;
    else if(port == GPIOF) g_OW_CLK = RCC_APB2Periph_GPIOF;
    else if(port == GPIOG) g_OW_CLK = RCC_APB2Periph_GPIOG;
    
    RCC_APB2PeriphClockCmd(g_OW_CLK, ENABLE);
    
    OW_IO_OUT();
    OW_DQ_1; // 默认拉高，确保总线空闲
}

// 
// 上图展示了复位时序：
// 1. Reset Pulse: 主机拉低至少 480us。
// 2. Release: 主机释放，电阻把线拉高。
// 3. Presence Pulse: 从机检测到上升沿后，等待 15-60us，然后拉低 60-240us 表示“我在”。

// ==========================================================
// 2. 复位信号 (Reset & Check Presence)
// ==========================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      复位总线并检测设备是否存在
// 返回参数      0: 成功 (检测到设备存在)
//               1: 失败 (无设备回应或线路故障)
// 备注信息      这是每一次通信周期的第一步。
//-------------------------------------------------------------------------------------------------------------------
uint8_t RUN_OneWire_Reset(void)
{
    uint8_t retry = 0;
    
    OW_IO_OUT();
    OW_DQ_0;             // 1. 主机拉低总线
    RUN_delay_us(750);   //    保持 480~960us (复位脉冲)
    
    OW_DQ_1;             // 2. 主机释放总线
    RUN_delay_us(15);    //    等待 15-60us (让电压回升，并等待从机醒来)
    
    OW_IO_IN();          // 3. 切换输入，侦听从机
    
    // 4. 等待从机拉低 (Presence Pulse)
    // 如果一直读到高电平(1)，说明没人拉低，超时则认为无设备
    while (OW_DQ_READ && retry < 200)
    {
        retry++;
        RUN_delay_us(1);
    }
    
    if(retry >= 200) return 1; // 错误：无人回应
    
    retry = 0;
    // 5. 等待从机释放总线
    // 从机拉低完会松手，总线应该变回高电平
    while (!OW_DQ_READ && retry < 240)
    {
        retry++;
        RUN_delay_us(1);
    }
    
    if(retry >= 240) return 1; // 错误：从机一直拉低(可能短路)

    return 0; // 握手成功
}

// 
// 上图展示了“写时隙”：
// 写 1: 主机拉低 <15us，然后立刻释放。
// 写 0: 主机拉低 >60us (由主机强行按住)。

// ==========================================================
// 3. 写一位 (Write Bit)
// ==========================================================
static void RUN_OneWire_WriteBit(uint8_t bit)
{
    OW_IO_OUT();
    OW_DQ_0; // 拉低，启动写时隙
    
    if(bit)
    {
        // --- 写 1 ---
        // 协议要求：拉低 1~15us，通常取 2us
        RUN_delay_us(2);
        OW_DQ_1;          // 释放总线
        // 之后的 >60us 时间里，总线是高电平，从机采样到高电平即认为是 1
        RUN_delay_us(60); 
    }
    else
    {
        // --- 写 0 ---
        // 协议要求：拉低 60~120us
        // 主机一直按住低电平，从机采样到低电平即认为是 0
        RUN_delay_us(60);
        OW_DQ_1;          // 释放总线
        RUN_delay_us(2);  // 恢复时间
    }
}

// 
// 上图展示了“读时隙”：
// 1. 主机拉低 >1us 通知从机“我要读了”。
// 2. 主机释放总线。
// 3. 如果从机想发 0，它会接手继续拉低；如果想发 1，它就什么都不做(由电阻拉高)。
// 4. 主机必须在 15us 内读取电平 (采样点)。

// ==========================================================
// 4. 读一位 (Read Bit)
// ==========================================================
static uint8_t RUN_OneWire_ReadBit(void)
{
    uint8_t bit = 0;
    
    OW_IO_OUT();
    OW_DQ_0; // 1. 拉低，启动读时隙
    RUN_delay_us(2); // 保持 >1us
    
    OW_DQ_1; // 2. 主机释放，把控制权交给从机
    OW_IO_IN(); 
    
    // 3. 等待采样点
    // 关键点：OneWire 的数据有效时间很短，必须在 15us 窗口内读取
    // 延时太久(>15us)则从机可能已经释放，读到的全是 1
    RUN_delay_us(12); 
    
    // 4. 采样
    if(OW_DQ_READ) bit = 1;
    else           bit = 0;
    
    RUN_delay_us(50); // 等待整个时隙结束 (至少 60us)
    
    return bit;
}

// ==========================================================
// 5. 字节操作封装
// ==========================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      写一个字节 (LSB First)
// 参数说明      data            要写入的8位数据
// 备注信息      OneWire 协议规定：低位先出 (Least Significant Bit First)
//-------------------------------------------------------------------------------------------------------------------
void RUN_OneWire_WriteByte(uint8_t data)
{
    uint8_t i;
    // 警告：OneWire 对时序极其敏感。
    // 如果系统中有高优先级中断频繁打断这里的 for 循环，建议在此处加上 __disable_irq();
    
    for (i = 0; i < 8; i++)
    {
        RUN_OneWire_WriteBit(data & 0x01); // 取最低位发送
        data >>= 1; // 右移，准备下一位
    }
    
    // 恢复中断： __enable_irq();
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      读一个字节 (LSB First)
// 返回参数      读取到的数据
//-------------------------------------------------------------------------------------------------------------------
uint8_t RUN_OneWire_ReadByte(void)
{
    uint8_t i, data = 0;
    
    for (i = 0; i < 8; i++)
    {
        if(RUN_OneWire_ReadBit())
        {
            data |= (1 << i); // 读到1，置位对应位 (从 bit0 开始填)
        }
        // 读到0不需要操作，因为 data 初始化即为 0
    }
    return data;
}