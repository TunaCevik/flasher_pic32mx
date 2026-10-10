#include "init/init_usart.h"

void UART5_Init(uint32_t baudrate)
{
    // 1. Enable UART5 peripheral clock on the APB1 bus
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART5);

    // 2. Configure USART5 parameters using Low-Layer structures
    LL_USART_InitTypeDef USART_InitStruct = {0};

    USART_InitStruct.BaudRate            = baudrate;
    USART_InitStruct.DataWidth           = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits            = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity              = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection   = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling        = LL_USART_OVERSAMPLING_16;

    LL_USART_Init(UART5, &USART_InitStruct);

    // 3. Let the DMA1 Stream0/Channel4 RX request actually fire (previously
    // missing: DMA was configured but the USART never told to feed it).
    LL_USART_EnableDMAReq_RX(UART5);

    // 4. Enable the USART peripheral
    LL_USART_Enable(UART5);
}

void UART5_SendChar(char c)
{
    while (!LL_USART_IsActiveFlag_TXE(UART5));
    LL_USART_TransmitData8(UART5, (uint8_t)c);
}

void UART5_SendString(const char *str)
{
    while (*str) {
        UART5_SendChar(*str++);
    }
}
