#ifndef INIT_DMA_H
#define INIT_DMA_H

#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_bus.h"

void UART_DMA_DoubleBuffer_Init(DMA_TypeDef *DMAx, uint32_t StreamIndex, uint32_t periph_addr, uint32_t mem1_addr, uint32_t mem2_addr);

#endif /* INIT_DMA_H */
