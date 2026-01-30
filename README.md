🎯 **文档持续更新中** | 🚀 **更多内容即将到来**

# **RUN库：STM32 的“极速开发”引擎**

**RUN库** 是专为 **STM32F103** 设计的轻量级驱动框架。它砍掉了标准库繁琐的配置流程，让你像玩 Arduino 一样轻松驾驭 STM32。

## **💥 核心亮点**

1. **全自动配置**：彻底告别手动开启 `RCC` 时钟、计算 `AFIO` 重映射、查分频系数。**一行代码，库全帮你做了。**
2. **人话编程**：用 `C13` 代替 `GPIO_Pin_13`，用 `RUN_TIM6` 代替定时器结构体。
3. **回调机制**：中断函数直接在 `main.c` 绑定，无需去 `it.c` 文件里翻找。

## **⚡️ 效率对比 (点灯为例)**


| 标准库                     | RUN库                  |
| -------------------------- | ---------------------- |
| 15行代码                   | 2行代码                |
| 需要配置时钟、GPIO、中断等 | 自动完成，一行代码搞定 |

* **❌ 传统标准库 (5步)**： 开时钟 -> 定义结构体 -> 配引脚 -> 配模式 -> 配速度 -> 初始化。
* **✅ RUN库 (1步)**：
  **C**

  ```
  RUN_gpio_init(C13, GPO, 0); // PC13推挽输出，默认低电平
  ```

## **🛠 功能全家桶**

* **基础**：GPIO、精准延时、自动PWM、串口 (带printf)。
* **通信**：SPI、I2C、单总线 (OneWire)。
* **外设**：MPU6050、DS18B20、电机驱动、Flash读写等，**拿来即用**。

## 🤝 交流与反馈

有问题？想吐槽？欢迎来这里找我：

* 📧 **个人邮箱**: `2046138442@qq.com`

## **🚀 一句话总结**

如果你想拥有 STM32 的性能，又不想被底层寄存器折磨，**用 RUN库 就对了**。

# 目录 (Table of Contents)

- [STM32 GPIO 驱动模块使用说明](#stm32-gpio-驱动模块使用说明)
- [STM32 SysTick 延时模块使用说明](#stm32-systick-延时模块使用说明)
- [STM32 UART 串口驱动模块使用说明](#stm32-uart-串口驱动模块使用说明)
- [STM32 定时器中断驱动模块使用说明](#stm32-定时器中断驱动模块使用说明)
- [STM32 PWM 驱动模块使用说明](#stm32-pwm-驱动模块使用说明)
- [STM32 EXTI 外部中断驱动模块使用说明](#stm32-exti-外部中断驱动模块使用说明)
- [STM32 ADC & DAC 模拟外设驱动说明](#stm32-adc--dac-模拟外设驱动说明)
- [STM32 硬件 SPI 驱动模块使用说明](#stm32-硬件-spi-驱动模块使用说明)
- [STM32 软件模拟 I2C 驱动模块使用说明](#stm32-软件模拟-i2c-驱动模块使用说明)
- [STM32 单总线 (OneWire) 驱动模块使用说明](#stm32-单总线-onewire-驱动模块使用说明)
- [STM32 内部 Flash 模拟 EEPROM 驱动模块使用说明](#stm32-内部-flash-模拟-eeprom-驱动模块使用说明)
- [STM32 DMA 数据搬运驱动模块使用说明](#stm32-dma-数据搬运驱动模块使用说明)
- [STM32 AT24C02 (EEPROM) 驱动模块使用说明](#stm32-at24c02-eeprom-驱动模块使用说明)
- [STM32 DS18B20 温度传感器驱动模块使用说明](#stm32-ds18b20-温度传感器驱动模块使用说明)
- [STM32 直流有刷电机 (小车底盘) 驱动模块使用说明](#stm32-直流有刷电机驱动模块使用说明)
- [STM32 步进电机驱动模块使用说明](#stm32-步进电机驱动模块使用说明)
- [STM32 MPU6050 6轴姿态传感器驱动说明](#stm32-mpu6050-6轴姿态传感器驱动说明)
- [STM32 W25Q64 Flash 存储驱动模块使用说明](#stm32-w25q64-flash-存储驱动模块使用说明)
- [STM32 - 姿态解算模块说明文档](#stm32---姿态解算模块使用说明)
- [STM32 PID 控制算法模块使用说明](#stm32-pid-控制算法模块使用说明)
- [STM32 字符串解析模块说明文档](#stm32-字符串解析模块说明文档)

# STM32 GPIO 驱动模块使用说明

本模块对 STM32 标准外设库进行了二次封装，提供了一套简单、统一的 GPIO 控制接口。使用者无需关心繁琐的时钟配置（RCC）和结构体初始化，通过简单的枚举索引即可操作引脚。

## 1. 核心特性

* **自动时钟管理**：初始化引脚时，驱动自动开启对应的端口时钟（RCC）。
* **极简索引**：使用 `A0`、`C13` 等直观的枚举名称，替代标准库冗长的宏定义。
* **全系列支持**：默认支持 GPIOA \~ GPIOG (Pin 0\~15) 所有引脚。

## 2. 快速上手示例

假设场景：

1. **LED** 接在 **PC13**，低电平点亮。
2. **按键** 接在 **PA0**，按下接地（需要开启内部上拉）。

**C**

```
#include "RUN_Gpio.h"

int main(void)
{
// --- 1. 初始化 ---

// 初始化 LED (PC13): 推挽输出 (GPO), 默认输出高电平 (1-灭)
RUN_gpio_init(C13, GPO, 1);

// 初始化 按键 (PA0): 上拉输入 (GPI_PU), 默认电平填 0 即可 (输入模式无效)
RUN_gpio_init(A0, GPI_PU, 0);

while (1)
{
// --- 2. 读取输入 ---
// 如果按键被按下 (读取到低电平 0)
if (RUN_gpio_get(A0) == 0)
{
    // 简单的延时消抖
    for(volatile int i=0; i<5000; i++);

    if (RUN_gpio_get(A0) == 0)
    {
        // --- 3. 翻转输出 ---
        // 翻转 PC13 电平，让 LED 闪烁
        RUN_gpio_toggle(C13);

        // 等待按键松开
        while(RUN_gpio_get(A0) == 0);
    }
}
}
}
```

---

## 3. API 接口详解

### 3.1 初始化引脚 `RUN_gpio_init`

**C**

```
void RUN_gpio_init(RUN_GPIO_enum pin, RUN_GPIO_Mode mode, uint8_t default_level);
```


| **参数**           | **说明**                                                    |
| ------------------ | ----------------------------------------------------------- |
| **pin**            | 引脚编号。格式为`Port`+`Pin`，例如：`A0`,`B12`,`C13`。      |
| **mode**           | 引脚工作模式（见下文参数速查表）。                          |
| **default\_level** | **仅输出模式有效**。初始化后的默认电平：`0`(低) 或`1`(高)。 |

> **注意**：调用此函数会自动开启该引脚所属端口的 RCC 时钟，无需手动配置。

### 3.2 设置输出电平 `RUN_gpio_set`

**C**

```
void RUN_gpio_set(RUN_GPIO_enum pin, uint8_t level);
```

* **功能**：将指定引脚置为高电平或低电平。
* **参数**：
* `pin`: 如 `A5`, `D2`。
* `level`: `1` (高电平/Set), `0` (低电平/Reset)。

### 3.3 翻转输出电平 `RUN_gpio_toggle`

**C**

```
void RUN_gpio_toggle(RUN_GPIO_enum pin);
```

* **功能**：读取当前输出状态并取反（高变低，低变高）。常用于 LED 闪烁控制。

### 3.4 读取输入电平 `RUN_gpio_get`

**C**

```
uint8_t RUN_gpio_get(RUN_GPIO_enum pin);
```

* **功能**：读取引脚的电平状态。
* **返回**：`1` (高电平), `0` (低电平)。

---

## 4. 参数速查表

### 4.1 引脚枚举 (`pin`)

直接使用 `端口号` + `引脚号` 的组合：


| **端口**  | **可用枚举范围** | **示例**                      |
| --------- | ---------------- | ----------------------------- |
| **GPIOA** | `A0`\~`A15`      | `RUN_gpio_set(A8, 1);`        |
| **GPIOB** | `B0`\~`B15`      | `RUN_gpio_toggle(B1);`        |
| **GPIOC** | `C0`\~`C15`      | `RUN_gpio_init(C13, GPO, 1);` |
| ...       | ...              | ...                           |
| **GPIOG** | `G0`\~`G15`      |                               |

### 4.2 模式枚举 (`mode`)


| **枚举值**   | **对应标准库模式**      | **说明**     | **适用场景**           |
| ------------ | ----------------------- | ------------ | ---------------------- |
| **`GPO`**    | `GPIO_Mode_Out_PP`      | **推挽输出** | LED、继电器、蜂鸣器    |
| **`GPO_OD`** | `GPIO_Mode_Out_OD`      | **开漏输出** | I2C 数据线、电平转换   |
| **`GPI_PU`** | `GPIO_Mode_IPU`         | **上拉输入** | 外部按键（对地触发）   |
| **`GPI_PD`** | `GPIO_Mode_IPD`         | **下拉输入** | 外部信号（高电平触发） |
| **`GPI`**    | `GPIO_Mode_IN_FLOATING` | **浮空输入** | 标准通信协议接收端     |
| **`AIN`**    | `GPIO_Mode_AIN`         | **模拟输入** | ADC 采集               |

# STM32 SysTick 延时模块使用说明

该模块利用 Cortex-M 内核自带的 **SysTick (系统滴答定时器)** 实现高精度的阻塞式延时。相比普通 `for` 循环延时，它不依赖编译器优化等级，时间更精准。

## 1. 核心特性

* **高精度**：利用硬件计数器实现微秒 (us) 级控制。
* **长延时支持**：毫秒 (ms) 级函数采用循环调用机制，突破了 SysTick 24位寄存器的溢出限制，支持秒级甚至更长的延时。
* **简单易用**：仅需初始化一次，即可在任意地方调用。

## 2. 快速上手

### 2.1 初始化 (必须)

在使用任何延时函数前，**必须**先在 `main` 函数最开始调用初始化函数。

**C**

```
// 对于 STM32F103 (标准库默认主频 72MHz)
RUN_delay_init(72);
```

### 2.2 使用示例

**C**

```
#include "RUN_Delay.h"
#include "RUN_Gpio.h" // 假设配合之前的 GPIO 模块使用

int main(void)
{
// 1. 初始化延时模块 (主频 72MHz)
RUN_delay_init(72);

// 2. 初始化 GPIO
RUN_gpio_init(C13, GPO, 1);

while (1)
{
RUN_gpio_toggle(C13); // 翻转 LED

// 毫秒延时：500ms
RUN_delay_ms(500);

// 微秒延时：50us (模拟短时间时序)
RUN_delay_us(50); 
}
}
```

---

## 3. API 接口详解

### 3.1 初始化 `RUN_delay_init`

配置 SysTick 时钟源（HCLK/8）并计算时钟节拍。

**C**

```
void RUN_delay_init(uint8_t sysclk_mhz);
```

* **sysclk\_mhz**: 系统主频 (MHz)。
* STM32F103 通常填 `72`。
* STM32F103 (内部时钟) 可能填 `64` 或 `8`。

### 3.2 微秒延时 `RUN_delay_us`

阻塞式等待指定微秒数。

**C**

```
void RUN_delay_us(uint32_t nus);
```

* **nus**: 延时时间 (us)。
* **限制警告**：由于 SysTick 寄存器为 24 位，单次调用最大延时约 **1.86 秒** (1,864,135 us)。超过此值会导致溢出和延时错误。

### 3.3 毫秒延时 `RUN_delay_ms`

阻塞式等待指定毫秒数。

**C**

```
void RUN_delay_ms(uint16_t nms);
```

* **nms**: 延时时间 (ms)。
* **特点**：函数内部通过循环调用 `RUN_delay_us(1000)` 实现。因此它**不受** 24 位寄存器溢出限制，支持超长延时 (如 10 秒、1 分钟等)。

---

## 4. 注意事项

1. **SysTick 冲突**：如果你使用了 FreeRTOS 或 uCOS 等实时操作系统，它们通常也会占用 SysTick 中断。在此类系统中，**不能**直接使用此模块，否则会导致系统时基混乱或死机。
2. **最大微秒限制**：虽然 `RUN_delay_us` 参数是 `uint32_t`，但不要传入超过 `1864135` 的数值。如果需要长延时，请直接使用 `RUN_delay_ms`。

# STM32 UART 串口驱动模块使用说明

本模块通过**枚举索引**的方式封装了 STM32 的串口功能。最大的特点是**自动化配置**：只需选择一个枚举值（如 `UART1_TX_PB6_RX_PB7`），驱动会自动处理繁琐的时钟开启（APB1/APB2）、GPIO 模式配置以及复杂的**引脚重映射 (Remap)** 逻辑。

## 1. 核心特性

* **全自动重映射**：自动识别是否需要开启 `AFIO` 时钟及配置 `GPIO_Remap`，用户无需查表。
* **方案化配置**：通过枚举直接选择引脚组合（如方案1：PA9/10，方案2：PB6/PB7），防止引脚配置错误。
* **Printf 支持**：内置 `fputc` 重定向，支持标准 C 库的 `printf` 输出（默认映射到第一个串口配置）。
* **灵活的中断控制**：初始化时可选择是否开启接收中断（RXNE）。

## 2. 快速上手

# 2.1 基础发送与接收

**C**

```
#include "RUN_UART.h"
#include <stdio.h> // 使用 printf 需要包含

int main(void)
{
// 1. 初始化串口 1 (使用 PA9/PA10 引脚)
// 波特率: 115200, 中断: 0 (关闭接收中断)
RUN_uart_init(UART1_TX_PA9_RX_PA10, 115200, 0);

// 2. 发送字符串
RUN_uart_putstr(UART1_TX_PA9_RX_PA10, "System Start...\r\n");

// 3. 使用 printf (默认重定向到枚举列表的第0个串口，即 UART1_PA9_PA10)
printf("Init Success! Baudrate: %d\r\n", 115200);

while (1)
{
// 查询是否收到数据 (非阻塞)
if (RUN_uart_query(UART1_TX_PA9_RX_PA10))
{
    // 读取一个字节
    uint8_t dat = RUN_uart_getchar(UART1_TX_PA9_RX_PA10);

    // 将收到的数据回传 (Echo)
    RUN_uart_putchar(UART1_TX_PA9_RX_PA10, dat);
}
}
}
```

### 2.2 使用重映射引脚 (如 UART1 映射到 PB6/PB7)

无需手动配置 AFIO，直接调用即可：

**C**

```
// 驱动会自动开启 AFIO 时钟并执行 GPIO_Remap_USART1
RUN_uart_init(UART1_TX_PB6_RX_PB7, 9600, 1); 
```

---

## 3. API 接口详解

### 3.1 初始化 `RUN_uart_init`

**C**

```
void RUN_uart_init(UART_PIN_enum uart_pin, uint32_t baud_rate, uint8_t enable_it);
```

* **uart\_pin**: 硬件方案枚举（见下表）。
* **baud\_rate**: 波特率（如 9600, 115200）。
* **enable\_it**:
* `0`: 轮询模式，禁用中断。
* `1`: 开启接收中断 (`USART_IT_RXNE`) 并配置 NVIC。**注意**：开启后需在 `stm32f10x_it.c` 或 `main.c` 中自行编写对应的中断服务函数（如 `USART1_IRQHandler`）。

### 3.2 数据发送


| **函数**                                | **说明**                                                                                            |
| --------------------------------------- | --------------------------------------------------------------------------------------------------- |
| `void RUN_uart_putchar(enum, uint8_t)`  | 发送单个字节（阻塞等待发送完成）。                                                                  |
| `void RUN_uart_putstr(enum, char*)`     | 发送字符串（以`\0`结尾）。                                                                          |
| `void RUN_uart_putbuff(enum, u8*, len)` | 发送指定长度的二进制数组。                                                                          |
| `printf(...)`                           | **仅限调试用**。底层调用`RUN_uart_putchar`，强制输出到枚举值为 0 的串口（通常是`UART1_PA9_PA10`）。 |

### 3.3 数据接收


| **函数**                         | **说明**                                                                     |
| -------------------------------- | ---------------------------------------------------------------------------- |
| `uint8_t RUN_uart_query(enum)`   | **查询状态**。返回`1`表示有数据可读，`0`表示无数据。非阻塞，不清除标志位。   |
| `uint8_t RUN_uart_getchar(enum)` | **读取字节**。阻塞式读取。如果未查询直接调用且无数据，程序会卡死在这里等待。 |

---

## 4. 硬件方案速查表 (Enum List)

该模块已预定义以下硬件组合，直接使用对应的 **枚举名称** 即可：


| **枚举名称 (UART\_PIN\_enum)** | **串口号** | **TX 引脚** | **RX 引脚** | **备注 (自动处理)**        |
| ------------------------------ | ---------- | ----------- | ----------- | -------------------------- |
| **`UART1_TX_PA9_RX_PA10`**     | USART1     | PA9         | PA10        | 默认引脚 (Printf 默认通道) |
| **`UART1_TX_PB6_RX_PB7`**      | USART1     | PB6         | PB7         | **重映射**(AFIO)           |
| **`UART2_TX_PA2_RX_PA3`**      | USART2     | PA2         | PA3         | 默认引脚                   |
| **`UART2_TX_PD5_RX_PD6`**      | USART2     | PD5         | PD6         | **重映射**                 |
| **`UART3_TX_PB10_RX_PB11`**    | USART3     | PB10        | PB11        | 默认引脚                   |
| **`UART3_TX_PC10_RX_PC11`**    | USART3     | PC10        | PC11        | **部分重映射**             |
| **`UART3_TX_PD8_RX_PD9`**      | USART3     | PD8         | PD9         | **完全重映射**             |
| **`UART4_TX_PC10_RX_PC11`**    | UART4      | PC10        | PC11        | 仅部分型号支持             |
| **`UART5_TX_PC12_RX_PD2`**     | UART5      | PC12        | PD2         | 仅部分型号支持             |

*[数据来源: RUN\_UART.c / uart\_cfg 数组]*

---

## 5. 注意事项

1. **Printf 的使用限制**：

* 代码中 `fputc` 强制写死了 `RUN_uart_putchar((UART_PIN_enum)0, ...)`。这意味着 `printf`**只能** 输出到 `UART_PIN_enum` 定义的第一个串口（即 `UART1_TX_PA9_RX_PA10`）。
* 如果你使用 Keil MDK，**必须**在工程选项 "Target" 标签页中勾选 **"Use MicroLIB"**，否则程序会卡死在启动代码中。

2. **中断服务函数**：

* 驱动仅负责开启中断开关（NVIC 和 USART\_IT\_RXNE）。
* **你必须**在项目中手动编写 `void USART1_IRQHandler(void)` 等函数来处理接收逻辑。

3. **阻塞风险**：

* `RUN_uart_getchar` 是**死等**模式（`while` 循环等待标志位）。在主循环中使用时，务必先用 `RUN_uart_query` 判断是否有数据，防止程序卡死。
  er(void)` 函数，以处理 UART 接收中断并实现数据的正确接收与处理。

# STM32 定时器中断驱动模块使用说明

本模块封装了 STM32 的 8 个定时器（TIM1 \~ TIM8），主要用于**周期性定时中断**任务。驱动会自动处理 APB1/APB2 总线频率差异和 NVIC 中断配置，用户只需指定“多少毫秒触发一次”即可。

## 1. 核心特性

* **全系列支持**：涵盖高级定时器 (TIM1/8)、通用定时器 (TIM2-5) 和基本定时器 (TIM6/7)。
* **自动总线适配**：自动识别定时器挂载在 APB1 (36MHz\*2) 还是 APB2 (72MHz)，统一计算分频系数。
* **中断自动配置**：初始化时自动开启 NVIC 中断通道（抢占优先级 2，子优先级 1）。

## 2. 快速上手

### 2.1 推荐用法 (使用基本定时器)

如果你只是需要一个纯软件定时（比如每秒打印一次日志，或者 LED 闪烁），**强烈推荐使用 `RUN_TIM6` 或 `RUN_TIM7`**。因为这两个定时器没有外部引脚，不会占用 GPIO 资源。

### 2.2 代码示例

假设我们要让 LED 每隔 500ms 翻转一次，使用 **TIM6**：

**C**

```
#include "RUN_Timer.h"
#include "RUN_Gpio.h" // 假设配合之前的 GPIO 模块

int main(void)
{
// 1. 初始化 GPIO (LED)
RUN_gpio_init(C13, GPO, 1);

// 2. 初始化定时器 6
// 设置为 500ms 触发一次中断
RUN_timer_init(RUN_TIM6, 500);

while (1)
{
    // 主循环可以处理其他事情，LED 翻转在中断里进行
}
}

// ==========================================================
// 3. 编写中断服务函数 (必须写在 main.c 或 stm32f10x_it.c 中)
// ==========================================================
// 注意：函数名必须是固定的 (见下文速查表)
void TIM6_IRQHandler(void)
{
// 检查更新中断标志位
if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
{
    // --- 用户逻辑开始 ---
    RUN_gpio_toggle(C13); // 翻转 LED
    // --- 用户逻辑结束 ---

    // 必须清除标志位！否则会死循环进入中断
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
}
}
```

---

## 3. API 接口详解

### 3.1 初始化定时器 `RUN_timer_init`

**C**

```
void RUN_timer_init(RUN_TIM_enum tim_n, uint16_t time_ms);
```

* **tim\_n**: 定时器枚举（如 `RUN_TIM6`）。
* **time\_ms**: 中断周期，单位毫秒 (ms)。
* **重要限制**：由于 STM32F103 的定时器是 16 位的，在默认分频策略下，最大定时周期约为 **6500ms (6.5秒)**。如果超过此值，定时器会溢出导致时间不准。

### 3.2 控制定时器开关 `RUN_timer_cmd`

**C**

```
void RUN_timer_cmd(RUN_TIM_enum tim_n, FunctionalState state);
```

* **tim\_n**: 定时器枚举。
* **state**: `ENABLE` (开始计数) 或 `DISABLE` (暂停计数)。

---

## 4. 中断服务函数速查表 (关键)

驱动只负责开启中断开关，**你必须自己在项目中编写中断服务函数**。如果函数名写错，程序将无法进入中断。


| **你的枚举 (tim\_n)** | **对应的中断函数名 (复制这个去写逻辑)** | **推荐用途**               |
| --------------------- | --------------------------------------- | -------------------------- |
| **`RUN_TIM6`**        | `void TIM6_IRQHandler(void)`            | **纯定时任务 (推荐)**      |
| **`RUN_TIM7`**        | `void TIM7_IRQHandler(void)`            | **纯定时任务 (推荐)**      |
| `RUN_TIM2`            | `void TIM2_IRQHandler(void)`            | 通用                       |
| `RUN_TIM3`            | `void TIM3_IRQHandler(void)`            | 通用                       |
| `RUN_TIM4`            | `void TIM4_IRQHandler(void)`            | 通用                       |
| `RUN_TIM5`            | `void TIM5_IRQHandler(void)`            | 通用                       |
| `RUN_TIM1`            | `void TIM1_UP_IRQHandler(void)`         | *高级定时器，中断名较特殊* |
| `RUN_TIM8`            | `void TIM8_UP_IRQHandler(void)`         | *高级定时器，中断名较特殊* |

---

## 5. 注意事项

1. **清除标志位**：在中断函数中，**必须**调用 `TIM_ClearITPendingBit(...)`，否则程序一旦退出中断会立刻再次进入，导致主程序卡死。
2. **多模块冲突**：

* TIM1 和 TIM8 是高级定时器，通常用于电机 PWM 控制。如果你的项目需要控制电机，请不要占用这两个定时器做普通的定时中断。
* TIM2/3/4/5 经常用于编码器读取或 PWM 输出，资源紧张时请优先使用 TIM6/7。

3. **最大时长限制**：输入参数 `time_ms` 不要超过 **6500**。如果需要更长的定时（比如 1 分钟），建议定一个 1000ms 的中断，然后在中断里用 `static int count` 变量累加计数。

# STM32 PWM 驱动模块使用说明

本模块支持 STM32F103 全系列定时器 (TIM1\~TIM8) 的 PWM 输出。通过统一的枚举接口，自动处理了复杂的**频率分频计算 (PSC/ARR)**、**GPIO 复用重映射 (Remap)** 以及**高级定时器的 MOE 开启**。

## 1. 核心特性

* **频率自适应**：输入 `20000` (20kHz) 即可，驱动会自动计算最佳的预分频系数 (PSC) 和重装载值 (ARR)，优先保证高分辨率。
* **自动重映射**：使用重映射引脚（如 TIM2 在 PA15）时，驱动自动开启 AFIO 并配置 `GPIO_FullRemap`，无需人工干预。
* **JTAG 冲突保护**：当使用 PA15/PB3/PB4 等 JTAG 复用引脚作为 PWM 时，驱动会自动关闭 JTAG 调试功能（保留 SWD），防止引脚不可控。
* **统一量程**：占空比控制统一为 `0 ~ 10000` (对应 0.00% \~ 100.00%)。

## 2. 快速上手

### 2.1 基础 PWM 输出

假设控制一个舵机（50Hz）和一个电机驱动（20kHz）。

**C**

```
#include "RUN_PWM.h"

int main(void)
{
// 1. 初始化舵机 PWM (PA8 - TIM1_CH1)
// 频率: 50Hz, 初始占空比: 7.5% (750/10000) -> 1.5ms
RUN_pwm_init(PWM_TIM1_CH1_PA8, 50, 750);

// 2. 初始化电机 PWM (PA0 - TIM2_CH1)
// 频率: 20kHz, 初始占空比: 0%
RUN_pwm_init(PWM_TIM2_CH1_PA0, 20000, 0);

while (1)
{
// 动态加速
for (int i = 0; i <= 10000; i += 100) 
{
    RUN_pwm_set(PWM_TIM2_CH1_PA0, i); // 设置占空比

    // 简单延时
    for(int d=0; d<10000; d++); 
}
}
}
```

---

## 3. API 接口详解

### 3.1 初始化 `RUN_pwm_init`

**C**

```
void RUN_pwm_init(RUN_PWM_enum pwm_ch, uint32_t freq, uint32_t duty);
```

* **pwm\_ch**: 通道枚举（如 `PWM_TIM1_CH1_PA8`）。
* **freq**: 目标频率 (Hz)。例如 `50` (舵机), `20000` (电机)。
* **duty**: 初始占空比，范围 **0 \~ 10000**。
* `0` = 0% (常低)
* `5000` = 50%
* `10000` = 100% (常高)

### 3.2 设置占空比 `RUN_pwm_set`

**C**

```
void RUN_pwm_set(RUN_PWM_enum pwm_ch, uint32_t duty);
```

* **功能**：实时修改指定通道的占空比。
* **duty**: 目标值 (0\~10000)。函数内部会自动根据当前频率的 ARR 值进行线性映射。

### 3.3 修改频率 `RUN_pwm_freq`

**C**

```
void RUN_pwm_freq(RUN_PWM_enum pwm_ch, uint32_t freq);
```

* **功能**：运行时动态改变 PWM 频率（如变频控制）。
* **注意**：修改频率会改变底层的 ARR 寄存器，虽然 `RUN_pwm_set` 会自动适应，但建议修改频率后立即刷新一次占空比。

---

## 4. 硬件资源速查表 (Enum List)

该模块已预置了 ZET6 芯片几乎所有的 PWM 资源。请根据枚举名称直接调用，**无需关心底层是部分重映射还是完全重映射**。

### 高级定时器 (TIM1, TIM8) - APB2 (72MHz)


| **定时器** | **通道** | **默认引脚**   | **重映射引脚 (Full Remap)** |
| ---------- | -------- | -------------- | --------------------------- |
| **TIM1**   | CH1      | **PA8**        | **PE9**                     |
|            | CH2      | **PA9**        | **PE11**                    |
|            | CH3      | **PA10**       | **PE13**                    |
|            | CH4      | **PA11**       | **PE14**                    |
| **TIM8**   | CH1\~4   | **PC6 \~ PC9** | (未配置重映射)              |

### 通用定时器 (TIM2 \~ TIM5) - APB1 (72MHz)


| **定时器** | **通道** | **默认引脚**   | **重映射引脚**   | **特殊说明**                 |
| ---------- | -------- | -------------- | ---------------- | ---------------------------- |
| **TIM2**   | CH1      | **PA0**        | **PA15**         | PA15 重映射会自动关闭 JTAG   |
|            | CH2      | **PA1**        | **PB3**          | PB3 重映射会自动关闭 JTAG    |
|            | CH3      | **PA2**        | **PB10**         |                              |
|            | CH4      | **PA3**        | **PB11**         |                              |
| **TIM3**   | CH1\~2   | **PA6, PA7**   | **PC6, PC7**     |                              |
|            | CH3\~4   | **PB0, PB1**   | **PC8, PC9**     |                              |
| **TIM4**   | CH1\~4   | **PB6 \~ PB9** | **PD12 \~ PD15** |                              |
| **TIM5**   | CH1\~4   | **PA0 \~ PA3** | (无)             | **注意与 TIM2 默认引脚冲突** |

*[数据来源: RUN\_PWM.c / pwm\_cfg 数组]*

---

## 5. 常见问题与注意事项

1. **引脚冲突**：

* **PA0/PA1/PA2/PA3** 同时被 TIM2 和 TIM5 使用。同一时间只能选择其中一个定时器初始化，否则会产生硬件冲突。
* **TIM2 重映射 (PA15/PB3)**：代码会自动执行 `GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE)`。这意味着初始化这些引脚后，**J-Link/ST-Link 的 JTAG 模式将失效**，但 SWD 模式（两根线）通常不受影响。请确保下载器使用 SWD 接口。

2. **高级定时器 (TIM1/TIM8)**：

* 这两个定时器有额外的 "死区控制" 和 "刹车功能"。本驱动默认开启了主输出 (MOE)，可以像普通定时器一样直接驱动电机。

3. **最低频率限制**：

* 由于算法使用 72MHz 主频，且 PSC 为 16 位，理论最低频率约为 **20Hz** (72M/65536/65536)。如果需要更低的频率，需要修改底层的时钟分频逻辑。

# STM32 EXTI 外部中断驱动模块使用说明

本模块封装了 STM32 的外部中断 (EXTI) 功能。它自动处理了 GPIO 输入配置、AFIO 中断线映射、NVIC 优先级配置以及中断服务函数的编写。用户只需关注“触发后执行什么代码”。

## 1. 核心特性

* **回调机制**：无需在 `stm32f10x_it.c` 编写 ISR，直接将你的 C 函数注册给驱动即可。
* **自动分发**：自动处理 STM32 的 `EXTI9_5` 和 `EXTI15_10` 共享中断入口，精确识别是哪个引脚触发了中断。
* **极简初始化**：一行代码完成 GPIO、AFIO 和 EXTI 的所有底层配置。

## 2. 快速上手

### 2.1 编写流程

1. **定义功能函数**：写一个 `void` 类型、无参数的函数（例如 `Key_Handler`）。
2. **初始化中断**：调用 `RUN_exti_init` 配置触发方式。
3. **挂载回调**：调用 `RUN_exti_attach` 将你的函数与引脚绑定。

### 2.2 代码示例

假设 **PA0** 连接了一个按键，按下时产生上升沿（高电平），触发中断翻转 LED。

**C**

```
#include "RUN_Exti.h"
#include "RUN_Gpio.h"

// 1. 定义你的中断处理逻辑 (回调函数)
void My_Key_Callback(void)
{
// 这里不需要清除标志位，驱动已经帮你清除了
RUN_gpio_toggle(C13); // 翻转 LED
}

int main(void)
{
// 初始化 LED
RUN_gpio_init(C13, GPO, 1);

// 2. 初始化外部中断 (PA0, 上升沿触发)
// 触发模式可选: EXTI_Trigger_Rising, EXTI_Trigger_Falling, EXTI_Trigger_Rising_Falling
RUN_exti_init(A0, EXTI_Trigger_Rising);

// 3. 挂载回调函数 (将 A0 中断指向 My_Key_Callback)
RUN_exti_attach(A0, My_Key_Callback);

while (1)
{
// 主循环继续做其他事...
}
}
```

---

## 3. API 接口详解

### 3.1 初始化中断 `RUN_exti_init`

**C**

```
void RUN_exti_init(RUN_GPIO_enum pin, EXTITrigger_TypeDef trigger);
```

* **pin**: 引脚枚举 (如 `A0`, `B5`, `C13`)。注意：此函数会自动将该引脚的 GPIO 模式配置为 **浮空输入 (IN\_FLOATING)**。
* **trigger**: 触发方式 (标准库宏)。
* `EXTI_Trigger_Rising`: 上升沿触发 (低 -> 高)
* `EXTI_Trigger_Falling`: 下降沿触发 (高 -> 低)
* `EXTI_Trigger_Rising_Falling`: 双边沿触发

### 3.2 注册回调 `RUN_exti_attach`

**C**

```
void RUN_exti_attach(RUN_GPIO_enum pin, ExtiCallback_t callback);
```

* **pin**: 产生中断的引脚 (如 `A0`)。
* **callback**: 函数的名字 (函数指针)。该函数必须是 `void func(void)` 格式。

### 3.3 解除注册 `RUN_exti_detach`

**C**

```
void RUN_exti_detach(RUN_GPIO_enum pin);
```

* **功能**: 取消该引脚的中断回调，触发后不再执行任何操作。

---

## 4. 关键注意事项 (必读)

### 4.1 中断线冲突 (Line Conflict)

STM32 的硬件机制决定了**相同的引脚号不能同时使用**。

* **错误示例**：同时使用 `A0` 和 `B0`。
* 原因：它们都对应 `EXTI_Line0`，硬件选通器 (AFIO) 同一时间只能选择其中一个连接到 CPU。
* **正确示例**：使用 `A0` 和 `B1`。

### 4.2 不要自己写 IRQHandler

本驱动 (`RUN_Exti.c`) 内部已经定义了以下中断服务函数：

* `EXTI0_IRQHandler`
* `EXTI1_IRQHandler`
* ...
* `EXTI15_10_IRQHandler`

**请勿**在其他文件（如 `stm32f10x_it.c`）中重复定义这些函数，否则编译器会报错 "Multiple definition of symbol"。

### 4.3 机械按键抖动

如果你将 EXTI 用于机械按键，**极其容易触发多次中断**（抖动）。

* **建议**：在回调函数里不要做复杂的翻转逻辑，或者配合定时器进行消抖。EXTI 对电平跳变非常敏感，ns 级的抖动都会被捕捉到。

### 4.4 优先级

驱动内部默认将所有 EXTI 中断的优先级配置为：

* **抢占优先级**: 2
* **子优先级**: 2
* 如果需要修改，请直接去 `RUN_Exti.c` 的 `RUN_exti_init` 函数中修改 `NVIC_InitStructure` 部分。

# STM32 ADC & DAC 模拟外设驱动说明

本说明文档包含两个独立模块：

1. **RUN\_ADC**: 模数转换器，用于读取传感器模拟电压。
2. **RUN\_DAC**: 数模转换器，用于输出指定电压信号。

---

# 第一部分：RUN\_ADC (模数转换)

该模块封装了 **ADC1** 的规则组采样功能。采用**单次转换、软件触发**的模式，并未开启 DMA 或中断，适合低频、简单的传感器读取场景。

## 1. 核心特性

* **安全时钟配置**：内部自动设置分频系数为 6 分频 (72MHz/6 = 12MHz)，确保不超过 STM32 ADC 的 14MHz 硬件限制，防止读数错误。
* **软件滤波**：提供 `Get_Average` 函数，支持多次采样取平均值，有效抑制信号抖动。
* **枚举映射**：通过枚举直接选择通道（如 `RUN_ADC_CH0_PA0`），无需记忆通道号。

## 2. 快速上手

### 2.1 基础读取示例

**C**

```
#include "RUN_ADC.h"
#include <stdio.h> // 用于 printf

int main(void)
{
// 1. 初始化 ADC (默认开启 ADC1)
RUN_ADC_Init();

// 注意：STM32 的 ADC 引脚建议配置为模拟输入 (AIN) 模式
// 虽然复位状态下引脚是浮空输入也能用，但配置为 AIN 可以关闭数字触发器，降低功耗和噪声。
// RUN_gpio_init(A0, AIN, 0); // 如果你有 RUN_Gpio 模块，建议加上这句

while (1)
{
// 2. 读取 PA0 电压值 (0 ~ 4095)
uint16_t adc_val = RUN_ADC_Get(RUN_ADC_CH0_PA0);

// 3. 换算为实际电压 (假设参考电压 3.3V)
float voltage = (float)adc_val * 3.3f / 4095.0f;

printf("PA0 Val: %d, Vol: %.2fv\r\n", adc_val, voltage);

// 4. 使用滤波读取 (连续采 20 次取平均)
uint16_t avg_val = RUN_ADC_Get_Average(RUN_ADC_CH0_PA0, 20);
}
}
```

## 3. API 接口详解

### 3.1 初始化 `RUN_ADC_Init`

**C**

```
void RUN_ADC_Init(void);
```

* **功能**：开启 ADC1 时钟，配置时钟分频 (<14MHz)，校准 ADC。
* **注意**：此函数只配置 ADC 外设本身，不包含 GPIO 配置。虽然 ADC 可以在 GPIO 默认浮空输入下工作，但建议配合 `RUN_Gpio` 将对应引脚设为 `AIN` (模拟输入)。

### 3.2 单次读取 `RUN_ADC_Get`

**C**

```
uint16_t RUN_ADC_Get(RUN_ADC_enum ch);
```

* **ch**: 通道枚举 (如 `RUN_ADC_CH0_PA0`)。
* **返回**: 12位原始数值 (0\~4095)。
* **机制**: 配置通道 -> 启动转换 -> 等待结束 -> 读取。属于阻塞式函数，但耗时极短 (约几微秒)。

### 3.3 平均读取 `RUN_ADC_Get_Average`

**C**

```
uint16_t RUN_ADC_Get_Average(RUN_ADC_enum ch, uint8_t times);
```

* **times**: 连续采样次数 (推荐 10\~50)。次数过多会增加阻塞时间。

## 4. ADC 硬件通道速查 (ADC1)


| **枚举名称**           | **对应引脚** | **备注**                       |
| ---------------------- | ------------ | ------------------------------ |
| **`RUN_ADC_CH0_PA0`**  | PA0          | 常用                           |
| **`RUN_ADC_CH1_PA1`**  | PA1          |                                |
| **`RUN_ADC_CH2_PA2`**  | PA2          |                                |
| **`RUN_ADC_CH3_PA3`**  | PA3          |                                |
| **`RUN_ADC_CH4_PA4`**  | PA4          | 如开启 DAC，不要用此通道做 ADC |
| **`RUN_ADC_CH5_PA5`**  | PA5          | 如开启 DAC，不要用此通道做 ADC |
| **`RUN_ADC_CH6_PA6`**  | PA6          |                                |
| **`RUN_ADC_CH7_PA7`**  | PA7          |                                |
| **`RUN_ADC_CH8_PB0`**  | PB0          |                                |
| **`RUN_ADC_CH9_PB1`**  | PB1          |                                |
| **`RUN_ADC_CH10_PC0`** | PC0          |                                |
| **`RUN_ADC_CH1x_PCx`** | PC1\~ PC5    | 详见 .h 文件                   |

---

# 第二部分：RUN\_DAC (数模转换)

该模块用于驱动 STM32F103ZET6 (或 RCT6 等大容量芯片) 自带的 DAC 模块，输出 0\~3.3V 的模拟电压。

## 1. 核心特性

* **电压直设**：提供 `Set_Vol` 函数，直接输入浮点电压值 (如 1.5)，驱动自动计算寄存器值。
* **自动初始化 GPIO**：初始化函数内部会自动开启 `GPIOA` 时钟并将 PA4/PA5 配置为 **模拟输入 (AIN)** 模式（这是 STM32 DAC 输出引脚的硬件要求）。
* **软件限幅**：防止输入的电压值超过 3.3V 导致计算溢出。

## 2. 快速上手

### 2.1 输出指定电压

**C**

```
#include "RUN_DAC.h"

int main(void)
{
// 1. 初始化 DAC 通道 1 (对应引脚 PA4)
RUN_DAC_Init(RUN_DAC_CH1_PA4);

while (1)
{
// 2. 输出 1.5V 电压
RUN_DAC_Set_Vol(RUN_DAC_CH1_PA4, 1.5f);

// 延时一会儿...

// 3. 输出最大电压 (3.3V)
RUN_DAC_Set_Vol(RUN_DAC_CH1_PA4, 3.3f);
}
}
```

## 3. API 接口详解

### 3.1 初始化 `RUN_DAC_Init`

**C**

```
void RUN_DAC_Init(RUN_DAC_Channel_t channel);
```

* **channel**:
* `RUN_DAC_CH1_PA4`: 使用 PA4 引脚输出。
* `RUN_DAC_CH2_PA5`: 使用 PA5 引脚输出。
* **注意**: 初始化会自动配置 GPIO，无需额外调用 GPIO 驱动。

### 3.2 设置电压 `RUN_DAC_Set_Vol`

**C**

```
void RUN_DAC_Set_Vol(RUN_DAC_Channel_t channel, float vol);
```

* **vol**: 目标电压 (0.0 \~ 3.3)。
* **精度**: STM32 DAC 为 12 位，理论分辨率约 0.8mV。
* **公式**: **\$DAC\\\_Value = \\frac{Voltage}{3.3} \\times 4096\$**。

### 3.3 设置原始值 `RUN_DAC_Set_Value`

**C**

```
void RUN_DAC_Set_Value(RUN_DAC_Channel_t channel, uint16_t val);
```

* **val**: 0 \~ 4095。
* **用途**: 比浮点运算更快，适合生成高频波形（如正弦波）。

## 4. 注意事项

1. **引脚冲突**：

* **PA4** 和 **PA5** 既是 DAC 输出，也是 ADC 通道 4/5，还是 SPI1 的 NSS/SCK 引脚。
* 一旦初始化为 DAC，请勿再将其作为普通 GPIO、ADC 输入或 SPI 接口使用。

1. **驱动能力**：

* STM32 的 DAC 输出阻抗较高，**驱动能力很弱**。
* 如果需要驱动负载（如电机、喇叭、强光 LED），**必须**外部加装运放跟随器 (Buffer) 或驱动电路，否则电压会被拉低。

3. **参考电压**：

* 代码中写死计算基准为 `3.3f`。如果你的板子 VDDA/Vref+ 接的是 3.0V 或 2.5V，请修改 `RUN_DAC.c` 中的计算公式。

# TM32 硬件 SPI 驱动模块使用说明

本模块支持 SPI1、SPI2 和 SPI3 的多种引脚组合。驱动会自动处理时钟开启、GPIO 复用配置以及 AFIO 重映射。

## 1. 核心特性

* **全硬件控制**：基于 STM32 硬件 SPI 控制器，支持高达 18MHz+ 的通信速率（取决于 APB 分频）。
* **智能重映射**：如果选择 `RUN_SPI_1_PB3_PB4_PB5_REMAP`，驱动会自动开启 AFIO 时钟并关闭 JTAG 调试端口（仅保留 SWD），防止引脚冲突。
* **动态变速**：提供 `RUN_SPI_SetSpeed` 函数，允许在运行时动态切换 SPI 速度（常用于 SD 卡初始化阶段低速，读写阶段高速）。
* **读写一体**：标准 SPI 机制，发送一个字节的同时必定接收一个字节。

## 2. 快速上手

### 2.1 接线与初始化

假设使用 SPI1 (PA5/6/7) 连接一个外部 Flash 模块。

注意：片选引脚 (CS/NSS) 需要你单独用普通 GPIO 控制，不包含在 SPI 硬件引脚中。

**C**

```
#include "RUN_SPI.h"
#include "RUN_Gpio.h" // 假设你有 GPIO 模块用于控制 CS 片选

#define CS_PIN  A4 // 假设片选接在 PA4

int main(void)
{
// 1. 初始化 CS 引脚 (推挽输出，默认拉高)
RUN_gpio_init(CS_PIN, GPO, 1);

// 2. 初始化 SPI1 (SCK=PA5, MISO=PA6, MOSI=PA7)
// 初始化后默认处于高速模式
RUN_SPI_Init(RUN_SPI_1_PA5_PA6_PA7);

while (1)
{
    // --- 发送/读取示例 ---

    // 1. 拉低片选，开始通信
    RUN_gpio_set(CS_PIN, 0);

    // 2. 发送 0xAA，同时读取从机返回的数据
    uint8_t received_data = RUN_SPI_ReadWriteByte(RUN_SPI_1_PA5_PA6_PA7, 0xAA);

    // 3. 拉高片选，结束通信
    RUN_gpio_set(CS_PIN, 1);
}
}
```

---

## 3. API 接口详解

### 3.1 初始化 `RUN_SPI_Init`

**C**

```
void RUN_SPI_Init(RUN_SPI_Port_t port_group);
```

* **功能**：初始化 GPIO 复用模式，配置 SPI 为主机模式 (Master)，默认 CPOL=1, CPHA=2 (模式3) 或 模式0 (具体视代码底层配置而定，通常通用驱动默认为模式0或3兼容性较好)。
* **port\_group**：引脚组合枚举（见下文速查表）。

### 3.2 读写交换 `RUN_SPI_ReadWriteByte` (核心)

**C**

```
uint8_t RUN_SPI_ReadWriteByte(RUN_SPI_Port_t port_group, uint8_t data);
```

* **功能**：SPI 的本质是“交换”。发送 `data` 的同时，返回从机发来的数据。
* **如果只想发**：忽略返回值即可。
* **如果只想读**：发送 `0xFF` (Dummy Byte) 来产生时钟信号，并接收返回值。

### 3.3 简易封装函数

为了方便使用，模块内提供了两个宏/封装：

* **`RUN_SPI_WriteByte(port, data)`**：只发送，不关心返回值。
* **`RUN_SPI_ReadByte(port)`**：发送 0xFF，只关心返回值。

### 3.4 动态变速 `RUN_SPI_SetSpeed`

**C**

```
void RUN_SPI_SetSpeed(RUN_SPI_Port_t port_group, uint8_t speed);
```

* **speed 参数**：
* `0`: **低速** (256分频) —— 约 280kHz (72M/256)。适合 SD 卡初始化。
* `1`: **中速** (16分频) —— 约 4.5MHz。
* `2`: **高速** (2分频) —— 约 36MHz (SPI1) 或 18MHz (SPI2/3)。
* **场景**：SD 卡驱动通常先用低速初始化，成功后再切高速读写。

---

## 4. 硬件资源速查表 (Enum List)

请根据你的 PCB 连线选择对应的枚举。


| **枚举名称 (port\_group)**    | **SCK** | **MISO** | **MOSI** | **APB总线** | **备注**                                          |
| ----------------------------- | ------- | -------- | -------- | ----------- | ------------------------------------------------- |
| **`RUN_SPI_1_PA5_PA6_PA7`**   | **PA5** | **PA6**  | **PA7**  | APB2 (快)   | **推荐，最常用**                                  |
| `RUN_SPI_1_PB3_PB4_PB5_REMAP` | PB3     | PB4      | PB5      | APB2 (快)   | **注意**：PB3/4 是 JTAG 口，初始化会自动关闭 JTAG |
| `RUN_SPI_2_PB13_PB14_PB15`    | PB13    | PB14     | PB15     | APB1        | 通用，ZET6 板载 Flash 常接此处                    |
| `RUN_SPI_3_PB3_PB4_PB5`       | PB3     | PB4      | PB5      | APB1        | 与 SPI1 重映射引脚相同，但属于 SPI3 外设          |

---

## 5. 常见问题与注意事项

### 5.1 片选 (CS/SS) 引脚在哪里？

SPI 驱动不管理片选引脚。

SPI 协议中，主机通过拉低 CS 引脚来选中从机。你需要使用普通 GPIO 驱动（如 RUN\_Gpio.h）将任意一个空闲引脚配置为推挽输出，并在读写前后手动拉低/拉高。

### 5.2 速度差异

* **SPI1** 挂载在 **APB2 (72MHz)** 总线上，最高速率可达 36MHz（理论值，受限于 IO 翻转速度和线长）。
* **SPI2/3** 挂载在 **APB1 (36MHz)** 总线上，最高速率为 18MHz。
* 如果你追求极限刷屏速度（如驱动 TFT 屏幕），请务必使用 **SPI1**。

### 5.3 读不到数据 (读出全是 0x00 或 0xFF)

1. **片选没拉低**：检查 CS 引脚代码逻辑。
2. **MISO/MOSI 接反**：主机的 MOSI 接从机的 MOSI（如果是直连），有的模块丝印也是 TX/RX 风格，容易混淆。正确逻辑是：Master Out -> Slave In。
3. **模式不匹配**：大部分模块支持 SPI Mode 0 (CPOL=0, CPHA=1Edge) 或 Mode 3。如果读写不稳定，可能需要去 `RUN_SPI.c` 的 Init 函数里修改 `SPI_CPOL` 和 `SPI_CPHA`。

# STM32 软件模拟 I2C 驱动模块使用说明

本模块通过操作 GPIO 的高低电平来模拟 I2C 时序。采用了**对象化**的设计思路，定义一个 `RUN_SoftI2C_Bus_t` 结构体即可代表一条 I2C 总线，因此支持同时驱动多路 I2C 设备（如一路接 OLED，一路接 MPU6050）。

## 1. 核心特性

* **多总线支持**：通过结构体管理总线，可以在任意引脚上定义无数个 I2C 接口。
* **稳定可靠**：避开了 STM32F1 硬件 I2C 易卡死（Busy Flag 锁死）的硬件 Bug。
* **高度封装**：提供了常用的 `WriteReg` 和 `ReadReg` 函数，自动处理起始信号、ACK 等细节。

## 2. 快速上手

### 2.1 典型接线场景

* **SCL**: PB6
* **SDA**: PB7
* **设备**: MPU6050 (地址 0xD0)

### 2.2 代码示例

**C**

```
#include "RUN_SoftI2C.h"

// 1. 定义一个 I2C 总线对象 (全局变量)
RUN_SoftI2C_Bus_t mpu_bus;

int main(void)
{
// 2. 初始化总线 (SCL->PB6, SDA->PB7)
// 驱动内部会自动调用 RUN_gpio_init 配置引脚为开漏输出
RUN_I2C_Init(&mpu_bus, B6, B7);

// 3. 写寄存器示例 (向 MPU6050 的 PWR_MGMT_1 寄存器 0x6B 写入 0x00 解除休眠)
// 参数: &总线, 设备地址, 寄存器地址, 数据
RUN_I2C_WriteReg(&mpu_bus, 0xD0, 0x6B, 0x00);

while (1)
{
    // 4. 读寄存器示例 (读取 WHO_AM_I 寄存器 0x75)
    uint8_t id = RUN_I2C_ReadReg(&mpu_bus, 0xD0, 0x75);

    // 简单延时
    RUN_delay_ms(100);
}
}
```

---

## 3. API 接口详解

### 3.1 初始化 `RUN_I2C_Init`

**C**

```
void RUN_I2C_Init(RUN_SoftI2C_Bus_t* bus, RUN_GPIO_enum scl, RUN_GPIO_enum sda);
```

* **bus**: 指向总线结构体的指针。
* **scl / sda**: 引脚枚举 (如 `B6`, `B7`)。
* **内部动作**: 该函数会自动调用 `RUN_gpio_init` 将引脚初始化为 **开漏输出 (GPO\_OD)** 模式，并将引脚拉高（释放总线）。

### 3.2 写寄存器 `RUN_I2C_WriteReg` (最常用)

**C**

```
void RUN_I2C_WriteReg(RUN_SoftI2C_Bus_t* bus, uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
```

* **功能**: 向指定设备的指定寄存器写入一个字节。
* **dev\_addr**: 8位设备地址 (如 `0xD0` 或 `0x78`)。注意：通常是写地址（最低位为0）。
* **流程**: Start -> 发送设备地址(W) -> WaitAck -> 发送寄存器地址 -> WaitAck -> 发送数据 -> WaitAck -> Stop。

### 3.3 读寄存器 `RUN_I2C_ReadReg` (最常用)

**C**

```
uint8_t RUN_I2C_ReadReg(RUN_SoftI2C_Bus_t* bus, uint8_t dev_addr, uint8_t reg_addr);
```

* **功能**: 读取指定寄存器的一个字节数据。
* **流程**: Start -> 发送设备地址(W) -> WaitAck -> 发送寄存器地址 -> WaitAck -> **Restart** -> 发送设备地址(R) -> WaitAck -> 读取数据 -> NoAck -> Stop。

### 3.4 底层读写 (高级用法)

如果需要传输多个字节（如连续读取 FIFO），可以使用底层函数组合：

**C**

```
RUN_I2C_Start(&mpu_bus);
RUN_I2C_SendByte(&mpu_bus, 0xD0); // 写地址
RUN_I2C_WaitAck(&mpu_bus);
RUN_I2C_SendByte(&mpu_bus, 0x3B); // 起始寄存器
RUN_I2C_WaitAck(&mpu_bus);

RUN_I2C_Start(&mpu_bus);         // 重复起始
RUN_I2C_SendByte(&mpu_bus, 0xD1); // 读地址
RUN_I2C_WaitAck(&mpu_bus);

uint8_t data1 = RUN_I2C_ReadByte(&mpu_bus, 1); // 1 = 发送 ACK (继续读)
uint8_t data2 = RUN_I2C_ReadByte(&mpu_bus, 0); // 0 = 发送 NACK (停止读)

RUN_I2C_Stop(&mpu_bus);
```

---

## 4. 注意事项

### 4.1 上拉电阻

* I2C 协议要求总线为空闲（高电平）状态。驱动中使用的是 **开漏输出 (Open Drain)** 模式。
* **必须确保** SCL 和 SDA 线上有上拉电阻（通常是 4.7kΩ 或 10kΩ）。
* 如果外部没有电阻，模块代码通常无法拉高电平（除非修改 `RUN_Gpio` 初始化部分为推挽输出，但这不符合 I2C 规范且不安全）。

### 4.2 延时与速度

* 在 `RUN_SoftI2C.c` 内部宏定义中通常包含微秒级延时（如 `RUN_delay_us`）。
* 如果通信失败或数据错乱，可能是延时太短导致波形畸变，或者延时太长导致通信过慢。目前的默认延时通常适配 100kHz \~ 400kHz 的标准速度。

### 4.3 设备地址

* 传入 `dev_addr` 时，请确保传入的是 **8位写地址**（例如 MPU6050 是 `0xD0` 而不是 7位地址 `0x68`）。函数内部会自动处理读写位的转换。

# STM32 单总线 (OneWire) 驱动模块使用说明

本模块通过极其精确的微秒级延时，在一个 GPIO 引脚上模拟单总线时序。支持动态指定引脚，内部自动处理输入/输出模式切换。

## 1. 核心特性

* **动态引脚配置**：初始化时传入 GPIO 端口和引脚号即可，无需修改头文件宏定义。
* **自动模式切换**：驱动内部会自动在“推挽输出”和“上拉输入”之间切换，以满足单总线“线与”逻辑。
* **依赖说明**：**必须** 配合 `RUN_Delay.h` 中的 `RUN_delay_us()` 使用，否则时序将会错乱。

## 2. 硬件连接与注意事项

1. **上拉电阻 (关键)**：

* OneWire 总线空闲时必须为高电平。
* 虽然代码中使用了 `GPIO_Mode_IPU` (内部上拉)，但 STM32 内部上拉电阻较弱 (约 40kΩ)。
* **强烈建议**在数据线 (DQ) 和 VCC (3.3V) 之间接一个 **4.7kΩ 的外部上拉电阻**，以保证通信稳定。

2. **时序敏感**：

* 单总线对时间要求极高（微秒级）。如果在读写过程中被高优先级中断打断，会导致数据出错。

## 3. 快速上手 (以读取 DS18B20 为例)

### 3.1 基础代码示例

**C**

```
#include "RUN_OneWire.h"
#include "RUN_Delay.h" // 必须包含

// 假设传感器接在 PA0
#define DS18B20_PORT GPIOA
#define DS18B20_PIN  GPIO_Pin_0

int main(void)
{
// 1. 系统依赖初始化 (延时函数必须先初始化)
RUN_delay_init(72);

// 2. 单总线初始化 (选中 PA0)
RUN_OneWire_Init(DS18B20_PORT, DS18B20_PIN);

// 3. 检测设备是否存在
if (RUN_OneWire_Reset() == 0) 
{
    // 复位成功，设备在线
    printf("Device Found!\r\n");
}
else
{
    printf("No Device!\r\n");
}

while (1)
{
    // --- DS18B20 温度读取流程 ---

    // Step 1: 复位
    RUN_OneWire_Reset();
    // Step 2: 跳过 ROM (0xCC) - 假设总线上只有一个设备
    RUN_OneWire_WriteByte(0xCC);
    // Step 3: 启动温度转换 (0x44)
    RUN_OneWire_WriteByte(0x44);

    // 等待转换完成 (DS18B20 12位精度最长需 750ms)
    RUN_delay_ms(750);

    // Step 4: 再次复位
    RUN_OneWire_Reset();
    // Step 5: 跳过 ROM
    RUN_OneWire_WriteByte(0xCC);
    // Step 6: 读取暂存器 (0xBE)
    RUN_OneWire_WriteByte(0xBE);

    // Step 7: 读取两个字节 (低8位, 高8位)
    uint8_t LSB = RUN_OneWire_ReadByte();
    uint8_t MSB = RUN_OneWire_ReadByte();

    // 合成温度值
    int16_t temp_raw = (MSB << 8) | LSB;
    float temperature = (float)temp_raw * 0.0625;

    printf("Temp: %.2f C\r\n", temperature);

    RUN_delay_ms(200);
}
}
```

---

## 4. API 接口详解

### 4.1 初始化 `RUN_OneWire_Init`

**C**

```
void RUN_OneWire_Init(GPIO_TypeDef* port, uint16_t pin);
```

* **功能**：开启对应 GPIO 的时钟，并将引脚初始化为推挽输出（默认拉高）。
* **Global 限制**：由于使用了全局变量记录引脚信息，**同一时刻只能操作一个 OneWire 引脚**。如果需要操作多个传感器（挂在不同引脚上），每次操作前都需要重新调用此函数切换目标引脚。

### 4.2 复位/检测 `RUN_OneWire_Reset`

**C**

```
uint8_t RUN_OneWire_Reset(void);
```

* **功能**：发送复位脉冲 (拉低 480us)，然后检测是否有从机发送“存在脉冲” (Presence Pulse)。
* **返回值**：
* `0`: 检测到设备 (成功)。
* `1`: 无响应 (失败，线路断开或设备损坏)。
* **时序图**：

### 4.3 写字节 `RUN_OneWire_WriteByte`

**C**

```
void RUN_OneWire_WriteByte(uint8_t data);
```

* **功能**：发送 8 位数据（低位先出 LSB First）。
* **实现**：内部循环调用 8 次 `RUN_OneWire_WriteBit`。
* **注意**：如果你的系统中有高频率的中断（如电机控制 PWM 中断），建议在调用此函数前后加上 `__disable_irq()` 和 `__enable_irq()`，防止时序被拉长导致写入错误。

### 4.4 读字节 `RUN_OneWire_ReadByte`

**C**

```
uint8_t RUN_OneWire_ReadByte(void);
```

* **功能**：读取 8 位数据。
* **实现**：主机拉低总线 1us 后释放，读取从机状态。

---

## 5. 进阶说明

### 5.1 全局变量风险

代码中使用了 `static GPIO_TypeDef* g_OW_PORT;` 等静态全局变量。

* **这意味着**：该驱动不是“线程安全”的，也不是“多实例”的。
* **场景**：如果你有 PA0 接了一个 DS18B20，PB5 接了另一个 DS18B20。
* **错误做法**：初始化 PA0，初始化 PB5，然后交替读写。
* **正确做法**：

1. `RUN_OneWire_Init(GPIOA, GPIO_Pin_0);` -> 读写 PA0。
2. `RUN_OneWire_Init(GPIOB, GPIO_Pin_5);` -> 读写 PB5。

### 5.2 延时精度

OneWire 对延时非常敏感。例如写 "1" 时，总线必须在拉低后 15us 内释放。

* 请确保 `RUN_delay_us` 函数足够精准。
* 如果使用了 FreeRTOS 或 UCOS，建议在读写时序段进入临界区（关中断）。

# STM32 内部 Flash 模拟 EEPROM 驱动模块使用说明

本模块专为 **保存系统参数** 设计。它开辟了一块 2KB 的 RAM 缓冲区，实现了“读出旧数据 -> 修改数据 -> 擦除整页 -> 回写新数据”的自动化流程。

## 1. 核心特性

* **零外部成本**：不需要外挂 AT24C02 等 EEPROM 芯片，直接利用单片机剩余空间。
* **任意读写**：虽然 Flash 物理上必须按页擦除，但本驱动支持**任意偏移量、任意长度**的写入，不会丢失该页其他位置的数据。
* **断电不丢失**：数据存储在 Flash ROM 中，掉电永久保存。
* **结构体支持**：通过指针操作，可以一行代码保存/读取复杂的结构体参数。

## 2. 快速上手

### 2.1 最佳实践：保存系统参数

假设你需要保存 PID 参数和屏幕亮度设置，建议定义一个结构体，然后整体读写。

**C**

```
#include "RUN_Flash.h"

// 1. 定义参数结构体 (必须小于 2KB)
typedef struct {
float kp;
float ki;
float kd;
uint8_t lcd_brightness;
uint32_t boot_count; // 开机次数记录
} SystemParams_t;

SystemParams_t my_params;

int main(void)
{
// --- 读取参数 (开机时) ---
// 从偏移量 0 处读取结构体大小的数据
RUN_Flash_Read(0, &my_params, sizeof(SystemParams_t));

// 如果参数全是 0xFF (第一次使用或被擦除)，则初始化默认值
if (my_params.kp == 0xFFFFFFFF && my_params.ki == 0xFFFFFFFF) // 简单的判断逻辑
{
    my_params.kp = 1.5f;
    my_params.ki = 0.05f;
    my_params.kd = 0.01f;
    my_params.lcd_brightness = 80;
    my_params.boot_count = 0;

    // 立即写入默认值
    RUN_Flash_Write(0, &my_params, sizeof(SystemParams_t));
}

// 更新开机次数
my_params.boot_count++;

// --- 保存参数 (关机前或修改参数后) ---
// 将修改后的结构体写回 Flash
RUN_Flash_Write(0, &my_params, sizeof(SystemParams_t));

while (1)
{
}
}
```

---

## 3. API 接口详解

### 3.1 写数据 `RUN_Flash_Write`

**C**

```
void RUN_Flash_Write(uint16_t offset, void *pData, uint16_t len);
```

* **功能**：将数据写入 Flash 最后一页的指定位置。
* **offset**：页内偏移地址 (0 \~ 2047)。例如偏移 10，实际地址是 `0x0807F80A`。
* **pData**：源数据指针（可以是变量地址、数组名、结构体指针）。
* **len**：写入数据的字节长度 (可以使用 `sizeof()`)。
* **警告**：此函数耗时较长（几十毫秒），因为涉及擦除操作。**切勿在中断中调用**。

### 3.2 读数据 `RUN_Flash_Read`

**C**

```
void RUN_Flash_Read(uint16_t offset, void *pData, uint16_t len);
```

* **功能**：从 Flash 读取数据到 RAM 变量中。
* **原理**：STM32 的 Flash 支持内存映射访问，读取操作等同于 `memcpy`，速度非常快，不消耗额外时间。

---

## 4. 技术细节与配置

### 4.1 芯片容量适配 (关键)

当前代码配置是针对 **STM32F103ZET6 (大容量 High Density)** 的：

**C**

```
#define FLASH_START_ADDR  0x0807F800  // 512KB Flash 的最后一页起始地址
#define FLASH_PAGE_SIZE   2048        // 大容量设备一页为 2KB
```

**如果你使用的是 C8T6 (中容量 64KB)：** 请务必修改 `.h` 文件：

**C**

```
// C8T6 配置 (64KB, Page=1KB)
#define FLASH_START_ADDR  0x0800FC00  // 64KB Flash 的最后一页 (0x08000000 + 64*1024 - 1024)
#define FLASH_PAGE_SIZE   1024        // 中容量设备一页为 1KB
```

### 4.2 "读-改-写" 机制原理

STM32 的 Flash 只能把 1 写成 0，不能把 0 写成 1。要写入新数据，必须先擦除（全变 1）。 为了不丢失该页其他数据，`RUN_Flash.c` 内部维护了一个 2KB 的数组 `Flash_Cache_Buffer`：

1. **Read**: 先把整个 Flash 页读到 RAM 数组中。
2. **Modify**: 在 RAM 数组中修改你需要更新的那部分数据。
3. **Erase**: 擦除整个 Flash 物理页。
4. **Write**: 将更新后的 RAM 数组全部写回 Flash。

---

## 5. 注意事项 (风险提示)

### 5.1 寿命限制

* STM32 内部 Flash 的额定擦写次数通常为 **10,000 次**。
* **不要**在 `while(1)` 循环中高频率调用 `RUN_Flash_Write`，否则几天内芯片就会损坏。
* **建议策略**：只在参数实际发生改变时写入，或者检测到掉电信号时写入。

### 5.2 堆栈溢出风险

* `RUN_Flash.c` 定义了一个 2KB 的静态全局数组 `Flash_Cache_Buffer`。
* 这会占用 RAM 的 `.bss` 段。对于 C8T6 (20KB RAM) 来说，占用 1/10 的内存是可以接受的，但如果你的 RAM 非常紧张，请注意这一点。

### 5.3 数据对齐

* 底层使用了 `FLASH_ProgramWord` (32位写入)。
* 尽管驱动内部做了处理，但为了最佳性能和稳定性，建议保存的结构体或变量按 **4字节对齐** (Total size 是 4 的倍数)。

# STM32 DMA 数据搬运驱动模块使用说明

本模块提供了一个通用的 DMA 配置接口，支持 STM32 的 **P2M (外设转内存)**、**M2P (内存转外设)** 和 **M2M (内存转内存)** 三种模式。它就像一个“通用搬运工”，可以配置搬运什么数据、搬运多少、搬运到哪里。

## 1. 核心特性

* **通用性强**：一个函数 (`RUN_DMA_Config`) 搞定所有 DMA 场景（串口发送、ADC 采集、I2C 通信等）。
* **参数直观**：使用枚举（如 `RUN_DMA_WIDTH_8BIT`, `RUN_DMA_DIR_P2M`）替代了晦涩的寄存器宏定义。
* **独立控制**：提供单独的 Enable/Disable 接口，方便中途暂停或恢复传输。

## 2. 快速上手

### 2.1 典型场景 1：ADC 连续采集 (P2M)

将 ADC1 的数据自动搬运到内存数组中，实现“后台自动采集”。

**注意**：ADC1 对应的硬件通道固定为 **DMA1\_Channel1**。

**C**

```
#include "RUN_DMA.h"
#include "RUN_ADC.h" // 假设配合之前的 ADC 模块

uint16_t adc_buffer[10]; // 存放数据的数组

int main(void)
{
// 1. 初始化 ADC (确保 ADC 时钟开启)
RUN_ADC_Init();

// 2. 配置 DMA
// 通道: DMA1_Channel1 (查表得)
// 外设: ADC1的数据寄存器地址 (&ADC1->DR)
// 内存: 数组首地址
// 数量: 10个
// 方向: 外设 -> 内存 (P2M)
// 宽度: 16位 (ADC数据是12位，占用半字)
// 模式: 循环模式 (CIRCULAR) -> 采满10个自动回头覆盖，永远在更新
RUN_DMA_Config(DMA1_Channel1, (uint32_t)&ADC1->DR, (uint32_t)adc_buffer, 10, 
            RUN_DMA_DIR_P2M, RUN_DMA_WIDTH_16BIT, RUN_DMA_MODE_CIRCULAR);

// 3. 开启 DMA 通道
RUN_DMA_Enable(DMA1_Channel1);

// 4. 【关键】开启 ADC 的 DMA 请求开关 (这步不能少！)
// 告诉 ADC：“采集完数据不要放着，发个信号叫 DMA 来拿”
ADC_DMACmd(ADC1, ENABLE);

// 5. 开启 ADC 转换 (如果是软件触发)
ADC_SoftwareStartConvCmd(ADC1, ENABLE);

while (1)
{
// 此时 adc_buffer 里的数据会自动更新，直接读数组即可
}
}
```

### 2.2 典型场景 2：串口高速发送 (M2P)

将内存字符串一次性发给串口，不占用 CPU 时间。

**注意**：USART1\_TX 对应的硬件通道固定为 **DMA1\_Channel4**。

**C**

```
#include "RUN_DMA.h"
#include "RUN_UART.h"

char send_buff[] = "Hello DMA World!\r\n";

int main(void)
{
// 1. 初始化串口
RUN_uart_init(UART1_TX_PA9_RX_PA10, 115200, 0);

// 2. 配置 DMA
// 方向: 内存 -> 外设 (M2P)
// 宽度: 8位 (串口发字节)
// 模式: 单次模式 (NORMAL) -> 发完就停
RUN_DMA_Config(DMA1_Channel4, (uint32_t)&USART1->DR, (uint32_t)send_buff, sizeof(send_buff), 
            RUN_DMA_DIR_M2P, RUN_DMA_WIDTH_8BIT, RUN_DMA_MODE_NORMAL);

// 3. 【关键】开启串口的 DMA 发送请求
USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);

// 4. 启动 DMA 发送
RUN_DMA_Enable(DMA1_Channel4);

while (1)
{
// 可以在这里做其他事，发送由 DMA 负责
}
}
```

---

## 3. API 接口详解

### 3.1 核心配置 `RUN_DMA_Config`

**C**

```
void RUN_DMA_Config(DMA_Channel_TypeDef* DMAy_Channelx, 
            uint32_t periph_addr, 
            uint32_t memory_addr, 
            uint16_t buffer_size, 
            RUN_DMA_Dir_t direction, 
            RUN_DMA_Width_t width, 
            RUN_DMA_Mode_t mode);
```

* **DMAy\_Channelx**: 硬件通道地址 (如 `DMA1_Channel1`)。**必须查表确认**。
* **periph\_addr**: 外设寄存器地址。通常写为 `(uint32_t)&ADC1->DR` 或 `(uint32_t)&USART1->DR`。
* **memory\_addr**: 内存变量地址。通常是数组名 `(uint32_t)buffer`。
* **buffer\_size**: 传输的数据个数 (注意不是字节数，而是“个”数，取决于 `width`)。
* **direction**:
* `RUN_DMA_DIR_P2M`: 外设 -> 内存 (ADC, 串口接收)
* `RUN_DMA_DIR_M2P`: 内存 -> 外设 (DAC, 串口发送)
* `RUN_DMA_DIR_M2M`: 内存 -> 内存 (数组拷贝)
* **width**: 数据宽度 (8位/16位/32位)。**两端宽度必须一致**（例如 ADC 是 16位，内存数组也得是 `uint16_t`）。
* **mode**:
* `RUN_DMA_MODE_NORMAL`: 单次模式。搬运完 `buffer_size` 个数据后停止。
* `RUN_DMA_MODE_CIRCULAR`: 循环模式。搬运完后自动重置指针，从头继续搬运（适合示波器、连续采集）。

### 3.2 启动传输 `RUN_DMA_Enable`

**C**

```
void RUN_DMA_Enable(DMA_Channel_TypeDef* DMAy_Channelx);
```

* **功能**: 使能 DMA 通道。
* **注意**: 对应代码中的逻辑，它会尝试恢复传输。但在 **Normal 模式** 下，如果上次传输已完成（计数器归零），直接调用此函数**可能无效**（因为重装载值可能未更新）。若要重新开始一次新的单次传输，建议重新调用 Config 或手动设置计数器。

---

## 4. STM32 DMA1 通道速查表 (Cheat Sheet)

STM32F103 的 DMA 通道是固定的，**不能随意连接**。请根据下表选择 `DMAy_Channelx` 参数：


| **DMA1 通道** | **对应外设 (常用)**      | **典型应用** |
| ------------- | ------------------------ | ------------ |
| **Channel 1** | **ADC1**                 | ADC 自动采集 |
| **Channel 2** | SPI1\_RX, USART3\_TX     |              |
| **Channel 3** | SPI1\_TX, USART3\_RX     |              |
| **Channel 4** | **USART1\_TX**, SPI2\_RX | 串口1 发送   |
| **Channel 5** | **USART1\_RX**, SPI2\_TX | 串口1 接收   |
| **Channel 6** | USART2\_RX, I2C1\_TX     |              |
| **Channel 7** | USART2\_TX, I2C1\_RX     |              |

*(注：STM32F103 大容量型号才有 DMA2，普通应用主要用 DMA1)*

---

## 5. 注意事项 (Pitfalls)

1. **外设开关**：配置好 DMA 只是修好了路，你还需要打开外设的“发货开关”。

* ADC 需要 `ADC_DMACmd`。
* 串口 需要 `USART_DMACmd`。
* DAC 需要 `DAC_DMACmd`。
* **如果忘了这一步，DMA 永远不会启动。**

2. **地址对齐与宽度**：

* 如果 `width` 选 16位 (`RUN_DMA_WIDTH_16BIT`)，那么你的内存数组必须定义为 `uint16_t`，且地址通常要是偶数。
* 不要尝试用 8位的 DMA 宽度去搬运 16位的 ADC 数据，会导致高低位错乱。

3. **M2M 模式通道**：

* STM32F1 的内存到内存 (M2M) 模式不限制通道，**任意** 空闲的 DMA 通道都可以使用（通常用没有外设连接的通道）。

4. **死循环风险**：

* 本驱动代码中包含了 `RUN_DMA_Config`，它内部会自动处理时钟开启吗？查看代码片段，它似乎专注于寄存器配置。**建议在初始化代码中确保 `RCC_AHBPeriph_DMA1` 时钟已开启**。通常 `RUN_DMA_Config` 的完整实现会包含时钟开启，如果没包含，请手动调用 `RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);`。

# STM32 AT24C02 (EEPROM) 驱动模块使用说明

本模块用于驱动常见的 2Kbit (256字节) 串行 EEPROM 芯片 AT24C02。它封装了底层 I2C 通信细节和 EEPROM 特有的“写入周期”等待逻辑，确保数据安全写入。

## 1. 核心特性

* **基于对象设计**：定义 `RUN_AT24C02_t` 结构体作为设备句柄。这意味着你可以在同一块板子上连接多个 AT24C02（只要引脚不同或地址不同），并分别控制。
* **自动时序管理**：内部集成了 `RUN_delay_ms(5)`，自动处理 EEPROM 必须的 5ms 内部写入周期，防止写入失败。
* **依赖说明**：本模块依赖 `RUN_SoftI2C` (提供 I2C 通信) 和 `RUN_Delay` (提供 5ms 延时)。

## 2. 硬件连接示意

* **SCL/SDA**: 连接到 STM32 的任意 GPIO (需上拉电阻)。
* **A0/A1/A2**: 地址引脚。通常全部接地 (GND)，此时设备地址为 `0xA0`。
* **WP (Write Protect)**: 写保护引脚。**必须接地 (GND)** 才能写入数据；接 VCC 则只能读取。

## 3. 快速上手

### 3.1 读写一个字节

假设使用 **PB6 (SCL)** 和 **PB7 (SDA)** 连接 AT24C02。

**C**

```
#include "RUN_AT24C02.h"
#include "RUN_Delay.h" // 必须初始化延时

// 1. 定义设备句柄 (全局变量)
RUN_AT24C02_t my_eeprom;

int main(void)
{
// 2. 初始化延时模块 (如果是 STM32F1, 主频72M)
RUN_delay_init(72);

// 3. 初始化 EEPROM
// 参数: 句柄, SCL引脚, SDA引脚, 设备地址(通常0xA0)
AT24C02_Init(&my_eeprom, B6, B7, 0xA0);

// 4. 写入数据
// 在地址 0x05 处写入数据 0x66
AT24C02_WriteByte(&my_eeprom, 0x05, 0x66);

// 注意: WriteByte 内部已经包含了 5ms 延时，这里不需要额外延时

while (1)
{
// 5. 读取数据
uint8_t data = AT24C02_ReadByte(&my_eeprom, 0x05);

// 验证读取结果
if (data == 0x66) {
    // 读取成功
}

RUN_delay_ms(1000);
}
}
```

---

## 4. API 接口详解

### 4.1 初始化 `AT24C02_Init`

**C**

```
void AT24C02_Init(RUN_AT24C02_t *dev, RUN_GPIO_enum scl, RUN_GPIO_enum sda, uint8_t addr);
```

* **dev**: 指向设备结构体的指针 (如 `&my_eeprom`)。
* **scl / sda**: 连接的 GPIO 引脚枚举 (如 `B6`, `B7`)。
* **addr**: 设备 8 位地址。
* A0/A1/A2 全接地 -> `0xA0` (最常见)。
* A0 接 VCC, 其他接地 -> `0xA2`。

### 4.2 写一个字节 `AT24C02_WriteByte`

**C**

```
void AT24C02_WriteByte(RUN_AT24C02_t *dev, uint8_t word_addr, uint8_t data);
```

* **word\_addr**: 数据写入的内部地址 (范围 **0 \~ 255**)。
* **data**: 要写入的 8 位数据。
* **阻塞特性**: 函数最后会强制延时 **5ms**。这是 EEPROM 物理特性要求的“搬运电荷”时间。如果此时断电或复位，数据可能写入失败。

### 4.3 读一个字节 `AT24C02_ReadByte`

**C**

```
uint8_t AT24C02_ReadByte(RUN_AT24C02_t *dev, uint8_t word_addr);
```

* **word\_addr**: 读取地址 (0 \~ 255)。
* **返回值**: 读取到的数据。
* **特性**: 读取操作不需要 5ms 等待，速度很快。

---

## 5. 常见问题与注意事项

### 5.1 地址越界

AT24C02 的容量是 **2K bit = 256 Byte**。

* **合法地址范围**: `0x00` \~ `0xFF` (0 \~ 255)。
* 如果你传入 `0x100` (256)，由于参数是 `uint8_t`，它会溢出变成 `0x00`，导致覆盖第 0 个地址的数据。
* 如果需要更大的容量 (如 AT24C04/08/64)，需要修改驱动中的寻址逻辑 (AT24C04+ 需要处理页地址或双字节地址)。

### 5.2 写入太慢？

由于每次 `WriteByte` 都要死等 5ms：

* 写入 100 个字节需要：`100 * 5ms = 500ms` (0.5秒)。
* **优化建议**：如果需要大量写入，建议扩展驱动实现 **页写入 (Page Write)** 功能。AT24C02 支持一次写入 8 个字节，但这也只需要一次 5ms 等待，速度能提升 8 倍。*(注：当前提供的代码仅包含单字节写入)*。

### 5.3 写入失败 (读出来全是 0xFF)

1. **WP 引脚悬空或接高**：检查 PCB，确保 WP 引脚接地。
2. **上拉电阻缺失**：I2C 是开漏输出，SCL/SDA 必须接上拉电阻 (4.7kΩ)，否则无法输出高电平。
3. **地址错误**：检查芯片的 A0-A2 引脚电平是否与初始化时的 `addr` 参数匹配。

# STM32 DS18B20 温度传感器驱动模块使用说明

本模块用于驱动 Dallas/Maxim 的 DS18B20 数字温度传感器。它通过单总线 (1-Wire) 协议通信，只需要占用一个 GPIO 引脚即可读取高精度的温度数据。

## 1. 核心特性

* **极简硬件**：仅需一根数据线 (DQ) 即可通信。
* **高精度输出**：内部自动处理数据转换，直接返回 `float` 类型的摄氏度温度 (如 `25.62`)。
* **依赖说明**：本模块底层依赖 `RUN_OneWire` 模块 (负责底层时序) 和 `RUN_Delay` (负责微秒延时)。

## 2. 硬件连接示意

DS18B20 也是开漏输出设备，因此 **DQ 引脚必须接上拉电阻**。

* **VCC**: 接 3.3V 或 5V。
* **GND**: 接地。
* **DQ (Data)**: 连接到 STM32 的任意 GPIO。
* **4.7kΩ 上拉电阻**: 连接在 **DQ** 和 **VCC** 之间。
* *注意：如果你的模块 PCB 上已经集成了电阻（通常带 LED 的小模块都有），则不需要外接。如果是裸露的三脚元件，必须接电阻。*

## 3. 快速上手

### 3.1 读取温度

假设传感器连接在 **PA0** 引脚。

**C**

```
#include "RUN_DS18B20.h"
#include "RUN_Delay.h" // 必须初始化延时
#include <stdio.h>

int main(void)
{
float temperature;

// 1. 初始化延时模块
RUN_delay_init(72);

// 2. 初始化 DS18B20
// 参数: GPIO端口, GPIO引脚
RUN_DS18B20_Init(GPIOA, GPIO_Pin_0);

while (1)
{
// 3. 读取温度
// 注意：函数内部包含启动转换指令
temperature = RUN_DS18B20_GetTemp();

// 打印结果 (需重定向 printf)
printf("Current Temp: %.2f C\r\n", temperature);

// 4. 采样间隔
// DS18B20 转换一次温度最长需要 750ms (12位精度)
// 建议读取间隔大于 1秒
RUN_delay_ms(1000);
}
}
```

---

## 4. API 接口详解

### 4.1 初始化 `RUN_DS18B20_Init`

**C**

```
void RUN_DS18B20_Init(GPIO_TypeDef* port, uint16_t pin);
```

* **port / pin**: 传感器连接的 GPIO 组和引脚 (如 `GPIOA`, `GPIO_Pin_0`)。
* **功能**: 本质上是调用 `RUN_OneWire_Init`，配置引脚为推挽输出/浮空输入切换模式，并进行一次总线复位检测。

### 4.2 读取温度 `RUN_DS18B20_GetTemp`

**C**

```
float RUN_DS18B20_GetTemp(void);
```

* **返回值**: 浮点型温度值 (单位：摄氏度)。
* **逻辑流程**:

1. 复位总线 -> 跳过 ROM (0xCC) -> 发送温度转换命令 (0x44)。
2. *(注意：代码中未阻塞等待 750ms，见下方常见问题)*。
3. 复位总线 -> 跳过 ROM (0xCC) -> 发送读暂存器命令 (0xBE)。
4. 读取低字节 (LSB) 和 高字节 (MSB)。
5. 合并数据并乘以精度系数 (0.0625) 得到实际温度。

---

## 5. 常见问题与注意事项 (Pitfalls)

### 5.1 第一次读取显示 85.0°C ？

* **原因**: 85°C 是 DS18B20 上电后的**默认复位值**。
* **机制**: 传感器收到 "转换 (0x44)" 命令后，需要约 **750ms** (12位精度下) 来完成模拟量到数字量的转换。
* **代码行为**: 当前提供的 `GetTemp` 函数发送转换命令后，**没有死等 750ms**，而是直接读取。
* **现象**: 如果你读取速度非常快 (比如 10ms 一次)，你读回来的可能是**上一次**转换的结果，或者是上电默认值。
* **解决**: 确保两次调用 `GetTemp` 的间隔在 1秒以上，或者在代码中取消 `RUN_delay_ms(750)` 的注释。

### 5.2 读取到 -127°C 或 0.0°C ？

* **检查接线**:
* **0.0°C**: 通常是 DQ 线短路到地，或者上拉电阻没接，导致总线一直为低电平。
* **-127°C / 127°C**: 通常是总线断路 (一直高电平) 或 CRC 校验失败（虽然本驱动未开启CRC校验）。
* **检查 OneWire 时序**: 单总线对时序要求微秒级精准。如果在读取过程中频繁发生中断 (如 SysTick 或 串口中断)，可能会打断时序导致读取乱码。
* *高阶优化*: 在 `RUN_OneWire` 的读写字节函数中加入 `__disable_irq()` 和 `__enable_irq()` 保护临界区。

### 5.3 多传感器挂载？

当前驱动使用了 `Skip ROM (0xCC)` 指令。

* 这意味着总线上**只能挂载 1 个** DS18B20。
* 如果你挂了多个，所有传感器会同时响应，导致数据线短路冲突。
* 如需多传感器，需要修改驱动使用 `Match ROM (0x55)` 并读取每个传感器的 64位 唯一 ID。

# STM32 直流有刷电机驱动模块使用说明

本模块基于 H 桥 (H-Bridge) 原理封装了电机控制逻辑。最大的特点是采用\*\*“整车对象”\*\*的设计思路，你不需要分别去控制 4 个电机，而是直接对“小车”下达“左侧速度、右侧速度”的指令。

## 1. 核心特性

* **静音驱动**：PWM 频率默认设为 **20kHz**。该频率超出了人耳听觉范围（20Hz\~20kHz），有效消除了传统低频 PWM (如 1kHz) 带来的刺耳电流啸叫声。
* **统一量程**：速度输入范围统一为 **-10000 \~ +10000**。
* `+` : 正转 (前进)
* `-` : 反转 (后退)
* `0` : 停止
* **对象化配置**：通过结构体数组一次性配置所有电机的 PWM 通道和方向引脚。

## 2. 硬件原理

本驱动适用于常见的 H 桥驱动模块 (如 TB6612FNG, L298N 等)，每个电机需要两根信号线：

1. **PWM 引脚**：连接驱动模块的 `PWM` / `EN` 输入端 (控制速度)。
2. **DIR 引脚**：连接驱动模块的 `IN1` / `DIR` 输入端 (控制方向)。

* *注：如果你的驱动模块需要 IN1/IN2 双线控制方向，你需要将 IN2 接逻辑非门或在硬件上将本驱动的 DIR 配合另一个 GPIO 手动取反，或者本驱动仅支持单 DIR 引脚模式的驱动器。*

## 3. 快速上手 (以 2WD 小车为例)

### 3.1 定义与初始化

假设左电机用 **PA8 (TIM1\_CH1)** 和 **PC0**，右电机用 **PA9 (TIM1\_CH2)** 和 **PC1**。

**C**

```
#include "RUN_Moter_Brushed.h"

// 1. 定义小车对象 (全局变量)
RUN_Car_t my_car;

int main(void)
{
// 2. 配置小车参数
my_car.motor_count = 2; // 双轮模式

// 配置左轮 (Motor 0)
my_car.motor[0].pwm_id  = PWM_TIM1_CH1_PA8; // 速度
my_car.motor[0].dir_pin = C0;               // 方向

// 配置右轮 (Motor 1)
my_car.motor[1].pwm_id  = PWM_TIM1_CH2_PA9; // 速度
my_car.motor[1].dir_pin = C1;               // 方向

// 3. 初始化底盘 (内部自动初始化 PWM 和 GPIO)
RUN_Car_Init(&my_car);

while (1)
{
// 4. 控制移动

// 前进 (速度 5000/10000 = 50%)
RUN_Car_Set2(&my_car, 5000, 5000);
RUN_delay_ms(1000);

// 原地旋转 (左轮后退，右轮前进)
RUN_Car_Set2(&my_car, -3000, 3000);
RUN_delay_ms(1000);

// 停止
RUN_Car_Set2(&my_car, 0, 0);
RUN_delay_ms(1000);
}
}
```

---

## 4. API 接口详解

### 4.1 初始化 `RUN_Car_Init`

**C**

```
void RUN_Car_Init(RUN_Car_t* car);
```

* **功能**：遍历 `car->motor[]` 数组，自动调用 `RUN_pwm_init` (20kHz) 和 `RUN_gpio_init` (推挽输出)。
* **注意**：必须先填充结构体内容，再调用此函数。

### 4.2 双轮控制 `RUN_Car_Set2`

**C**

```
void RUN_Car_Set2(RUN_Car_t* car, int16_t speed1, int16_t speed2);
```

* **speed1**: 对应 `motor[0]` 的速度 (通常是左轮)。
* **speed2**: 对应 `motor[1]` 的速度 (通常是右轮)。
* **范围**: -10000 \~ +10000。超出范围会自动限幅。

### 4.3 四轮控制 `RUN_Car_Set4`

**C**

```
void RUN_Car_Set4(RUN_Car_t* car, int16_t speed1, int16_t speed2, int16_t speed3, int16_t speed4);
```

* **适用**: 麦克纳姆轮或 4WD 独立驱动小车。
* **参数**: 分别对应 `motor[0]` \~ `motor[3]`。

---

## 5. 常见问题与注意事项

### 5.1 电机方向反了？

* **现象**：给正速度 `5000`，电机却反转（车倒退）。
* **解决**：不要改代码逻辑！最快的方法是**对调电机接线座上的两根电源线**。或者在初始化结构体时，不要动引脚，而是去修改底层的 GPIO 电平逻辑（不推荐）。

### 5.2 低速不转 (死区问题)

* **现象**：给速度 `500` 或 `1000`，电机有电流声但不转。
* **原因**：直流电机存在“启动电压”或“死区”。由于摩擦力和电机特性，PWM 占空比低于一定值（例如 15% 即 1500）时，产生的力矩不足以克服静摩擦力。
* **建议**：在代码中设置一个“最小起转速度”，例如 `if(abs(speed) < 1500) speed = 0;`。

### 5.3 20kHz 频率说明

* 驱动默认使用 **20kHz** 是为了静音。
* **警告**：部分老旧的或低端的电机驱动模块（如部分光耦隔离型）可能因光耦响应慢而无法支持 20kHz 的高频 PWM。如果发现电机无力或发热严重，请尝试在 `RUN_Moter_Brushed.c` 的初始化函数中将 `20000` 改为 `1000` 或 `50` 测试。

# STM32 步进电机驱动模块使用说明

本模块采用 **PUL (脉冲) + DIR (方向)** 的标准控制方式。通过改变 PWM 的频率来控制电机转速，通过 GPIO 电平控制旋转方向。

## 1. 核心特性

* **频率即速度**：直接输入目标频率 (Hz)，驱动自动调整 PWM 周期。
* **正负控方向**：输入正数 (如 `1000`) 正转，输入负数 (如 `-1000`) 反转，输入 `0` 停止。
* **占空比锁定**：运行时自动锁定 50% 占空比，确保驱动器接收到标准的方波脉冲。
* **对象化封装**：支持同时控制多个步进电机。

## 2. 硬件原理与接线

本驱动适用于“脉冲+方向”类型的驱动器。

**接线方式 (以共阴极接法为例)**：

1. **PUL+ (Pulse)**: 接 STM32 的 **PWM 引脚**。
2. **DIR+ (Direction)**: 接 STM32 的 **GPIO 引脚**。
3. **PUL- / DIR-**: 接 GND。
4. **ENA+ / ENA-**: 通常**悬空不接**。

* *注：大多数驱动器在 ENA 悬空时默认为“使能/锁轴”状态。如果接了线且给了有效电平，电机反而会脱机（无力矩）。*

## 3. 快速上手

### 3.1 定义与控制

假设使用 **PA8 (TIM1\_CH1)** 控制脉冲，**PC13** 控制方向。

**C**

```
#include "RUN_Moter_Stepper.h"
#include "RUN_Delay.h"

// 1. 定义电机对象
RUN_Stepper_t my_stepper;

int main(void)
{
// 2. 配置引脚参数
my_stepper.pwm_pin = PWM_TIM1_CH1_PA8; // 脉冲脚 (必须是 PWM 支持引脚)
my_stepper.dir_pin = C13;              // 方向脚 (任意 GPIO)

// 3. 初始化电机
// 此时电机处于锁死状态 (Holding)，PWM=0
RUN_Step_Init(&my_stepper);

while (1)
{
// 4. 正转 (1000Hz)
// 假设细分设为 1600步/圈，则转速 = 1000/1600 * 60 = 37.5 RPM
RUN_Step_SetSpeed(&my_stepper, 1000);
RUN_delay_ms(2000);

// 5. 停止
RUN_Step_SetSpeed(&my_stepper, 0);
RUN_delay_ms(1000);

// 6. 反转 (2000Hz)
// 注意：如果电机带负载，直接从 0 跳到 2000 可能会堵转 (详见注意事项)
RUN_Step_SetSpeed(&my_stepper, -2000);
RUN_delay_ms(2000);
}
}
```

---

## 4. API 接口详解

### 4.1 初始化 `RUN_Step_Init`

**C**

```
void RUN_Step_Init(RUN_Stepper_t* motor);
```

* **功能**: 初始化 PWM 引脚（默认频率 1kHz，但占空比为 0，即不发脉冲）和 DIR 引脚（推挽输出）。
* **状态**: 调用后，电机通常会有电流流过（锁轴音），手拧不动，等待指令。

### 4.2 设置速度与方向 `RUN_Step_SetSpeed`

**C**

```
void RUN_Step_SetSpeed(RUN_Stepper_t* motor, int32_t freq_hz);
```

* **motor**: 电机对象指针。
* **freq\_hz**: 脉冲频率 (Hz)。
* **`> 0`**: **正转** (DIR引脚置 1, PWM 输出 50% 占空比)。
* **`< 0`**: **反转** (DIR引脚置 0, PWM 输出 50% 占空比)。
* **`= 0`**: **停止** (PWM 占空比设为 0，停止发送脉冲，电机保持锁死)。

---

## 5. 关键注意事项 (Pitfalls)

### 5.1 堵转问题 (Stalling)

* **现象**: 代码让电机以 5000Hz 运行，但电机只发出“滋滋”的高频啸叫声，轴不转动。
* **原因**: 步进电机**不能瞬间启动到高才转速**。必须有一个“加速过程” (Ramp up)。
* 例如：0Hz -> 500Hz -> 1000Hz -> ... -> 5000Hz。
* **本驱动限制**: `SetSpeed` 函数是直接设置频率的。如果在主循环中直接给一个很大的值，电机大概率会堵转。你需要自己在应用层写一个 `for` 循环来缓慢增加频率。

### 5.2 转速计算公式

步进电机的实际转速 (RPM) 取决于驱动器上的**细分设置 (Microsteps)**。

\$\$\\text{RPM} = \\frac{\\text{Frequency (Hz)} \\times 60}{\\text{StepsPerRev} \\times \\text{Microsteps}}\$\$

* **StepsPerRev**: 电机固有步数，通常 1.8° 电机一圈是 **200** 步。
* **Microsteps**: 驱动器拨码开关设定的细分 (如 1, 2, 4, 8, 16...)。
* *示例*: 驱动器设为 8 细分，电机 1.8°。
* 转一圈需要脉冲数 = **\$200 \\times 8 = 1600\$** 个。
* 如果你给 **1600Hz**，电机转速就是 **60 RPM (1圈/秒)**。

### 5.3 为什么不转？

1. **共阳/共阴接错**: 确认你的 `PUL-` 和 `DIR-` 是接地，还是 `PUL+` 和 `DIR+` 接了电源。本代码输出推挽信号，建议使用**共阴极接法** (信号线接 PUL+/DIR+，PUL-/DIR- 接地)。
2. **频率太高**: 先尝试给 **500Hz** 测试，不要一开始就给 10kHz。
3. **ENA 接线**: 拔掉 ENA 所有的线再试（悬空通常是最安全的使能状态）。

# STM32 MPU6050 6轴姿态传感器驱动说明

本模块实现了对 MPU6050 寄存器的读写操作，提供了获取原始数据以及**直接获取实际物理量**（加速度 **\$g\$**、角速度 **\$^\\circ/s\$**）的接口。

## 1. 核心特性

* **软件 I2C 通信**：不占用硬件 I2C 资源，可自定义任意引脚 (SCL/SDA)。
* **多机支持**：采用对象化设计 (`RUN_MPU6050_t`)，支持同一 MCU 挂载多个传感器。
* **物理量直读**：内置单位转换函数，可直接读取浮点型物理数据，无需手动计算。
* **默认配置**：
  * **加速度量程**：**\$\\pm 2g\$** (灵敏度高，适合检测重力)
  * **陀螺仪量程**：**\$\\pm 2000^\\circ/s\$** (量程大，防止快速旋转时数据溢出)
  * **采样率**：125Hz
  * **滤波**：5Hz 低通滤波 (减少机械震动干扰)

## 2. 硬件接线

MPU6050 使用 I2C 接口。


| **MPU6050 引脚** | **STM32 引脚 (示例)** | **说明**                                       |
| ---------------- | --------------------- | ---------------------------------------------- |
| **VCC**          | 3.3V / 5V             | 模块通常带稳压，但也支持 3.3V 直供             |
| **GND**          | GND                   | 共地                                           |
| **SCL**          | PB6 (任意GPIO)        | I2C 时钟线 (需上拉)                            |
| **SDA**          | PB7 (任意GPIO)        | I2C 数据线 (需上拉)                            |
| **AD0**          | GND                   | 地址选择。接 GND 地址为`0xD0`，接 VCC 为`0xD2` |
| **INT**          | (可选)                | 外部中断引脚，如果不使用数据准备中断可悬空     |

## 3. 快速上手

### 3.1 读取实际物理值 (推荐)

使用 `_Real` 后缀的函数，直接获取浮点数结果。

**C**

```
#include "RUN_MPU6050.h"
#include "RUN_Delay.h"
#include <stdio.h> 

// 1. 定义设备对象
RUN_MPU6050_t my_mpu;

int main(void)
{
    // 初始化延时和串口(假设已实现)
    RUN_Delay_Init(72);
    // UART_Init(...); 
  
    // 2. 初始化 MPU6050
    // 参数: 对象指针, SCL引脚, SDA引脚, 设备地址(默认0xD0)
    MPU6050_Init(&my_mpu, B6, B7, MPU_ADDR_DEFAULT);
  
    // 变量存储实际物理值 (float)
    float accX, accY, accZ;     // 单位: g
    float gyroX, gyroY, gyroZ;  // 单位: deg/s (度/秒)
    float temp;                 // 单位: ℃

    while (1)
    {
        // 3. 读取加速度 (实际值)
        MPU6050_Get_Accel_Real(&my_mpu, &accX, &accY, &accZ);

        // 4. 读取角速度 (实际值)
        MPU6050_Get_Gyro_Real(&my_mpu, &gyroX, &gyroY, &gyroZ);

        // 5. 读取温度
        temp = MPU6050_Get_Temp(&my_mpu);

        // 打印结果 (%.2f 保留两位小数)
        printf("Acc: %.2f, %.2f, %.2f g\r\n", accX, accY, accZ);
        printf("Gyro: %.2f, %.2f, %.2f dps\r\n", gyroX, gyroY, gyroZ);
        printf("Temp: %.2f C\r\n", temp);

        RUN_Delay_ms(50); // 控制采样频率
    }
}
```

---

## 4. API 接口详解

### 4.1 初始化 `MPU6050_Init`

**C**

```
void MPU6050_Init(RUN_MPU6050_t *mpu, RUN_GPIO_enum scl, RUN_GPIO_enum sda, uint8_t addr);
```

* **功能**: 初始化软件I2C，唤醒传感器，配置量程(**\$\\pm2g, \\pm2000dps\$**)。
* **addr**: 通常填 `MPU_ADDR_DEFAULT` (0xD0)。

### 4.2 获取实际物理值 (Float)

这是最常用的接口，内部已自动除以灵敏度系数。

**C**

```
// 获取加速度 (单位: g)
void MPU6050_Get_Accel_Real(RUN_MPU6050_t *mpu, float *val_x, float *val_y, float *val_z);

// 获取角速度 (单位: 度/秒)
void MPU6050_Get_Gyro_Real(RUN_MPU6050_t *mpu, float *val_x, float *val_y, float *val_z);
```

### 4.3 获取原始 ADC 值 (Int16)

如果你需要自行处理数据或追求极致速度，可使用此接口。

**C**

```
// 返回原始 16位 整数 (LSB)
void MPU6050_Get_Accel(RUN_MPU6050_t *mpu, int16_t *ax, int16_t *ay, int16_t *az);
void MPU6050_Get_Gyro (RUN_MPU6050_t *mpu, int16_t *gx, int16_t *gy, int16_t *gz);
```

### 4.4 获取温度

**C**

```
float MPU6050_Get_Temp(RUN_MPU6050_t *mpu);
```

* **返回**: 芯片内部温度 (摄氏度)。

---

## 5. 数据换算原理

驱动内部 `_Real` 函数遵循以下换算公式（基于当前配置）：

1. **加速度 (**\$\\pm 2g\$**)**:
   \$\$\\text{实际值}(g) = \\frac{\\text{原始值}}{16384.0}\$\$

   * *现象*: 平放静止时，Z轴读数应约为 **1.00g** (或 16384)。
2. **角速度 (**\$\\pm 2000^\\circ/s\$**)**:
   \$\$\\text{实际值}(^\\circ/s) = \\frac{\\text{原始值}}{16.4}\$\$

   * *现象*: 静止时，所有轴读数应接近 **0.00**。

---

## 6. 常见问题 (FAQ)

1. **读数全是 0？**
   * 检查是否调用了 `MPU6050_Init`。
   * 检查 I2C 线是否松动，或者 SCL/SDA 引脚是否填错。
2. **静止时角速度不为 0 (例如 0.5 或 -1.2)？**
   * 这是正常的 **零点漂移**。
   * 建议在程序启动静止时，连续读取 100 次求平均值作为 Offset，后续测量时减去该 Offset。
3. **如何计算角度 (Roll/Pitch/Yaw)？**
   * MPU6050 仅提供角速度和加速度。
   * 若需角度，请配合 `RUN_IMU_GetAngle` 模块使用，将 `Get_Accel_Real` 和 `Get_Gyro_Real` 的返回值传入姿态解算算法。

# STM32 W25Q64 Flash 存储驱动模块使用说明

本模块基于 `RUN_SPI` 底层驱动，封装了对 W25Q64（8M字节）外部串行 Flash 的常用操作。它处理了复杂的 SPI 时序和 Flash 特有的指令集，使用户可以像读写数组一样操作外部存储器。

## 1. 核心特性

* **硬件 SPI 加速**：基于 STM32 硬件 SPI 外设，读写速度远超模拟 SPI。
* **跨页写入处理**：自动处理 Flash 的“页编程 (Page Program)”限制，用户无需关心 256 字节的分页边界，可一次性写入任意长度数据。
* **灵活的引脚配置**：支持动态指定片选引脚 (CS) 和 SPI 总线通道。
* **标准指令集**：支持 Read ID、Sector Erase、Block Erase、Chip Erase 等标准指令。

## 2. 硬件连接与原理

W25Q64 使用标准的 **SPI 接口** (4线制)。

* **CS (片选)**：由软件控制的任意 GPIO（低电平有效）。
* **CLK/MISO/MOSI**：连接到 STM32 的硬件 SPI 引脚。
* **VCC**：3.3V (严禁接 5V)。

存储结构详解（重要）：

W25Q64 总容量 8MB (64M-bit)，地址范围 0x000000 \~ 0x7FFFFF。内部层级如下：

1. **块 (Block)**：共 128 个，每个 64KB。
2. **扇区 (Sector)**：每个块分 16 个扇区，每个 **4KB** (4096字节)。

* *注意：扇区是最小的“擦除”单位。*

3. **页 (Page)**：每个扇区分 16 页，每个 **256字节**。

* *注意：页是“写入”的基本操作单位。*

## 3. 快速上手示例

假设我们将 W25Q64 挂载在 **SPI2** 上，片选脚使用 **PB12**。

### 3.1 初始化与检测

**C**

```
#include "RUN_W25Q64.h"
#include "RUN_SPI.h"

void Main_Init(void)
{
// 1. 初始化 SPI 硬件接口 (SPI2: SCK=PB13, MISO=PB14, MOSI=PB15)
RUN_SPI_Init(RUN_SPI_2_PB13_PB14_PB15);

// 2. 初始化 W25Q64 驱动
// 参数：使用哪个SPI口, 片选端口, 片选引脚
RUN_W25Q_Init(RUN_SPI_2_PB13_PB14_PB15, GPIOB, GPIO_Pin_12);

// 3. 读取芯片 ID 验证连接
// W25Q64 的 ID 通常为 0xEF16 (厂商EF, 容量16)
uint16_t flash_id = RUN_W25Q_ReadID();

if (flash_id == 0xEF16) {
printf("W25Q64 Detected!\n");
} else {
printf("Flash Error! ID: %04X\n", flash_id);
}
}
```

### 3.2 读写操作示例

**⚠️ 警告**：Flash 的特性是 **“只能将 1 写为 0，不能将 0 写为 1”**。因此，在写入新数据之前，必须确保目标区域已被**擦除** (擦除后数据全为 0xFF)。

**C**

```
uint8_t write_buf[] = "Hello STM32 Flash!";
uint8_t read_buf[50];
uint32_t addr = 0x000000; // 从地址 0 开始

void Test_Flash_RW(void)
{
// 1. 擦除扇区 (4KB)
// 写入前必须擦除！擦除该地址所在的整个 4KB 扇区
printf("Erasing sector...\n");
RUN_W25Q_Erase_Sector(addr); 

// 2. 写入数据
// 驱动内部会自动处理“页写入”的分包逻辑
printf("Writing data...\n");
RUN_W25Q_Write(write_buf, addr, sizeof(write_buf));

// 3. 读取数据
printf("Reading data...\n");
RUN_W25Q_Read(read_buf, addr, sizeof(write_buf));

// 验证
printf("Read Content: %s\n", read_buf);
}
```

## 4. API 函数速查


| **函数名**              | **描述**             | **典型耗时**       | **注意事项**                            |
| ----------------------- | -------------------- | ------------------ | --------------------------------------- |
| `RUN_W25Q_Init`         | 初始化驱动和 CS 引脚 | 微秒级             | 需先调用`RUN_SPI_Init`                  |
| `RUN_W25Q_ReadID`       | 读取厂商和设备 ID    | 微秒级             | 用于检测芯片是否存在                    |
| `RUN_W25Q_Read`         | 读取数据             | 快 (取决于SPI频率) | 可读取任意长度，无限制                  |
| `RUN_W25Q_Write`        | 写入数据             | 3ms / 256字节      | **不会自动擦除**，需确保区域为空 (0xFF) |
| `RUN_W25Q_Erase_Sector` | **扇区擦除 (4KB)**   | **45ms**           | 最小擦除单位，传入该扇区内任一地址即可  |
| `RUN_W25Q_ChipErase`    | **整片擦除**         | **数秒 \~ 几十秒** | 极其耗时，期间 CPU 会阻塞等待           |

## 5. 常见问题 (FAQ)

### Q1: 为什么写入的数据读出来不对？

* **原因 A**：**未擦除**。如果该位置原本有数据（非 0xFF），直接写入会发生“与”逻辑错误（例如原数据 0x0F，写入 0xF0，结果变成 0x00）。请先调用 `Erase_Sector`。
* **原因 B**：**写保护**。检查硬件上的 WP 引脚是否被拉低，或者软件上 Flash 进入了掉电模式。

### Q2: 既然有 `RUN_W25Q_Write`，还需要自己分也写吗？

* **不需要**。该驱动的 `Write` 函数内部已经实现了算法：如果数据跨越了“页边界”（Page Boundary, 地址 % 256 == 0），它会自动分多次指令写入。用户只需把它当成普通的内存拷贝即可。

### Q3: 读写 Flash 时能响应中断吗？

* **可以**。SPI 通信本身是字节级的，可以被打断。但在执行 **擦除 (Erase)** 或 **写入 (Program)** 指令发送期间，建议不要有高优先级中断打断过久，否则可能导致时序错乱（虽然硬件 SPI 容错率较高）。
* *注意*：擦除函数 `Erase_Sector` 内部有 `WaitForWriteEnd()` 循环等待，这会阻塞当前线程（死等 Flash 忙完）。如果在 RTOS 中使用，建议改造为信号量等待机制。

# STM32 - 姿态解算模块说明文档

## 1. 模块简介 (Overview)

本模块 (`RUN_IMU_GetAngle`) 专注于将 6 轴传感器（三轴加速度计 + 三轴陀螺仪）的原始物理数据融合，解算出物体的 **欧拉角 (Euler Angles)**：横滚角 (Roll)、俯仰角 (Pitch) 和航向角 (Yaw)。

模块内置了两种不同量级的算法，分别适用于低算力场景和高精度场景。

* **源文件**: `RUN_IMU_GetAngle.c`
* **头文件**: `RUN_IMU_GetAngle.h`
* **依赖库**: `<math.h>`

---

## 2. 算法选型指南 (Algorithm Selection)

本模块提供两套核心算法，请根据项目需求选择：


| **特性**       | **互补滤波 (Lightweight)**       | **Mahony 四元数 (High Precision)**   |
| -------------- | -------------------------------- | ------------------------------------ |
| **函数接口**   | `RUN_CF_Update`                  | `RUN_IMU_Update`                     |
| **核心原理**   | 基于三角函数与加权平均           | 基于四元数微分方程与 PI 控制器       |
| **CPU 占用**   | **极低**(仅几次乘加与三角运算)   | **中等**(涉及多次开方与浮点矩阵运算) |
| **动态性能**   | 一般，快速运动时可能有滞后       | **优秀**，动态跟随快，无万向节锁风险 |
| **推荐场景**   | 平衡小车、简单的水平仪、低端 MCU | 四轴飞行器、云台稳像、3D 空中鼠标    |
| **Yaw 轴特性** | 纯积分 (无修正)，漂移较大        | 纯积分 (无修正)，漂移较小            |

---

## 3. 接口说明 (API Reference)

### 3.1 预备知识：输入单位

无论使用哪种算法，输入参数必须符合以下物理单位标准：

* **加速度 (ax, ay, az)**: 单位 **g** 或 **\$m/s^2\$** 均可（算法内部会自动归一化，只看方向）。
* **角速度 (gx, gy, gz)**: 单位 **度/秒 (deg/s)**。

### 3.2 方案 A：互补滤波 (Complementary Filter)

适用于资源受限或只需要粗略姿态的场景。

**C**

```
/**
 * @brief  简易互补滤波更新
 * @param  ax/ay/az  : 三轴加速度数据
 * @param  gx/gy/gz  : 三轴角速度数据 (deg/s)
 * @param  roll      : [输出] 横滚角 (deg)
 * @param  pitch     : [输出] 俯仰角 (deg)
 * @param  yaw       : [输出] 航向角 (deg)
 */
void RUN_CF_Update(float ax, float ay, float az, 
                   float gx, float gy, float gz, 
                   float *roll, float *pitch, float *yaw);
```

### 3.3 方案 B：Mahony 算法 (Mahony Filter)

适用于对精度和动态响应要求较高的场景。

**C**

```
/**
 * @brief  Mahony 四元数姿态更新
 * @param  ax/ay/az  : 三轴加速度数据
 * @param  gx/gy/gz  : 三轴角速度数据 (deg/s)
 * @param  roll      : [输出] 横滚角 (deg)
 * @param  pitch     : [输出] 俯仰角 (deg)
 * @param  yaw       : [输出] 航向角 (deg)
 */
void RUN_IMU_Update(float ax, float ay, float az, 
                    float gx, float gy, float gz, 
                    float *roll, float *pitch, float *yaw);

/**
 * @brief  重置算法状态
 * @note   将四元数复位为 (1,0,0,0) 并清空积分误差。
 * 通常在系统启动或检测到姿态发散时调用。
 */
void RUN_IMU_Reset(void);
```

---

## 4. 关键参数配置 (Configuration)

算法的准确性高度依赖采样周期的稳定性。请在文件头部根据实际情况修改宏定义。

### 4.1 全局采样时间 (必须严格匹配)

在 `RUN_IMU_GetAngle.h` 和 `RUN_IMU_GetAngle.c` 中：

**C**

```
// 单位：毫秒 (ms)
// 警告：如果你在 main 函数中每 10ms 调用一次算法，这里必须改为 10.0f
#define RUN_IMU_SAMPLE_TIME_MS  5.0f 
```

* **参数**: `CF_DT` (互补滤波用) 和 `dt` (Mahony用) 均由此导出。
* **影响**: 如果此参数与实际调用频率不符，解算出的角度速度会偏快或偏慢，导致严重误差。

### 4.2 Mahony 算法参数 (`RUN_IMU_GetAngle.c`)

**C**

```
#define Kp 10.0f   // 比例增益
#define Ki 0.005f  // 积分增益
```

* **Kp (响应速度)**: 控制加速度计修正陀螺仪的“力度”。
  * **增大**: 收敛快，但抗震动能力变差（受加速度计高频噪声影响大）。
  * **减小**: 姿态平滑，但剧烈运动后回正较慢。
* **Ki (稳态误差)**: 用于消除陀螺仪的零点漂移。通常设为极小值（0.001\~0.01），过大会导致姿态在静止时缓慢震荡。

### 4.3 互补滤波参数 (`RUN_IMU_GetAngle.c`)

**C**

```
#define CF_ALPHA    0.98f  // 互补系数 (0.0 ~ 1.0)
```

* **含义**: 98% 信任陀螺仪积分，2% 信任加速度计纠正。
* **调整**:
  * 若角度 **滞后/反应慢** -> **减小** 此值 (如 0.95)。
  * 若角度 **跳动/受震动干扰大** -> **增大** 此值 (如 0.99)。

---

## 5. 坐标系定义 (Coordinate System)

本模块假设传感器符合标准的 **右手定则**：

* **Roll (横滚)**: 绕 X 轴旋转。
* **Pitch (俯仰)**: 绕 Y 轴旋转。
* **Yaw (航向)**: 绕 Z 轴旋转。

> **注意**: 如果你的传感器安装方向不同，请在传入函数前交换 x/y/z 参数的位置，或者取反符号。

---

## 6. 常见问题 (FAQ)

**Q1: 为什么 Z 轴 (Yaw) 角度会随着时间一直漂移？**

* **原因**: 本模块是 6 轴算法。加速度计只能检测重力方向（修正 Roll 和 Pitch），无法检测水平方向的旋转。
* **原理**: Yaw 角完全依赖陀螺仪 Z 轴积分。由于传感器误差，积分误差会随时间累积。
* **解决**: 如需无漂移的 Yaw，必须使用带磁力计的 9 轴传感器及 9 轴融合算法。

**Q2: 刚上电时角度不准，需要几秒钟恢复？**

* **Mahony 算法特性**: 初始四元数默认为水平状态 `(1,0,0,0)`。如果上电时物体是倾斜的，算法需要依靠 `Kp` 参数控制加速度计慢慢将姿态“拉”回真实角度。
* **优化**: 可以在上电初始化时，先用加速度计算出的角度初始化四元数（需修改源码），或者保持静止几秒钟等待收敛。

**Q3: 静态下 Roll/Pitch 有微小跳动正常吗？**

* 正常。这是加速度计的白噪声引起的。可以通过降低 `Kp` (Mahony) 或增大 `CF_ALPHA` (互补滤波) 来抑制，但会牺牲一定的动态响应速度。


# STM32 PID 控制算法模块使用说明

## 1. 模块简介 (Overview)

本模块 (`RUN_PID`) 提供了一套轻量级、通用的 PID 控制算法库。为了满足不同的控制需求（如位置控制、速度控制），模块内部同时集成了 **位置式 (Positional)** 和 **增量式 (Incremental)** 两种核心算法。

* **源文件**: `RUN_PID.c`
* **头文件**: `RUN_PID.h`
* **依赖**: `<math.h>` (仅用于浮点数运算)

---

## 2. 算法选型指南 (Selection Guide)

在使用前，请根据你的控制目标选择正确的 PID 类型：


| **对比项**     | **位置式 PID (Positional)**                              | **增量式 PID (Incremental)**                        |
| -------------- | -------------------------------------------------------- | --------------------------------------------------- |
| **结构体名称** | `RUN_PID_Pos_t`                                          | `RUN_PID_Inc_t`                                     |
| **核心特点**   | 直接计算当前的**实际输出值**(如 PWM 占空比)。            | 计算输出的**变化量**(**\$\\Delta\$**)，累加后输出。 |
| **积分特性**   | 积分累积在历史误差中，**容易饱和**(需抗饱和处理)。       | 积分包含在增量中，**不易饱和**，误动作影响小。      |
| **典型应用**   | **直立环**(平衡车)、**角度环**(无人机)、温控、舵机位置。 | **速度环**(电机调速)、流量控制、步进电机控制。      |
| **推荐组合**   | **外环**(追求定位准确)                                   | **内环**(追求变化平滑)                              |

---

## 3. 接口说明 (API Reference)

### 3.1 位置式 PID 驱动 (Positional)

适用于**直立环、角度环、舵机位置、温控**等需要精确到达目标值的场景。

#### 3.1.1 初始化 `RUN_PID_Pos_Init`

**C**

```
void RUN_PID_Pos_Init(RUN_PID_Pos_t *pid, float kp, float ki, float kd, float max_out, float max_i);
```


| **参数**       | **说明**                                                                                                   |
| -------------- | ---------------------------------------------------------------------------------------------------------- |
| **pid**        | 位置式 PID 结构体句柄（指针）。                                                                            |
| **kp, ki, kd** | PID 三项控制系数。                                                                                         |
| **max\_out**   | 输出总限幅（绝对值）。例如 PWM 满量程 1000。                                                               |
| **max\_i**     | **积分限幅**（绝对值）。防止由于长时间无法到达目标导致积分项过大（抗饱和）。建议设为`max_out`的 30%\~50%。 |

#### 3.1.2 计算输出 `RUN_PID_Pos_Calc`

**C**

```
float RUN_PID_Pos_Calc(RUN_PID_Pos_t *pid, float target, float measure);
```


| **参数**    | **说明**                                       |
| ----------- | ---------------------------------------------- |
| **pid**     | 位置式 PID 结构体句柄。                        |
| **target**  | 目标值（Setpoint）。例如期望角度`0.0`。        |
| **measure** | 实际测量值（Feedback）。例如传感器读回的角度。 |
| **返回值**  | 计算后的控制输出量（已包含限幅处理）。         |

#### 3.1.3 复位状态 `RUN_PID_Pos_Reset`

**C**

* **功能**：将 PID 内部的误差 (`Error`)、积分项 (`Integral`) 和历史状态清零。建议在电机启动前调用。

---

### 3.2 增量式 PID 驱动 (Incremental)

适用于**速度环、步进电机、流量控制**等执行机构带积分特性，或需要平滑切换的场景。

#### 3.2.1 初始化 `RUN_PID_Inc_Init`

**C**

```
void RUN_PID_Inc_Init(RUN_PID_Inc_t *pid, float kp, float ki, float kd, float max_out);
```


| **参数**       | **说明**                        |
| -------------- | ------------------------------- |
| **pid**        | 增量式 PID 结构体句柄（指针）。 |
| **kp, ki, kd** | PID 三项控制系数。              |
| **max\_out**   | 输出总限幅（绝对值）。          |

> **注意**：增量式算法天然具有抗积分饱和特性，因此**不需要**`max_i` 积分限幅参数。

#### 3.2.2 计算输出 `RUN_PID_Inc_Calc`

**C**

```
float RUN_PID_Inc_Calc(RUN_PID_Inc_t *pid, float target, float measure);
```


| **参数**    | **说明**                                                                          |
| ----------- | --------------------------------------------------------------------------------- |
| **pid**     | 增量式 PID 结构体句柄。                                                           |
| **target**  | 目标值。例如期望速度。                                                            |
| **measure** | 实际测量值。例如编码器测得的速度。                                                |
| **返回值**  | 最终控制量。**注意**：此函数返回的是自动累加后的最终值，可直接赋值给 PWM 寄存器。 |

#### 3.2.3 复位状态 `RUN_PID_Inc_Reset`

**C**

```
void RUN_PID_Inc_Reset(RUN_PID_Inc_t *pid);
```

* **功能**：清除最近三次的历史误差记录和当前累计输出值。

## 4. 快速集成示例 (Quick Start)

以下代码展示了如何在 STM32 的定时器中断（推荐 5ms 或 10ms）中，结合 **MPU6050** 实现一个简单的 **串级 PID 控制**（角度环 + 速度环）。

**C**

```
#include "RUN_PID.h"
#include "RUN_IMU_GetAngle.h" // 假设你有姿态解算库

// 1. 定义 PID 对象
RUN_PID_Pos_t PID_Angle;  // 外环：角度环 (位置式)
RUN_PID_Inc_t PID_Speed;  // 内环：速度环 (增量式)

// 2. 初始化 (在 main 函数中调用)
void System_Init(void)
{
    // 角度环: Kp=30, Ki=0, Kd=1.5
    // 输出限幅: 100 (期望速度的最大值)
    // 积分限幅: 0 (直立环通常不需要积分，纯 PD 即可)
    RUN_PID_Pos_Init(&PID_Angle, 30.0f, 0.0f, 1.5f, 100.0f, 0.0f);
  
    // 速度环: Kp=2.0, Ki=0.1, Kd=0
    // 输出限幅: 1000 (PWM 满占空比)
    RUN_PID_Inc_Init(&PID_Speed, 2.0f, 0.1f, 0.0f, 1000.0f);
}

// 3. 定时器中断服务函数 (例如 5ms 一次)
void TIMx_IRQHandler(void)
{
    float current_angle;
    float current_speed;
    float target_speed;
    float final_pwm;

    // ... 清除中断标志 ...

    // A. 获取数据
    current_angle = Get_Pitch_Angle(); // 来自 MPU6050
    current_speed = Get_Encoder_Speed(); // 来自编码器

    // B. 外环计算 (角度环)
    // 目标: 0度 (平衡) | 测量: 当前角度 | 输出: 期望速度
    target_speed = RUN_PID_Pos_Calc(&PID_Angle, 0.0f, current_angle);

    // C. 内环计算 (速度环)
    // 目标: 外环的输出 | 测量: 当前速度 | 输出: 电机 PWM
    final_pwm = RUN_PID_Inc_Calc(&PID_Speed, target_speed, current_speed);

    // D. 执行控制
    Motor_Set_PWM((int16_t)final_pwm);
}
```

---

## 5. 参数整定口诀 (Tuning Guide)

PID 调参是工程中最大的难点。对于平衡小车或类似系统，请严格按照以下步骤进行：

### 5.1 直立环 (位置式 PD)

> **目标**：让小车受到推力后能迅速回正，且不大幅震荡。

1. **确定极性**：先给一个很小的 Kp，观察轮子转动方向。车向前倒，轮子必须向前加速跑（去接住车身）。如果反了，车会瞬间加速倒地，此时需将 Kp 设为负数。
2. **调 Kp (比例)**：
   * Ki = 0, Kd = 0。
   * 逐渐增大 Kp，直到车开始出现 **低频大幅度震荡** (像不倒翁一样大幅摇摆)。
   * 此时记下 Kp 值，乘以 0.6 作为基础 Kp。
3. **调 Kd (微分)**：
   * 保持 Kp 不变。
   * 逐渐增大 Kd。你会发现车的震荡频率变快、幅度变小，感觉车身变“硬”了。
   * 继续增大，直到出现 **高频抖动** (电机发出滋滋声)，此时稍微回调一点 Kd。

### 5.2 速度环 (增量式 PI)

> **目标**：消除静差，让车在保持直立的同时，整体速度趋于 0 (不乱跑)。

1. **调 Kp**：增大 Kp，直到车能大概跟随给定的速度。
2. **调 Ki**：速度环必须有积分项才能消除静差。给一点点 Ki (通常是 Kp 的 1/20 左右)，让车在受到持续外力推的时候，能产生抵抗力。

### 5.3 常见现象诊断


| **现象**               | **原因分析**     | **解决方案**                 |
| ---------------------- | ---------------- | ---------------------------- |
| **高频震荡 (滋滋响)**  | D 参数太大       | 减小 Kd                      |
| **大幅度低频晃动**     | P 太大 或 D 太小 | 减小 Kp 或 增大 Kd           |
| **静态下很难回中**     | I 参数太小       | 稍微增大 Ki                  |
| **动作迟缓，这就倒了** | P 太小           | 增大 Kp                      |
| **超调严重 (冲过头)**  | 积分饱和         | 减小`MaxI`(位置式) 或减小 Ki |

---

## 6. 注意事项 (Notes)

1. **采样周期恒定**：PID 是基于时间的算法。务必确保 `Calc` 函数是在定时器中断中以 **固定频率** 调用的（例如每 5ms 或 10ms）。如果在 `while(1)` 里跑，由于循环时间不固定，微分项 (D) 会计算错误，导致系统不稳定。
2. **单位统一**：虽然 PID 系数没有单位，但输入输出的物理量级要心中有数。例如角度是 -90\~90，PWM 是 0\~1000，这里的 Kp 大概就是 10\~50 的数量级。如果 PWM 是 0\~100，Kp 就要相应缩小 10 倍。
3. **安全第一**：调试电机前，**务必限制 PWM 最大值**，或把车架空，防止参数配错导致电机全速飞转伤人。

# STM32 字符串解析模块说明文档

## 1. 模块简介 (Overview)

`RUN_Str` 是一个轻量级的字符串处理工具箱，主要用于从复杂的指令字符串中批量提取数值。

* **适用场景**：串口指令解析（如 `"POS: 100, 200"`）、参数配置文件读取、G代码风格指令解析。
* **核心优势**：
  * **零依赖**：仅依赖 `strlen`，无需链接庞大的标准库格式化函数。
  * **高容错**：自动忽略非数字字符（逗号、空格、冒号等均视为分隔符）。
  * **混合解析**：支持负数、小数（浮点模式），支持省略整数位的写法（如 `.5`）。

## 2. 接口说明 (API Reference)

### 2.1 批量提取整数 `RUN_Str_GetIntArray`

从字符串中扫描并提取所有整数（`int32_t`）。

**C**

```
int RUN_Str_GetIntArray(const char* str, int32_t* out_buff, int max_len);
```


| **参数**      | **说明**                                                   |
| ------------- | ---------------------------------------------------------- |
| **str**       | 输入的源字符串。例如`"Servo: 1, 90, -45"`。                |
| **out\_buff** | 用于存储提取结果的整数数组缓冲区。                         |
| **max\_len**  | 数组的最大容量。当提取数量达到此限制时停止，防止内存越界。 |
| **返回值**    | **(int)**实际成功提取到的数字个数。                        |

> **特性说明**：
>
> * 支持负号（如 `-100`）。
> * 连续的分隔符会被忽略（如 `10,,,20` 等同于 `10, 20`）。
> * 小数点会被视为分隔符（如 `12.5` 会被解析为 `12` 和 `5` 两个整数）。

---

### 2.2 批量提取浮点数 `RUN_Str_GetFloatArray`

从字符串中扫描并提取所有浮点数（`float`）。

**C**

```
int RUN_Str_GetFloatArray(const char* str, float* out_buff, int max_len);
```


| **参数**      | **说明**                                     |
| ------------- | -------------------------------------------- |
| **str**       | 输入的源字符串。例如`"PID: 1.5, 0.02, 10"`。 |
| **out\_buff** | 用于存储提取结果的浮点数组缓冲区。           |
| **max\_len**  | 数组的最大容量。                             |
| **返回值**    | **(int)**实际成功提取到的数字个数。          |

> **特性说明**：
>
> * 完美支持整数输入（如 `10` 会被解析为 `10.0`）。
> * 支持省略整数部分的小数写法（如 `.5` 会被解析为 `0.5`）。
> * 支持负小数（如 `-0.05`）。

---

## 3. 使用示例 (Examples)

### 3.1 场景：解析舵机角度控制指令 (整数)

假设上位机发送指令：`"SET_POS: 90, 45, -30"`，分别对应 ID1, ID2, ID3 号舵机。

**C**

```
#include "RUN_Str.h"
#include <stdio.h>

void Parse_Servo_Cmd(char *rx_buffer)
{
    int32_t angles[3]; // 准备接收3个角度
    int count;

    // 调用解析
    count = RUN_Str_GetIntArray(rx_buffer, angles, 3);

    // 校验是否提取到了足够的参数
    if (count == 3)
    {
        printf("Servo 1: %d\n", angles[0]); // 90
        printf("Servo 2: %d\n", angles[1]); // 45
        printf("Servo 3: %d\n", angles[2]); // -30
      
        // 执行电机控制...
        // Motor_Set(1, angles[0]);
    }
    else
    {
        printf("Error: Invalid Command Format\n");
    }
}
```

### 3.2 场景：解析 PID 参数 (浮点数)

假设用于在线调参，发送指令风格比较随意，例如：`"pid p=12.5 i=0.05 d=8"`。

**C**

```
#include "RUN_Str.h"

// 全局 PID 参数结构体
typedef struct { float kp, ki, kd; } PID_t;
PID_t MyPID;

void Parse_PID_Cmd(char *rx_buffer)
{
    float params[3];
    int count;

    // 即使指令中有 "p=", "i=" 等字母，函数会自动跳过它们寻找数字
    count = RUN_Str_GetFloatArray(rx_buffer, params, 3);

    if (count >= 3)
    {
        MyPID.kp = params[0]; // 12.5
        MyPID.ki = params[1]; // 0.05
        MyPID.kd = params[2]; // 8.0
      
        // Update_PID_Settings();
    }
}
```

---

## 4. 算法逻辑图解

为了让你更清楚代码内部是如何工作的，这里简述其状态机逻辑（以整数解析为例）：

1. **扫描字符**：逐个读取字符串。
2. **判断类型**：
   * 如果是 **数字 ('0'-'9')**：标记 `is_parsing = 1`，将当前数字累加到临时变量 (`val = val*10 + new`)。
   * 如果是 **负号 ('-')**：且当前不在解析数字中，标记 `is_neg = 1`。
   * 如果是 **其他字符** (空格, 逗号, 字母)：
     * 检查 `is_parsing`：如果之前正在解析数字，说明数字结束了 -> **保存结果到数组**，重置临时变量。
     * 重置负号标记（除非紧接着遇到负号）。

这种逻辑使得 `10,20`、`10 20`、`Val:10/20` 都能被正确解析为 `{10, 20}`。
