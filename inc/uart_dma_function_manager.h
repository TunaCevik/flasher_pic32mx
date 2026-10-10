#ifndef UART_DMA_FUNCTION_MANAGER_H
#define UART_DMA_FUNCTION_MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef enum {
    UART_DMA_MODE_LOAD_PE = 0,
    UART_DMA_MODE_PROGRAM_FLASH
} uart_dma_mode_t;

typedef enum {
    UART_DMA_RESULT_OK = 0,
    UART_DMA_RESULT_HEADER_CRC_ERROR,
    UART_DMA_RESULT_DATA_CRC_ERROR,
    UART_DMA_RESULT_PE_FAIL
} uart_dma_result_t;

typedef struct {
    uint32_t start_address;
    uint32_t total_bytes;
    uint16_t crc;
} uart_dma_transfer_summary_t;

void UART_DMA_FunctionManagerInit(void);
uart_dma_result_t UART_DMA_RunTransfer(uart_dma_mode_t mode, uart_dma_transfer_summary_t *summary);

#endif /* UART_DMA_FUNCTION_MANAGER_H */
