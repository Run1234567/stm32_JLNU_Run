#include "RUN_header_file.h"

// ===============================================================================
// 全局变量定义区域
// -------------------------------------------------------------------------------
// 用于存储各串口接收到的数据包及接收完成标志位
// 协议格式：@ + 数据 + \r\n
// ===============================================================================

// 串口1 变量
char UART1_RxPacket[MAX_RX_LEN]; // 接收缓冲区
uint8_t UART1_RxFlag = 0;        // 接收完成标志 (1:完成, 0:未完成)

// 串口2 变量
char UART2_RxPacket[MAX_RX_LEN];
uint8_t UART2_RxFlag = 0;

// 串口3 变量
char UART3_RxPacket[MAX_RX_LEN];
uint8_t UART3_RxFlag = 0;

// 串口4 变量
char UART4_RxPacket[MAX_RX_LEN];
uint8_t UART4_RxFlag = 0;

// 串口5 变量
char UART5_RxPacket[MAX_RX_LEN];
uint8_t UART5_RxFlag = 0;

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      串口1 数据包解析函数 (状态机)
// 参数说明      data            接收到的单个字节数据
// 返回参数      void
// 使用示例      uart1_rx_interrupt(received_byte); // 在中断中调用
// 备注信息      协议格式：包头'@'，包尾'\r\n'。解析完成后置位 UART1_RxFlag
//-------------------------------------------------------------------------------------------------------------------
void uart1_rx_interrupt(uint8_t data)
{
    static uint8_t RxState = 0;
    static uint8_t pRxIndex = 0;

    if (RxState == 0) // 状态0：寻找包头
    {
        if (data == '@' && UART1_RxFlag == 0)
        {
            RxState = 1;
            pRxIndex = 0;
            memset(UART1_RxPacket, 0, MAX_RX_LEN); // 清空缓存
        }
    }
    else if (RxState == 1) // 状态1：接收数据主体
    {
        if (data == '\r')
        {
            RxState = 2;
        }
        else
        {
            if (pRxIndex < MAX_RX_LEN - 1) // 防止缓冲区溢出
            {
                UART1_RxPacket[pRxIndex++] = data;
            }
        }
    }
    else if (RxState == 2) // 状态2：寻找包尾
    {
        if (data == '\n')
        {
            RxState = 0;
            UART1_RxPacket[pRxIndex] = '\0'; // 添加字符串结束符
            UART1_RxFlag = 1;                // 标记接收完成
        }
        else
        {
            RxState = 0; // 格式错误，复位状态机
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      串口2 数据包解析函数 (状态机)
// 参数说明      data            接收到的单个字节数据
// 返回参数      void
// 使用示例      uart2_rx_interrupt(received_byte);
// 备注信息      协议格式：包头'@'，包尾'\r\n'。解析完成后置位 UART2_RxFlag
//-------------------------------------------------------------------------------------------------------------------
void uart2_rx_interrupt(uint8_t data)
{
    static uint8_t RxState = 0;
    static uint8_t pRxIndex = 0;

    if (RxState == 0)
    {
        if (data == '@' && UART2_RxFlag == 0)
        {
            RxState = 1;
            pRxIndex = 0;
            memset(UART2_RxPacket, 0, MAX_RX_LEN);
        }
    }
    else if (RxState == 1)
    {
        if (data == '\r')
        {
            RxState = 2;
        }
        else
        {
            if (pRxIndex < MAX_RX_LEN - 1)
            {
                UART2_RxPacket[pRxIndex++] = data;
            }
        }
    }
    else if (RxState == 2)
    {
        if (data == '\n')
        {
            RxState = 0;
            UART2_RxPacket[pRxIndex] = '\0';
            UART2_RxFlag = 1;
        }
        else
        {
            RxState = 0;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      串口3 数据包解析函数 (状态机)
// 参数说明      data            接收到的单个字节数据
// 返回参数      void
// 使用示例      uart3_rx_interrupt(received_byte);
// 备注信息      协议格式：包头'@'，包尾'\r\n'。解析完成后置位 UART3_RxFlag
//-------------------------------------------------------------------------------------------------------------------
void uart3_rx_interrupt(uint8_t data)
{
    static uint8_t RxState = 0;
    static uint8_t pRxIndex = 0;

    if (RxState == 0)
    {
        if (data == '@' && UART3_RxFlag == 0)
        {
            RxState = 1;
            pRxIndex = 0;
            memset(UART3_RxPacket, 0, MAX_RX_LEN);
        }
    }
    else if (RxState == 1)
    {
        if (data == '\r')
        {
            RxState = 2;
        }
        else
        {
            if (pRxIndex < MAX_RX_LEN - 1)
            {
                UART3_RxPacket[pRxIndex++] = data;
            }
        }
    }
    else if (RxState == 2)
    {
        if (data == '\n')
        {
            RxState = 0;
            UART3_RxPacket[pRxIndex] = '\0';
            UART3_RxFlag = 1;
        }
        else
        {
            RxState = 0;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      串口4 数据包解析函数 (状态机)
// 参数说明      data            接收到的单个字节数据
// 返回参数      void
// 使用示例      uart4_rx_interrupt(received_byte);
// 备注信息      协议格式：包头'@'，包尾'\r\n'。解析完成后置位 UART4_RxFlag
//-------------------------------------------------------------------------------------------------------------------
void uart4_rx_interrupt(uint8_t data)
{
    static uint8_t RxState = 0;
    static uint8_t pRxIndex = 0;

    if (RxState == 0)
    {
        if (data == '@' && UART4_RxFlag == 0)
        {
            RxState = 1;
            pRxIndex = 0;
            memset(UART4_RxPacket, 0, MAX_RX_LEN);
        }
    }
    else if (RxState == 1)
    {
        if (data == '\r')
        {
            RxState = 2;
        }
        else
        {
            if (pRxIndex < MAX_RX_LEN - 1)
            {
                UART4_RxPacket[pRxIndex++] = data;
            }
        }
    }
    else if (RxState == 2)
    {
        if (data == '\n')
        {
            RxState = 0;
            UART4_RxPacket[pRxIndex] = '\0';
            UART4_RxFlag = 1;
        }
        else
        {
            RxState = 0;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      串口5 数据包解析函数 (状态机)
// 参数说明      data            接收到的单个字节数据
// 返回参数      void
// 使用示例      uart5_rx_interrupt(received_byte);
// 备注信息      协议格式：包头'@'，包尾'\r\n'。解析完成后置位 UART5_RxFlag
//-------------------------------------------------------------------------------------------------------------------
void uart5_rx_interrupt(uint8_t data)
{
    static uint8_t RxState = 0;
    static uint8_t pRxIndex = 0;

    if (RxState == 0)
    {
        if (data == '@' && UART5_RxFlag == 0)
        {
            RxState = 1;
            pRxIndex = 0;
            memset(UART5_RxPacket, 0, MAX_RX_LEN);
        }
    }
    else if (RxState == 1)
    {
        if (data == '\r')
        {
            RxState = 2;
        }
        else
        {
            if (pRxIndex < MAX_RX_LEN - 1)
            {
                UART5_RxPacket[pRxIndex++] = data;
            }
        }
    }
    else if (RxState == 2)
    {
        if (data == '\n')
        {
            RxState = 0;
            UART5_RxPacket[pRxIndex] = '\0';
            UART5_RxFlag = 1;
        }
        else
        {
            RxState = 0;
        }
    }
}

// ==========================================================
// 弱定义 (Weak) 区域
// 这些是“备胎”函数。如果你在 main.c 里没写，编译器就用这些空的。
// 如果你在 main.c 里写了同名函数，这些就会被覆盖，不再起作用。
// ==========================================================

//__weak void uart1_rx_interrupt(uint8_t data) {}
//__weak void uart2_rx_interrupt(uint8_t data) {}
//__weak void uart3_rx_interrupt(uint8_t data) {}
//__weak void uart4_rx_interrupt(uint8_t data) {}
//__weak void uart5_rx_interrupt(uint8_t data) {}

// ==========================================================
// 中断服务函数 (IRQ)
// 硬件自动调用这里，这里再自动调用你的函数
// ==========================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      USART1 全局中断服务函数
// 参数说明      void
// 返回参数      void
// 使用示例      (Hardware Triggered)
// 备注信息      检测 RXNE 标志位，调用解析函数并清除标志位
//-------------------------------------------------------------------------------------------------------------------
void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        // 调用用户的函数（如果用户定义了的话）
        uart1_rx_interrupt((uint8_t)USART_ReceiveData(USART1));
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      USART2 全局中断服务函数
// 参数说明      void
// 返回参数      void
// 使用示例      (Hardware Triggered)
// 备注信息      检测 RXNE 标志位，调用解析函数并清除标志位
//-------------------------------------------------------------------------------------------------------------------
void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        uart2_rx_interrupt((uint8_t)USART_ReceiveData(USART2));
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      USART3 全局中断服务函数
// 参数说明      void
// 返回参数      void
// 使用示例      (Hardware Triggered)
// 备注信息      检测 RXNE 标志位，调用解析函数并清除标志位
//-------------------------------------------------------------------------------------------------------------------
void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        uart3_rx_interrupt((uint8_t)USART_ReceiveData(USART3));
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      UART4 全局中断服务函数
// 参数说明      void
// 返回参数      void
// 使用示例      (Hardware Triggered)
// 备注信息      检测 RXNE 标志位，调用解析函数并清除标志位
//-------------------------------------------------------------------------------------------------------------------
void UART4_IRQHandler(void)
{
    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
    {
        uart4_rx_interrupt((uint8_t)USART_ReceiveData(UART4));
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      UART5 全局中断服务函数
// 参数说明      void
// 返回参数      void
// 使用示例      (Hardware Triggered)
// 备注信息      检测 RXNE 标志位，调用解析函数并清除标志位
//-------------------------------------------------------------------------------------------------------------------
void UART5_IRQHandler(void)
{
    if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
    {
        uart5_rx_interrupt((uint8_t)USART_ReceiveData(UART5));
        USART_ClearITPendingBit(UART5, USART_IT_RXNE);
    }
}

// ==========================================================
// 中断回调接口 (Weak)
// 作用：提供给用户在 main.c 中重写，实现定时器中断逻辑
// ==========================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      定时器中断回调函数 (弱定义)
// 参数说明      void
// 返回参数      void
// 使用示例      在 main.c 中重写 void TIM1_Callback(void) { ... }
// 备注信息      若用户未重写，则默认调用此空函数
//-------------------------------------------------------------------------------------------------------------------
__weak void TIM1_Callback(void) {}
__weak void TIM8_Callback(void) {}
__weak void TIM2_Callback(void) {}
__weak void TIM3_Callback(void) {}
__weak void TIM4_Callback(void) {}
__weak void TIM5_Callback(void) {}
__weak void TIM6_Callback(void) {}
__weak void TIM7_Callback(void) {}

// ==========================================================
// 硬件中断服务函数 (IRQ Handlers)
// ==========================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      TIM1 更新中断服务函数 (高级定时器)
// 参数说明      void
// 返回参数      void
// 使用示例      (Hardware Triggered)
// 备注信息      清除 Update 标志位并调用 TIM1_Callback
//-------------------------------------------------------------------------------------------------------------------
void TIM1_UP_IRQHandler(void) {
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
        TIM1_Callback();
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      TIM8 更新中断服务函数 (高级定时器)
// 参数说明      void
// 返回参数      void
// 使用示例      (Hardware Triggered)
// 备注信息      注意 TIM8 的中断名称通常为 TIM8_UP
//-------------------------------------------------------------------------------------------------------------------
void TIM8_UP_IRQHandler(void) {
    if (TIM_GetITStatus(TIM8, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
        TIM8_Callback();
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      TIM2 全局中断服务函数 (通用定时器)
// 参数说明      void
// 返回参数      void
// 使用示例      (Hardware Triggered)
// 备注信息      清除 Update 标志位并调用 TIM2_Callback
//-------------------------------------------------------------------------------------------------------------------
void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        TIM2_Callback();
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      TIM3 全局中断服务函数 (通用定时器)
// 参数说明      void
// 返回参数      void
// 使用示例      (Hardware Triggered)
// 备注信息      清除 Update 标志位并调用 TIM3_Callback
//-------------------------------------------------------------------------------------------------------------------
void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        TIM3_Callback();
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      TIM4 全局中断服务函数 (通用定时器)
// 参数说明      void
// 返回参数      void
// 使用示例      (Hardware Triggered)
// 备注信息      清除 Update 标志位并调用 TIM4_Callback
//-------------------------------------------------------------------------------------------------------------------
void TIM4_IRQHandler(void) {
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
        TIM4_Callback();
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      TIM5 全局中断服务函数 (通用定时器)
// 参数说明      void
// 返回参数      void
// 使用示例      (Hardware Triggered)
// 备注信息      清除 Update 标志位并调用 TIM5_Callback
//-------------------------------------------------------------------------------------------------------------------
void TIM5_IRQHandler(void) {
    if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
        TIM5_Callback();
    }
}

// --- TIM6 (兼容性处理) ---

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      TIM6 中断处理核心逻辑
// 参数说明      void
// 返回参数      void
// 使用示例      Internal Use
// 备注信息      实际执行中断标志清除和回调调用的函数
//-------------------------------------------------------------------------------------------------------------------
void RUN_TIM6_Handler_Core(void) {
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
        TIM6_Callback();
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      TIM6 全局中断入口 (标准名称)
// 参数说明      void
// 返回参数      void
// 备注信息      调用 Core 函数处理
//-------------------------------------------------------------------------------------------------------------------
void TIM6_IRQHandler(void) { RUN_TIM6_Handler_Core(); }

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      TIM6 全局中断入口 (带DAC名称)
// 参数说明      void
// 返回参数      void
// 备注信息      兼容不同启动文件，防止因名称不匹配进不去中断
//-------------------------------------------------------------------------------------------------------------------
void TIM6_DAC_IRQHandler(void) { RUN_TIM6_Handler_Core(); }

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      TIM7 全局中断服务函数 (基本定时器)
// 参数说明      void
// 返回参数      void
// 使用示例      (Hardware Triggered)
// 备注信息      清除 Update 标志位并调用 TIM7_Callback
//-------------------------------------------------------------------------------------------------------------------
void TIM7_IRQHandler(void) {
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
        TIM7_Callback();
    }
}