#include "RUN_header_file.h"

// ===============================================================================
// 核心配置表 (Lookup Table)
// -------------------------------------------------------------------------------
// 作用：将用户易读的枚举值 (如 UART1_TX_PA9_RX_PA10) 映射到底层硬件资源。
// 每一行对应一个串口引脚方案，包含：
// 1. USARTx 基地址
// 2. 串口时钟 (RCC)
// 3. 是否挂载在 APB2 总线上 (1=APB2, 0=APB1) -> 决定用哪个RCC函数
// 4. TX端口, TX引脚, TX时钟
// 5. RX端口, RX引脚, RX时钟
// 6. 是否需要端口重映射 (Remap)
// ===============================================================================
const uart_info_t uart_cfg[UART_PIN_MAX] = {
    // --- UART1 (挂载在 APB2 高速总线) ---
    // 方案0: 默认引脚 PA9/PA10
    {USART1, RCC_APB2Periph_USART1, 1, GPIOA, GPIO_Pin_9,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_10, RCC_APB2Periph_GPIOA, 0}, 
    // 方案1: 重映射引脚 PB6/PB7 (需要开启 AFIO 时钟并设置 Remap)
    {USART1, RCC_APB2Periph_USART1, 1, GPIOB, GPIO_Pin_6,  RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_7,  RCC_APB2Periph_GPIOB, GPIO_Remap_USART1}, 
    
    // --- UART2 (挂载在 APB1 低速总线) ---
    // 方案2: 默认引脚 PA2/PA3
    {USART2, RCC_APB1Periph_USART2, 0, GPIOA, GPIO_Pin_2,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_3,  RCC_APB2Periph_GPIOA, 0},
    // 方案3: 重映射引脚 PD5/PD6
    {USART2, RCC_APB1Periph_USART2, 0, GPIOD, GPIO_Pin_5,  RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_6,  RCC_APB2Periph_GPIOD, GPIO_Remap_USART2},
    
    // --- UART3 (挂载在 APB1 低速总线) ---
    // 方案4: 默认引脚 PB10/PB11
    {USART3, RCC_APB1Periph_USART3, 0, GPIOB, GPIO_Pin_10, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_11, RCC_APB2Periph_GPIOB, 0},
    // 方案5: 部分重映射 PC10/PC11
    {USART3, RCC_APB1Periph_USART3, 0, GPIOC, GPIO_Pin_10, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_11, RCC_APB2Periph_GPIOC, GPIO_PartialRemap_USART3},
    // 方案6: 完全重映射 PD8/PD9
    {USART3, RCC_APB1Periph_USART3, 0, GPIOD, GPIO_Pin_8,  RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_9,  RCC_APB2Periph_GPIOD, GPIO_FullRemap_USART3},
    
    // --- UART4 (仅 APB1) ---
    // 方案7: PC10/PC11
    {UART4,  RCC_APB1Periph_UART4,  0, GPIOC, GPIO_Pin_10, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_11, RCC_APB2Periph_GPIOC, 0},
    
    // --- UART5 (仅 APB1) ---
    // 方案8: PC12/PD2
    {UART5,  RCC_APB1Periph_UART5,  0, GPIOC, GPIO_Pin_12, RCC_APB2Periph_GPIOC, GPIOD, GPIO_Pin_2,  RCC_APB2Periph_GPIOD, 0}
};

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      获取UART对应的中断通道号 (内部函数)
// 参数说明      uart_base       USART1 ~ UART5 的基地址
// 返回参数      uint8_t         对应的 NVIC 中断通道号 (IRQn)
// 使用示例      Internal Use
// 备注信息      静态辅助函数，供 RUN_uart_init 调用
//-------------------------------------------------------------------------------------------------------------------
static uint8_t get_uart_irqn(USART_TypeDef* uart_base)
{
    if (uart_base == USART1) return USART1_IRQn;
    if (uart_base == USART2) return USART2_IRQn;
    if (uart_base == USART3) return USART3_IRQn;
    if (uart_base == UART4)  return UART4_IRQn;
    if (uart_base == UART5)  return UART5_IRQn;
    return 0; // 这里的 0 其实是不安全的，但理论上不会传入错误的基地址
}

// -----------------------------------------------------------
// 初始化函数
// -----------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      通用串口初始化配置
// 参数说明      uart_pin        选择串口及引脚方案 (枚举值，如 UART1_TX_PA9_RX_PA10)
// 参数说明      baud_rate       设置波特率 (如 115200)
// 参数说明      enable_it       是否开启接收中断 (1:开启, 0:关闭)
// 返回参数      void
// 使用示例      RUN_uart_init(UART1_TX_PA9_RX_PA10, 115200, 1); // 初始化串口1，开启中断
// 备注信息      函数自动处理时钟使能、GPIO复用配置、重映射及NVIC优先级配置(默认1,1)
//-------------------------------------------------------------------------------------------------------------------
void RUN_uart_init(UART_PIN_enum uart_pin, uint32_t baud_rate, uint8_t enable_it)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 参数安全检查：防止数组越界
    if (uart_pin >= UART_PIN_MAX) return;

    // =========================================================
    // 1. 时钟使能 (Clock Enable)
    // =========================================================
    // 使能 TX 和 RX 所在的 GPIO 端口时钟 (GPIO 都是挂在 APB2 上的)
    RCC_APB2PeriphClockCmd(uart_cfg[uart_pin].tx_rcc | uart_cfg[uart_pin].rx_rcc, ENABLE);

    // 使能 USART 外设时钟
    // 注意：USART1 在 APB2 (高速)，其他通常在 APB1 (低速)，需要区分处理
    if (uart_cfg[uart_pin].is_apb2) 
        RCC_APB2PeriphClockCmd(uart_cfg[uart_pin].uart_rcc, ENABLE);
    else                            
        RCC_APB1PeriphClockCmd(uart_cfg[uart_pin].uart_rcc, ENABLE);

    // =========================================================
    // 2. 引脚重映射 (Pin Remap)
    // =========================================================
    // 如果该方案需要重映射 (remap_val 不为 0)
    if (uart_cfg[uart_pin].remap_val != 0) {
        // 必须先开启 AFIO (辅助功能IO) 时钟，才能执行重映射
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        // 执行重映射配置
        GPIO_PinRemapConfig(uart_cfg[uart_pin].remap_val, ENABLE);
    }

    // =========================================================
    // 3. GPIO 配置 (GPIO Configuration)
    // =========================================================
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 设置引脚速度

    // --- 配置 TX (发送引脚) ---
    // 串口发送脚必须配置为 "复用推挽输出" (AF_PP)
    GPIO_InitStructure.GPIO_Pin = uart_cfg[uart_pin].tx_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(uart_cfg[uart_pin].tx_port, &GPIO_InitStructure);

    // --- 配置 RX (接收引脚) ---
    // 串口接收脚通常配置为 "浮空输入" (IN_FLOATING) 或 "上拉输入"
    GPIO_InitStructure.GPIO_Pin = uart_cfg[uart_pin].rx_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(uart_cfg[uart_pin].rx_port, &GPIO_InitStructure);

    // =========================================================
    // 4. USART 参数配置 (USART Configuration)
    // =========================================================
    USART_InitStructure.USART_BaudRate = baud_rate;                  // 波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;      // 字长 8位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;           // 停止位 1位
    USART_InitStructure.USART_Parity = USART_Parity_No;              // 无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无流控
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  // 同时开启收发模式
    USART_Init(uart_cfg[uart_pin].uart_base, &USART_InitStructure);

    // =========================================================
    // 5. 中断配置 (Interrupt Configuration)
    // =========================================================
    if (enable_it)
    {
        // 如果开启中断，配置 NVIC 控制器
        NVIC_InitStructure.NVIC_IRQChannel = get_uart_irqn(uart_cfg[uart_pin].uart_base);
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        // 子优先级
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 使能中断通道
        NVIC_Init(&NVIC_InitStructure);
        
        // 开启串口的 "接收寄存器非空" (RXNE) 中断
        // 当收到数据时，会触发中断服务函数
        USART_ITConfig(uart_cfg[uart_pin].uart_base, USART_IT_RXNE, ENABLE);
    }
    else
    {
        // 如果不开启，则禁用该中断
        USART_ITConfig(uart_cfg[uart_pin].uart_base, USART_IT_RXNE, DISABLE);
    }

    // =========================================================
    // 6. 使能串口 (Enable USART)
    // =========================================================
    // 所有的配置完成后，打开串口开关，开始工作
    USART_Cmd(uart_cfg[uart_pin].uart_base, ENABLE);
}

// -----------------------------------------------------------
// 发送与查询函数 (Blocking Mode / Polling)
// -----------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      串口发送一个字节 (阻塞式)
// 参数说明      uart_pin        选择串口 (枚举值)
// 参数说明      dat             要发送的字节数据
// 返回参数      void
// 使用示例      RUN_uart_putchar(UART1_TX_PA9_RX_PA10, 0xAA); // 发送十六进制0xAA
// 备注信息      函数会轮询等待 TXE (发送寄存器空) 标志位，属于阻塞发送
//-------------------------------------------------------------------------------------------------------------------
void RUN_uart_putchar(UART_PIN_enum uart_pin, uint8_t dat) {
    if (uart_pin >= UART_PIN_MAX) return;
    // 等待 TXE (发送寄存器为空) 标志位置 1
    while (USART_GetFlagStatus(uart_cfg[uart_pin].uart_base, USART_FLAG_TXE) == RESET);
    // 写入数据到 DR 寄存器
    USART_SendData(uart_cfg[uart_pin].uart_base, dat);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      串口发送字符串
// 参数说明      uart_pin        选择串口 (枚举值)
// 参数说明      str             字符串指针
// 返回参数      void
// 使用示例      RUN_uart_putstr(UART1_TX_PA9_RX_PA10, "Hello World\r\n");
// 备注信息      字符串必须以 '\0' 结尾
//-------------------------------------------------------------------------------------------------------------------
void RUN_uart_putstr(UART_PIN_enum uart_pin, char *str) {
    if (uart_pin >= UART_PIN_MAX) return;
    // 循环发送每一个字符，直到遇到字符串结束符 '\0'
    while (*str) RUN_uart_putchar(uart_pin, *str++);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      串口发送指定长度的字节数组
// 参数说明      uart_pin        选择串口 (枚举值)
// 参数说明      buff            数据缓冲区指针
// 参数说明      len             要发送的数据长度
// 返回参数      void
// 使用示例      RUN_uart_putbuff(UART1_TX_PA9_RX_PA10, buffer, 10); // 发送数组前10个字节
// 备注信息      适用于发送二进制数据或非文本数据
//-------------------------------------------------------------------------------------------------------------------
void RUN_uart_putbuff(UART_PIN_enum uart_pin, uint8_t *buff, uint32_t len) {
    if (uart_pin >= UART_PIN_MAX) return;
    for(uint32_t i = 0; i < len; i++) RUN_uart_putchar(uart_pin, buff[i]);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      串口接收一个字节 (阻塞式)
// 参数说明      uart_pin        选择串口 (枚举值)
// 返回参数      uint8_t         接收到的数据
// 使用示例      uint8_t data = RUN_uart_getchar(UART1_TX_PA9_RX_PA10);
// 备注信息      【注意】如果串口没有收到数据，程序会死等在此处，建议先使用 RUN_uart_query 查询
//-------------------------------------------------------------------------------------------------------------------
uint8_t RUN_uart_getchar(UART_PIN_enum uart_pin) {
    if (uart_pin >= UART_PIN_MAX) return 0;
    // 等待 RXNE (接收寄存器非空) 标志位置 1
    while (USART_GetFlagStatus(uart_cfg[uart_pin].uart_base, USART_FLAG_RXNE) == RESET);
    // 读取数据
    return (uint8_t)USART_ReceiveData(uart_cfg[uart_pin].uart_base);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      查询串口是否有数据 (非阻塞)
// 参数说明      uart_pin        选择串口 (枚举值)
// 返回参数      uint8_t         1: 有数据待读取, 0: 无数据
// 使用示例      if(RUN_uart_query(UART1_TX_PA9_RX_PA10)) { ... }
// 备注信息      仅检查 RXNE 标志位，不会清除标志位，也不会读取数据
//-------------------------------------------------------------------------------------------------------------------
uint8_t RUN_uart_query(UART_PIN_enum uart_pin) {
    if (uart_pin >= UART_PIN_MAX) return 0;
    // 仅仅检查标志位，不等待
    return (USART_GetFlagStatus(uart_cfg[uart_pin].uart_base, USART_FLAG_RXNE) != RESET);
}

// -----------------------------------------------------------
// Printf 重定向
// -----------------------------------------------------------
// 这里的代码是为了让 C 语言标准库的 printf() 函数能输出到串口
// 默认强制映射到枚举值为 0 的串口 (通常是 UART1_TX_PA9_RX_PA10)
#if 1
#pragma import(__use_no_semihosting) // 告诉编译器不使用半主机模式
struct __FILE { int handle; };
FILE __stdout;
void _sys_exit(int x) { x = x; } // 定义 _sys_exit 以避免使用半主机模式时的错误

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      Printf 重定向底层函数 (fputc)
// 参数说明      ch              待发送的字符
// 参数说明      f               文件指针 (标准库内部使用)
// 返回参数      int             返回发送的字符
// 使用示例      printf("Value: %d", 123);
// 备注信息      强制映射到枚举值为 0 的串口，需在 Keil 中勾选 Use MicroLIB
//-------------------------------------------------------------------------------------------------------------------
// 重写 fputc，printf 底层就是调用这个函数发送字符的
int fputc(int ch, FILE *f) {
    // 强制使用列表中的第 0 个串口发送 (通常是 PA9/PA10)
    RUN_uart_putchar((UART_PIN_enum)0, (uint8_t)ch);
    return ch;
}
#endif