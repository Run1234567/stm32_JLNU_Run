#include "RUN_SoftI2C.h"

// 
// 上图展示了 I2C 的物理层核心：开漏输出 + 上拉电阻。
// 1. "写1" 实际上是 MOS 管截止，靠电阻把电压拉高 (高阻态)。
// 2. "写0" 是 MOS 管导通，直接接地 (强下拉)。
// 这种结构允许“线与”逻辑，防止多个设备同时发送数据时短路。

// -----------------------------------------------------------
// 私有宏定义 (简化底层电平操作)
// -----------------------------------------------------------
// 封装 GPIO 操作，方便移植到不同平台
#define I2C_SCL_H(bus)  RUN_gpio_set((bus)->SCL_Pin, 1) // 释放 SCL (被上拉电阻拉高)
#define I2C_SCL_L(bus)  RUN_gpio_set((bus)->SCL_Pin, 0) // 拉低 SCL
#define I2C_SDA_H(bus)  RUN_gpio_set((bus)->SDA_Pin, 1) // 释放 SDA (被上拉电阻拉高)
#define I2C_SDA_L(bus)  RUN_gpio_set((bus)->SDA_Pin, 0) // 拉低 SDA
#define I2C_SDA_READ(bus) RUN_gpio_get((bus)->SDA_Pin)  // 读取 SDA 电平

// I2C 速率控制
// 4us 延时对应半个周期，完整周期约 8us -> 125kHz (标准 I2C 为 100kHz)
#define I2C_DELAY_TIME 4 

static void i2c_delay(void)
{
    RUN_delay_us(I2C_DELAY_TIME);
}

// ============================================================================
//                                核心驱动函数
// ============================================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      I2C 总线初始化
// 参数说明      bus             总线结构体指针 (用于保存 SCL/SDA 的引脚号)
// 参数说明      scl             SCL 时钟引脚枚举
// 参数说明      sda             SDA 数据引脚枚举
// 返回参数      void
// 使用示例      RUN_I2C_Init(&OLED_Bus, RUN_PIN_B6, RUN_PIN_B7);
// 备注信息      【关键】引脚配置为 "通用开漏输出 (GPO_OD)"。
//               I2C 协议要求总线空闲时为高电平，必须配合外部上拉电阻使用。
//-------------------------------------------------------------------------------------------------------------------
void RUN_I2C_Init(RUN_SoftI2C_Bus_t* bus, RUN_GPIO_enum scl, RUN_GPIO_enum sda)
{
    bus->SCL_Pin = scl;
    bus->SDA_Pin = sda;
    
    // 初始化 GPIO
    // 初始状态输出 1 (高阻态)，由上拉电阻拉高，表示总线空闲
    RUN_gpio_init(scl, GPO_OD, 1); 
    RUN_gpio_init(sda, GPO_OD, 1); 
}

// 
// 上图展示了起始(Start)和停止(Stop)信号的时序：
// Start: SCL 为高时，SDA 下降沿。
// Stop:  SCL 为高时，SDA 上升沿。

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      产生 I2C 起始信号 (Start Condition)
// 参数说明      bus             总线对象
// 返回参数      void
// 备注信息      时序：SCL 高电平期间，SDA 由高变低。
//               动作结束后拉低 SCL，占用总线，准备发送数据。
//-------------------------------------------------------------------------------------------------------------------
void RUN_I2C_Start(RUN_SoftI2C_Bus_t* bus)
{
    I2C_SDA_H(bus); // 确保 SDA 为高
    I2C_SCL_H(bus); // 确保 SCL 为高
    i2c_delay();
    
    I2C_SDA_L(bus); // SCL 高期间，SDA 拉低 -> START
    i2c_delay();
    
    I2C_SCL_L(bus); // 钳住总线，准备发送数据
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      产生 I2C 停止信号 (Stop Condition)
// 参数说明      bus             总线对象
// 返回参数      void
// 备注信息      时序：SCL 高电平期间，SDA 由低变高。
//               动作结束后释放总线 (SCL/SDA 均变高)。
//-------------------------------------------------------------------------------------------------------------------
void RUN_I2C_Stop(RUN_SoftI2C_Bus_t* bus)
{
    I2C_SCL_L(bus); // 先拉低，确保数据变化时 SCL 为低
    I2C_SDA_L(bus); // 准备 SDA 为低
    i2c_delay();
    
    I2C_SCL_H(bus); // SCL 拉高
    i2c_delay();
    
    I2C_SDA_H(bus); // SCL 高期间，SDA 拉高 -> STOP
    i2c_delay();
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      等待从机应答信号 (Wait ACK)
// 参数说明      bus             总线对象
// 返回参数      uint8_t         0: 成功 (接收到 ACK)
//                               1: 失败 (接收到 NACK 或超时)
// 备注信息      Master 释放 SDA，如果 Slave 拉低 SDA，则表示 ACK (应答)。
//-------------------------------------------------------------------------------------------------------------------
uint8_t RUN_I2C_WaitAck(RUN_SoftI2C_Bus_t* bus)
{
    uint8_t timeOut = 0;
    
    I2C_SDA_H(bus); // 主机释放 SDA (写1进入高阻态)，交由从机控制
    i2c_delay();
    I2C_SCL_H(bus); // SCL 拉高，采样 SDA
    i2c_delay();
    
    // 检查 SDA 电平
    // 如果 SDA 为高，说明从机没有拉低，还在等待或者无设备
    while(I2C_SDA_READ(bus))
    {
        timeOut++;
        if(timeOut > 250) // 超时机制，防止死循环
        {
            RUN_I2C_Stop(bus);
            return 1; // 无应答
        }
    }
    
    I2C_SCL_L(bus); // 时钟周期结束，拉低 SCL
    return 0;       // 收到应答
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      发送一个字节 (Send Byte)
// 参数说明      bus             总线对象
// 参数说明      byte            要发送的 8 位数据
// 返回参数      void
// 备注信息      I2C 数据传输遵循 "MSB First" (最高位先出)。
//               数据在 SCL 低电平时变化，在 SCL 高电平时保持稳定供从机采样。
//-------------------------------------------------------------------------------------------------------------------
void RUN_I2C_SendByte(RUN_SoftI2C_Bus_t* bus, uint8_t byte)
{
    uint8_t i;
    for(i = 0; i < 8; i++)
    {
        I2C_SCL_L(bus); // 拉低 SCL，允许 SDA 数据变化
        
        // 检查最高位 (0x80 = 1000 0000)
        if(byte & 0x80) I2C_SDA_H(bus);
        else            I2C_SDA_L(bus);
        
        byte <<= 1;     // 左移一位，准备下一位
        i2c_delay();
        
        I2C_SCL_H(bus); // 拉高 SCL，通知从机读取数据
        i2c_delay();
    }
    I2C_SCL_L(bus); // 发送完毕，拉低 SCL
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      读取一个字节 (Read Byte)
// 参数说明      bus             总线对象
// 参数说明      ack             应答策略: 1=发送ACK (还要继续读), 0=发送NACK (读完不读了)
// 返回参数      uint8_t         读取到的数据
// 备注信息      主机释放 SDA，并在 SCL 高电平期间读取 SDA 的状态。
//-------------------------------------------------------------------------------------------------------------------
uint8_t RUN_I2C_ReadByte(RUN_SoftI2C_Bus_t* bus, uint8_t ack)
{
    uint8_t i, receive = 0;
    
    I2C_SDA_H(bus); // 主机释放总线，切换为输入模式
    
    for(i = 0; i < 8; i++)
    {
        I2C_SCL_L(bus); // 准备下一位
        i2c_delay();
        I2C_SCL_H(bus); // SCL 拉高，数据稳定有效
        
        receive <<= 1;  // 先左移，空出最低位
        if(I2C_SDA_READ(bus)) receive++; // 如果读到 1，最低位 +1
        
        i2c_delay();
    }
    
    // --- 发送应答位 (Master ACK/NACK) ---
    I2C_SCL_L(bus);
    if(ack) I2C_SDA_L(bus); // 发送 ACK (拉低): 告诉从机 "我还要读"
    else    I2C_SDA_H(bus); // 发送 NACK (拉高): 告诉从机 "读完了，释放总线吧"
    
    i2c_delay();
    I2C_SCL_H(bus); // 产生时钟脉冲让从机读取应答
    i2c_delay();
    I2C_SCL_L(bus);
    
    return receive;
}

// ============================================================================
//                                寄存器操作接口
// ============================================================================

// 
// 上图展示了完整的 I2C 读写帧格式。
// 写操作: Start -> DeviceAddr(W) -> ACK -> RegAddr -> ACK -> Data -> ACK -> Stop
// 读操作: Start -> DeviceAddr(W) -> ACK -> RegAddr -> ACK -> Start(Repeated) -> DeviceAddr(R) -> ACK -> Data -> NACK -> Stop

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      I2C 写寄存器 (封装了完整的写协议)
// 参数说明      bus             总线对象
// 参数说明      dev_addr        设备I2C地址 (如 0x78)
// 参数说明      reg_addr        目标寄存器地址
// 参数说明      data            写入的数据
// 返回参数      void
// 使用示例      RUN_I2C_WriteReg(&OledBus, 0x78, 0x00, 0xAF);
//-------------------------------------------------------------------------------------------------------------------
void RUN_I2C_WriteReg(RUN_SoftI2C_Bus_t* bus, uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
    RUN_I2C_Start(bus);
    
    RUN_I2C_SendByte(bus, dev_addr); // 发送设备地址 (写模式，最后一位通常为0)
    RUN_I2C_WaitAck(bus);
    
    RUN_I2C_SendByte(bus, reg_addr); // 发送寄存器地址
    RUN_I2C_WaitAck(bus);
    
    RUN_I2C_SendByte(bus, data);     // 发送数据
    RUN_I2C_WaitAck(bus);
    
    RUN_I2C_Stop(bus);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      I2C 读寄存器 (封装了完整的读协议)
// 参数说明      bus             总线对象
// 参数说明      dev_addr        设备地址 (只需提供写地址，函数内部会自动处理读位)
// 参数说明      reg_addr        目标寄存器地址
// 返回参数      uint8_t         寄存器内的值
// 备注信息      读过程稍微复杂，需要用到 "重复起始信号 (Repeated Start)"：
//               先“写”寄存器地址，再重新 START 并切换为“读”模式。
//-------------------------------------------------------------------------------------------------------------------
uint8_t RUN_I2C_ReadReg(RUN_SoftI2C_Bus_t* bus, uint8_t dev_addr, uint8_t reg_addr)
{
    uint8_t data;
    
    // 1. 哑写入 (Dummy Write): 告诉设备我们要操作哪个寄存器
    RUN_I2C_Start(bus);
    RUN_I2C_SendByte(bus, dev_addr); // 发送写地址
    RUN_I2C_WaitAck(bus);
    RUN_I2C_SendByte(bus, reg_addr); // 写入寄存器指针
    RUN_I2C_WaitAck(bus);
    
    // 2. 重复起始，进入读模式
    RUN_I2C_Start(bus);
    RUN_I2C_SendByte(bus, dev_addr | 0x01); // 发送读地址 (写地址 | 1)
    RUN_I2C_WaitAck(bus);
    
    // 3. 读取数据
    // 参数传 0 (NACK)，表示只读这一个字节，读完就叫停
    data = RUN_I2C_ReadByte(bus, 0); 
    
    RUN_I2C_Stop(bus);
    
    return data;
}