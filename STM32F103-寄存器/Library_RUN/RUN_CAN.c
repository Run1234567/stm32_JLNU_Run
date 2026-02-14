#include "RUN_CAN.h"

// ===============================================================================
// 变量定义
// ===============================================================================
 uint8_t CAN_Rx_Buffer[8];
volatile uint8_t CAN_Rx_Flag = 0;

// ===============================================================================
// 核心配置表
// 注意：Remap 字段必须填入 AFIO_MAPR 寄存器的实际对应值
// ===============================================================================
const can_info_t can_cfg[CAN_PIN_MAX] = {
    // 方案0: PA11/PA12 (默认, Remap=0)
    {CAN1, 0, GPIOA, GPIO_Pin_12, 0, GPIOA, GPIO_Pin_11, 0, 0}, 
    
    // 方案1: PB9/PB8 (Remap=2, AFIO_MAPR_CAN_REMAP_REMAP1 = 0x00004000)
    {CAN1, 0, GPIOB, GPIO_Pin_9,  0, GPIOB, GPIO_Pin_8,  0, 0x00004000}, 
    
    // 方案2: PD1/PD0 (Remap=3, AFIO_MAPR_CAN_REMAP_REMAP2 = 0x00006000)
    {CAN1, 0, GPIOD, GPIO_Pin_1,  0, GPIOD, GPIO_Pin_0,  0, 0x00006000}  
};

// 内部辅助函数：获取引脚编号 (支持 0x1000 格式转为 12)
static uint8_t CAN_GetPinNumber(uint16_t pin_def) {
    uint8_t index = 0;
    // 如果值大于15，说明是 GPIO_Pin_x 宏 (如 0x1000)，需要转换
    if (pin_def > 15) { 
        while (((pin_def >> index) & 0x01) == 0 && index < 16) {
            index++;
        }
    } else {
        index = (uint8_t)pin_def;
    }
    return index;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      CAN 模块初始化 (寄存器版 - 修正健壮版)
//-------------------------------------------------------------------------------------------------------------------
void RUN_can_init(CAN_PIN_enum can_pin, uint32 baud_rate, RUN_CAN_MODE_enum can_mode, RUN_CAN_FILTER_enum filter_mode, uint32 f_id, uint32 f_mask)
{
    uint32_t wait_count = 0;
    
    if (can_pin >= CAN_PIN_MAX) return;

    // 1. 开启外设时钟
    // 开启 AFIO 和 CAN1 时钟
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; 
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
    
    // 软件复位 CAN1 (确保从干净的状态开始)
    RCC->APB1RSTR |= RCC_APB1RSTR_CAN1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_CAN1RST;

    // 开启 GPIO 时钟
    if (can_cfg[can_pin].tx_port == GPIOA) RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    else if (can_cfg[can_pin].tx_port == GPIOB) RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    else if (can_cfg[can_pin].tx_port == GPIOD) RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;

    // 2. 引脚重映射配置
    AFIO->MAPR &= ~AFIO_MAPR_CAN_REMAP;    // 清除旧配置
    AFIO->MAPR |= can_cfg[can_pin].remap;  // 写入新配置 (0, 0x4000, 或 0x6000)

    // 3. GPIO 引脚配置 (获取实际引脚索引 0-15)
    uint8_t tx_idx = CAN_GetPinNumber(can_cfg[can_pin].tx_pin);
    uint8_t rx_idx = CAN_GetPinNumber(can_cfg[can_pin].rx_pin);
    GPIO_TypeDef* port = can_cfg[can_pin].tx_port; // 假设 TX/RX 同端口

    // --- TX 配置: 复用推挽 (AF_PP), 50MHz ---
    // MODE=11(50MHz), CNF=10(Alt Push-Pull) -> 0xB
    if(tx_idx < 8) {
        port->CRL &= ~(0xF << (tx_idx * 4));
        port->CRL |= (0xB << (tx_idx * 4)); 
    } else {
        port->CRH &= ~(0xF << ((tx_idx - 8) * 4));
        port->CRH |= (0xB << ((tx_idx - 8) * 4));
    }

    // --- RX 配置: 上拉输入 (IPU) ---
    // MODE=00(Input), CNF=10(Pull-up/down) -> 0x8
    if(rx_idx < 8) {
        port->CRL &= ~(0xF << (rx_idx * 4));
        port->CRL |= (0x8 << (rx_idx * 4)); 
    } else {
        port->CRH &= ~(0xF << ((rx_idx - 8) * 4));
        port->CRH |= (0x8 << ((rx_idx - 8) * 4));
    }
    port->ODR |= (1 << rx_idx); // ODR=1 设置为上拉

    // 4. CAN 进入初始化模式
    CAN1->MCR &= ~CAN_MCR_SLEEP; // 退出睡眠
    CAN1->MCR |= CAN_MCR_INRQ;   // 请求初始化
    
    wait_count = 0;
    while(((CAN1->MSR & CAN_MSR_INAK) == 0) && (wait_count < 0xFFFF)) {
        wait_count++; // 等待 INAK 置位
    }

    CAN1->MCR |= CAN_MCR_ABOM;   // 自动离线管理 (Bus-Off 自动恢复)

    // 5. 波特率计算 (APB1 = 36MHz, TS1=3, TS2=4, SJW=1, Total=8tq)
    // 36M / 8 / BaudRate = Prescaler
    uint32_t brp = 36000000 / 8 / baud_rate;
    if(brp > 0) brp = brp - 1; // 寄存器值 = 实际值 - 1
    
    CAN1->BTR = 0;
    CAN1->BTR |= (uint32_t)can_mode << 30; // 模式: 0=Normal, 1=LoopBack
    CAN1->BTR |= (0 << 24);                // SJW = 1tq (0)
    CAN1->BTR |= (3 << 20);                // TS2 = 4tq (3)
    CAN1->BTR |= (2 << 16);                // TS1 = 3tq (2)
    CAN1->BTR |= brp;                      // BRP Prescaler

    // 6. 过滤器配置
    CAN1->FMR |= CAN_FMR_FINIT; // 进入过滤器初始化模式
    CAN1->FA1R &= ~(1 << 0);    // 禁用过滤器 0

    CAN1->FS1R |= (1 << 0);     // 32位 宽模式
    
    // 过滤器寄存器说明: FR1(ID), FR2(Mask/ID2)
    // 32位模式下映射: [STID 31:21] [EXID 20:3] [IDE 2] [RTR 1] [0]
    // 传入的 f_id 为标准 ID，需要左移 21 位
    
    if (filter_mode == RUN_CAN_FILTER_ALL) {
        CAN1->FM1R &= ~(1 << 0); // 掩码模式
        CAN1->sFilterRegister[0].FR1 = 0;
        CAN1->sFilterRegister[0].FR2 = 0;
    } else if (filter_mode == RUN_CAN_FILTER_MASK) {
        CAN1->FM1R &= ~(1 << 0); // 掩码模式
        CAN1->sFilterRegister[0].FR1 = (f_id << 21);   
        CAN1->sFilterRegister[0].FR2 = (f_mask << 21);
    } else { // 列表模式
        CAN1->FM1R |= (1 << 0);  // 列表模式
        CAN1->sFilterRegister[0].FR1 = (f_id << 21);
        CAN1->sFilterRegister[0].FR2 = (f_mask << 21); // f_mask 作为第二个ID
    }

    CAN1->FA1R |= (1 << 0);      // 激活过滤器 0
    CAN1->FMR &= ~CAN_FMR_FINIT; // 退出过滤器初始化模式

    // 7. 退出 CAN 初始化模式
    CAN1->MCR &= ~CAN_MCR_INRQ;
    wait_count = 0;
    while((CAN1->MSR & CAN_MSR_INAK) && (wait_count < 0xFFFF)) {
        wait_count++; // 等待 INAK 清零
    }

    // 8. 中断配置
    CAN1->IER |= CAN_IER_FMPIE0; // 允许 FIFO0 接收中断
    
    // 使用标准库函数配置 NVIC (兼容性更好)
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      CAN 报文发送 (寄存器版)
//-------------------------------------------------------------------------------------------------------------------
uint8_t RUN_can_send_msg(CAN_PIN_enum can_pin, uint32 id, uint8* msg, uint8 len)
{
    uint8_t mbox;
    uint16_t timeout = 0;

    // 1. 寻找空闲邮箱
    if ((CAN1->TSR & CAN_TSR_TME0) == CAN_TSR_TME0) mbox = 0;
    else if ((CAN1->TSR & CAN_TSR_TME1) == CAN_TSR_TME1) mbox = 1;
    else if ((CAN1->TSR & CAN_TSR_TME2) == CAN_TSR_TME2) mbox = 2;
    else return 0; // 无空闲邮箱

    // 2. 写入标识符 (标准ID STID 位于 bit 31:21)
    CAN1->sTxMailBox[mbox].TIR = (id << 21); 

    // 3. 写入长度
    CAN1->sTxMailBox[mbox].TDTR &= ~CAN_TDT0R_DLC;
    CAN1->sTxMailBox[mbox].TDTR |= (len & 0x0F);

    // 4. 写入数据
    CAN1->sTxMailBox[mbox].TDLR = (((uint32_t)msg[3] << 24) | ((uint32_t)msg[2] << 16) | 
                                   ((uint32_t)msg[1] << 8)  | (uint32_t)msg[0]);
    CAN1->sTxMailBox[mbox].TDHR = (((uint32_t)msg[7] << 24) | ((uint32_t)msg[6] << 16) | 
                                   ((uint32_t)msg[5] << 8)  | (uint32_t)msg[4]);

    // 5. 请求发送 (TXRQ)
    CAN1->sTxMailBox[mbox].TIR |= CAN_TI0R_TXRQ;

    // 6. 等待完成
    uint32_t txok_mask = (CAN_TSR_TXOK0 << (mbox * 8));
    while ((!(CAN1->TSR & txok_mask)) && (timeout < 0xFFFF)) {
        timeout++;
    }

    return (timeout < 0xFFFF) ? 1 : 0;
}

//-------------------------------------------------------------------------------------------------------------------
// 中断函数 (寄存器版)
//-------------------------------------------------------------------------------------------------------------------
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    if (CAN1->RF0R & CAN_RF0R_FMP0) {
        // 读取数据
        uint32_t data_low = CAN1->sFIFOMailBox[0].RDLR;
        uint32_t data_high = CAN1->sFIFOMailBox[0].RDHR;
        
        CAN_Rx_Buffer[0] = (uint8_t)(data_low);
        CAN_Rx_Buffer[1] = (uint8_t)(data_low >> 8);
        CAN_Rx_Buffer[2] = (uint8_t)(data_low >> 16);
        CAN_Rx_Buffer[3] = (uint8_t)(data_low >> 24);

        CAN_Rx_Buffer[4] = (uint8_t)(data_high);
        CAN_Rx_Buffer[5] = (uint8_t)(data_high >> 8);
        CAN_Rx_Buffer[6] = (uint8_t)(data_high >> 16);
        CAN_Rx_Buffer[7] = (uint8_t)(data_high >> 24);

        CAN_Rx_Flag = 1;

        // 释放 FIFO0
        CAN1->RF0R |= CAN_RF0R_RFOM0;
    }
}