#include "init/init_dma.h"

void UART_DMA_DoubleBuffer_Init(DMA_TypeDef *DMAx, uint32_t StreamIndex, uint32_t periph_addr, uint32_t mem1_addr, uint32_t mem2_addr)
{
    // 1. Enable the DMA1 controller's own peripheral clock -- without this
    // every register write below lands on an unclocked block and has no
    // effect (previously missing entirely; GPIOC/GPIOD/UART5 each enable
    // their own clock in their own init function, DMA1 never did).
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

    // 2. Ensure the stream is disabled before configuration
    LL_DMA_DisableStream(DMAx, StreamIndex);
    while (LL_DMA_IsEnabledStream(DMAx, StreamIndex));

    // 3. Populate the LL initialization structure with exact F4 LL field names
    LL_DMA_InitTypeDef DMA_InitStruct = {0};

    // Channel 4 is UART5_RX's actual DMA request line on DMA1 Stream0 (RM0390
    // DMA request mapping table) -- dsl/init_dma.mkd's CHSEL=000 (channel 0)
    // does not match this; kept at channel 4 since channel 0 would never see
    // UART5's DMA requests at all. Flagged for confirmation.
    DMA_InitStruct.Channel                 = LL_DMA_CHANNEL_4;
    DMA_InitStruct.PeriphOrM2MSrcAddress   = periph_addr;
    DMA_InitStruct.MemoryOrM2MDstAddress   = mem1_addr;
    DMA_InitStruct.Direction               = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    DMA_InitStruct.Mode                    = LL_DMA_MODE_CIRCULAR;
    DMA_InitStruct.PeriphOrM2MSrcDataSize  = LL_DMA_PDATAALIGN_BYTE;
    DMA_InitStruct.MemoryOrM2MDstDataSize  = LL_DMA_MDATAALIGN_BYTE;
    DMA_InitStruct.PeriphOrM2MSrcIncMode   = LL_DMA_PERIPH_NOINCREMENT;
    DMA_InitStruct.MemoryOrM2MDstIncMode   = LL_DMA_MEMORY_INCREMENT;
    DMA_InitStruct.NbData                  = 128;
    DMA_InitStruct.Priority                = LL_DMA_PRIORITY_HIGH;
    DMA_InitStruct.FIFOMode                = LL_DMA_FIFOMODE_ENABLE;
    DMA_InitStruct.FIFOThreshold           = LL_DMA_FIFOTHRESHOLD_1_2;
    DMA_InitStruct.MemBurst                = LL_DMA_MBURST_SINGLE;
    DMA_InitStruct.PeriphBurst             = LL_DMA_PBURST_SINGLE;

    LL_DMA_Init(DMAx, StreamIndex, &DMA_InitStruct);

    // 4. dsl/init_dma.mkd steps 5-6: real double buffering. LL_DMA_InitTypeDef
    // has no field for the second buffer or for DBM, so both are set directly
    // -- M1AR must be programmed before DBM is enabled. Enabling DBM implies
    // circular mode in hardware; LL_DMA_MODE_CIRCULAR above already set CIRC
    // too, so behavior matches the dsl spec either way.
    LL_DMA_SetMemory1Address(DMAx, StreamIndex, mem2_addr);
    LL_DMA_EnableDoubleBufferMode(DMAx, StreamIndex);

    // 5. Configure Transfer Complete and Transfer Error interrupts
    LL_DMA_EnableIT_TC(DMAx, StreamIndex);
    LL_DMA_EnableIT_TE(DMAx, StreamIndex);

    // 6. Enable the DMA stream
    LL_DMA_EnableStream(DMAx, StreamIndex);
}
