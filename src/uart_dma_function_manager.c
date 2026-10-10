#include "uart_dma_function_manager.h"
#include "hardware.h"
#include "init/init_dma.h"
#include "init/init_usart.h"
#include "flash_pic32mx/pseudo_operations.h"
#include "flash_pic32mx/download_pe.h"
#include "flash_pic32mx/pic32_pe.h"
#include "flash_pic32mx/tap_controller.h"

#define UART_DMA_BUFFER_BYTES 128U
#define UART_DMA_ACK_BYTE     0x06U
#define UART_DMA_CRC8_POLY    0x07U
#define UART_DMA_CRC16_POLY   0x1021U
#define UART_DMA_CRC16_INIT   0xFFFFU

static uint8_t s_buf1[UART_DMA_BUFFER_BYTES];
static uint8_t s_buf2[UART_DMA_BUFFER_BYTES];
/* false = s_buf1 (Memory 0), true = s_buf2 (Memory 1) */
static bool s_current_buf = false;
static uint32_t s_offset = 0;

void UART_DMA_FunctionManagerInit(void)
{
    UART_DMA_DoubleBuffer_Init(DMA1, LL_DMA_STREAM_0, (uint32_t)&UART5->DR,
                                (uint32_t)s_buf1, (uint32_t)s_buf2);
    s_current_buf = false;
    s_offset = 0;
}

static uint8_t Crc8Update(uint8_t crc, uint8_t byte_in)
{
    uint8_t crc_local = (uint8_t)(crc ^ byte_in);
    uint8_t bit;

    for (bit = 0; bit < 8U; bit++) {
        if ((crc_local & 0x80U) != 0U) {
            crc_local = (uint8_t)((uint32_t)(crc_local << 1) ^ UART_DMA_CRC8_POLY);
        } else {
            crc_local = (uint8_t)(crc_local << 1);
        }
    }

    return crc_local;
}

static uint16_t Crc16Update(uint16_t crc, uint8_t byte_in)
{
    uint16_t crc_local = (uint16_t)(crc ^ ((uint16_t)byte_in << 8));
    uint8_t bit;

    for (bit = 0; bit < 8U; bit++) {
        if ((crc_local & 0x8000U) != 0U) {
            crc_local = (uint16_t)((uint32_t)(crc_local << 1) ^ UART_DMA_CRC16_POLY);
        } else {
            crc_local = (uint16_t)(crc_local << 1);
        }
    }

    return crc_local;
}

static uint8_t NextByte(void)
{
    /* [CHECK_BUFFER_BOUNDARY] */
    if (s_offset == 0U) {
        bool dma_active_buf;

	/* [POLL_DMA_TARGET_FLIP] */
        do {
            dma_active_buf = (LL_DMA_GetCurrentTargetMem(DMA1, LL_DMA_STREAM_0)
                              == LL_DMA_CURRENTTARGETMEM1);
        } while (dma_active_buf == s_current_buf);
    }

    /* [FETCH_BYTE_FROM_BUFFER] */
    uint8_t *buf = s_current_buf ? s_buf2 : s_buf1;
    uint8_t byte_val = buf[s_offset++];

    /* [FLIP_BUFFER_AND_ACK] */
    if (s_offset == UART_DMA_BUFFER_BYTES) {
        UART5_SendChar((char)UART_DMA_ACK_BYTE);
        s_offset = 0U;
        s_current_buf = !s_current_buf;
    }

    return byte_val;
}

static inline uint32_t FetchNextWord(uint8_t *data_crc_calc,
                                     uint16_t *transfer_crc,
                                     uint32_t *transfer_total_bytes)
{
    /* [STREAM_PAYLOAD_WORDS] */
    uint8_t b0 = NextByte();
    uint8_t b1 = NextByte();
    uint8_t b2 = NextByte();
    uint8_t b3 = NextByte();

    *data_crc_calc = Crc8Update(*data_crc_calc, b0);
    *data_crc_calc = Crc8Update(*data_crc_calc, b1);
    *data_crc_calc = Crc8Update(*data_crc_calc, b2);
    *data_crc_calc = Crc8Update(*data_crc_calc, b3);

    *transfer_crc = Crc16Update(*transfer_crc, b0);
    *transfer_crc = Crc16Update(*transfer_crc, b1);
    *transfer_crc = Crc16Update(*transfer_crc, b2);
    *transfer_crc = Crc16Update(*transfer_crc, b3);

    *transfer_total_bytes += 4U;

    return ((uint32_t)b0) |
           (((uint32_t)b1) << 8) |
           (((uint32_t)b2) << 16) |
           (((uint32_t)b3) << 24);
}

uart_dma_result_t UART_DMA_RunTransfer(uart_dma_mode_t mode, uart_dma_transfer_summary_t *summary)
{
    /* [INIT_TRANSFER_SESSION] */
    uint32_t transfer_first_address = 0;
    uint32_t transfer_total_bytes = 0;
    uint16_t transfer_crc = UART_DMA_CRC16_INIT;
    uint8_t have_first_address = 0;

    for (;;) {
        /* [READ_64BIT_HEADER] */
        uint8_t hdr[8];
        uint8_t i;
        uint32_t word0;
        uint32_t address;
        uint8_t data_crc_expected;
        uint8_t header_crc_expected;
        uint32_t byte_count;
        uint8_t more_data;
        uint8_t header_crc_calc;
        uint32_t remaining;
        uint8_t data_crc_calc;

        for (i = 0; i < 8U; i++) {
            hdr[i] = NextByte();
        }

        word0 = (uint32_t)hdr[0] | ((uint32_t)hdr[1] << 8) |
                ((uint32_t)hdr[2] << 16) | ((uint32_t)hdr[3] << 24);
        address = (uint32_t)hdr[4] | ((uint32_t)hdr[5] << 8) |
                  ((uint32_t)hdr[6] << 16) | ((uint32_t)hdr[7] << 24);

        data_crc_expected = (uint8_t)(word0 & 0xFFU);
        header_crc_expected = (uint8_t)((word0 >> 8) & 0xFFU);
        byte_count = (word0 >> 16) & 0x7FFFU;
        more_data = (uint8_t)((word0 >> 31) & 0x1U);

        /* [VERIFY_HEADER_CRC] */
        header_crc_calc = 0;
        for (i = 2; i < 8U; i++) {
            header_crc_calc = Crc8Update(header_crc_calc, hdr[i]);
        }

        if (header_crc_calc != header_crc_expected) {
            UART5_SendString("CRC Error in Header");
            return UART_DMA_RESULT_HEADER_CRC_ERROR;
        }

        /* [CHECK_EOF_FLAG] */
        if (more_data == 0U) {
            /* [FINALIZE_TRANSFER] */
            if (summary != NULL) {
                summary->start_address = transfer_first_address;
                summary->total_bytes = transfer_total_bytes;
                summary->crc = transfer_crc;
            }
            return UART_DMA_RESULT_OK;
        }

        if (have_first_address == 0U) {
            transfer_first_address = address;
            have_first_address = 1U;
        }

        /* [DISPATCH_CHUNK_DESTINATION] */
        remaining = byte_count;
        data_crc_calc = 0;

        switch (mode) {
        case UART_DMA_MODE_LOAD_PE:
            /* [STREAM_PE_TO_FASTDATA] */
            SendCommand(ETAP_FASTDATA);
            DownloadPE_SendBlockHeader(address, byte_count / 4U);

            while (remaining > 0U) {
                uint32_t word = FetchNextWord(&data_crc_calc, &transfer_crc, &transfer_total_bytes);
                DownloadPE_SendWord(word);
                remaining -= 4U;
            }
            break;

        case UART_DMA_MODE_PROGRAM_FLASH:
            /* [ROUTE_WORD_TO_SINK] */
            {
                uint32_t word_index = 0;

                while (remaining > 0U) {
                    uint32_t word = FetchNextWord(&data_crc_calc, &transfer_crc, &transfer_total_bytes);

                    pic32_pe_status_t status =
                        PIC32_WordProgram(address + word_index * 4U, word);

                    if (status.code != PE_RESP_PASS) {
                        UART5_SendString("PE WORD_PROGRAM failed");
                        return UART_DMA_RESULT_PE_FAIL;
                    }

                    word_index++;
                    remaining -= 4U;
                }
            }
            break;
        }

        /* [VERIFY_DATA_CRC] */
        if (data_crc_calc != data_crc_expected) {
            UART5_SendString("CRC Error in Data");
            return UART_DMA_RESULT_DATA_CRC_ERROR;
        }
    }
}
