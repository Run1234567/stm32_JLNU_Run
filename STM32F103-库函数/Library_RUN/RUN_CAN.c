#include "RUN_CAN.h"

// 接收缓冲区
uint8_t CAN_Rx_Buffer[8];
uint8_t CAN_Rx_Flag = 0;

// ===============================================================================
// 核心配置表 (参考 RUN_UART 的查表风格)
// ===============================================================================
const can_info_t can_cfg[CAN_PIN_MAX] = {
    // 方案0: 默认引脚 (无重映射)
    {CAN1, RCC_APB1Periph_CAN1, GPIOA, GPIO_Pin_12, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_11, RCC_APB2Periph_GPIOA, 0},
    // 方案1: PB8/PB9 (部分重映射)
    {CAN1, RCC_APB1Periph_CAN1, GPIOB, GPIO_Pin_9,  RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_8,  RCC_APB2Periph_GPIOB, GPIO_Remap1_CAN1},
    // 方案2: PD0/PD1 (完全重映射)
    {CAN1, RCC_APB1Periph_CAN1, GPIOD, GPIO_Pin_1,  RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_0,  RCC_APB2Periph_GPIOD, GPIO_Remap2_CAN1}
};

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      CAN 模块初始化
// 参数说明      can_pin         引脚方案选择
// 参数说明      baud_rate       波特率 (bps)
// 参数说明      can_mode        回环或正常模式
// 参数说明      filter_mode     过滤器工作模式
// 参数说明      f_id            过滤器基准 ID
// 参数说明      f_mask          过滤器掩码 (仅掩码模式有效，列表模式时作为第二个 ID)
// 返回参数      void
// 使用示例      RUN_can_init(CAN1_RX_PB8_TX_PB9, 500000, RUN_CAN_MODE_LOOPBACK, RUN_CAN_FILTER_ALL, 0, 0);
//-------------------------------------------------------------------------------------------------------------------
void RUN_can_init(CAN_PIN_enum can_pin, uint32 baud_rate, RUN_CAN_MODE_enum can_mode, RUN_CAN_FILTER_enum filter_mode, uint32 f_id, uint32 f_mask)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    CAN_InitTypeDef CAN_InitStructure;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    if (can_pin >= CAN_PIN_MAX) return;

    // 1. 时钟配置
    RCC_APB2PeriphClockCmd(can_cfg[can_pin].tx_rcc | can_cfg[can_pin].rx_rcc | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(can_cfg[can_pin].can_rcc, ENABLE);

    // 2. 引脚重映射
    if (can_cfg[can_pin].remap != 0) {
        GPIO_PinRemapConfig(can_cfg[can_pin].remap, ENABLE);
    }

    // 3. GPIO 引脚配置
    GPIO_InitStructure.GPIO_Pin = can_cfg[can_pin].tx_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(can_cfg[can_pin].tx_port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = can_cfg[can_pin].rx_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(can_cfg[can_pin].rx_port, &GPIO_InitStructure);

    // 4. CAN 控制器初始化
    CAN_DeInit(can_cfg[can_pin].can_base);
    CAN_InitStructure.CAN_TTCM = DISABLE;
    CAN_InitStructure.CAN_ABOM = ENABLE;
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = DISABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = DISABLE;
    CAN_InitStructure.CAN_Mode = (uint8_t)can_mode;

    // 波特率计算 (APB1 时钟默认为 36MHz)
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS2_4tq;
    CAN_InitStructure.CAN_Prescaler = 36000000 / 8 / baud_rate; 
    CAN_Init(can_cfg[can_pin].can_base, &CAN_InitStructure);

    // 5. 过滤器逻辑配置
    CAN_FilterInitStructure.CAN_FilterNumber = 0;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;

    if (filter_mode == RUN_CAN_FILTER_ALL) {
        CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
        CAN_FilterInitStructure.CAN_FilterIdHigh = 0;
        CAN_FilterInitStructure.CAN_FilterIdLow = 0;
        CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0;
        CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0;
    } 
    else if (filter_mode == RUN_CAN_FILTER_MASK) {
        CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
        CAN_FilterInitStructure.CAN_FilterIdHigh = (f_id << 5); 
        CAN_FilterInitStructure.CAN_FilterIdLow = 0;
        CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (f_mask << 5);
        CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0;
    }
    else { // 列表模式
        CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdList;
        CAN_FilterInitStructure.CAN_FilterIdHigh = (f_id << 5);
        CAN_FilterInitStructure.CAN_FilterIdLow = 0;
        CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (f_mask << 5); // 存入第二个 ID
        CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0;
    }
    CAN_FilterInit(&CAN_FilterInitStructure);

    // 6. 中断配置
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    CAN_ITConfig(can_cfg[can_pin].can_base, CAN_IT_FMP0, ENABLE);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      CAN 报文发送
// 返回参数      uint8_t         发送成功返回 1，失败返回 0
//-------------------------------------------------------------------------------------------------------------------
uint8_t RUN_can_send_msg(CAN_PIN_enum can_pin, uint32 id, uint8* msg, uint8 len)
{
    CanTxMsg TxMessage;
    uint8_t mbox;
    uint16_t timeout = 0;

    TxMessage.StdId = id;
    TxMessage.IDE = CAN_Id_Standard;
    TxMessage.RTR = CAN_RTR_Data;
    TxMessage.DLC = len;
    for(uint8 i=0; i<len; i++) TxMessage.Data[i] = msg[i];

    mbox = CAN_Transmit(can_cfg[can_pin].can_base, &TxMessage);
    
    // 等待发送完成
    while((CAN_TransmitStatus(can_cfg[can_pin].can_base, mbox) != CAN_TxStatus_Ok) && (timeout < 0xFFFF)) timeout++;
    
    return (timeout < 0xFFFF);
}

// 中断函数 (固定关联 CAN1 FIFO0)
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    CanRxMsg RxMessage;
    if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET) {
        CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
        for (int i = 0; i < RxMessage.DLC; i++) {
            CAN_Rx_Buffer[i] = RxMessage.Data[i];
        }
        CAN_Rx_Flag = 1;
        CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    }
}