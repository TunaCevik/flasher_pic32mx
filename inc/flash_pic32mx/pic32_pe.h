#ifndef PIC32_PE_H
#define PIC32_PE_H

#include <stdint.h>

/* PE Command OpCodes (Header bits 31:16) */
#define PE_CMD_ROW_PROGRAM      0x0U
#define PE_CMD_READ             0x1U
#define PE_CMD_PROGRAM          0x2U
#define PE_CMD_WORD_PROGRAM     0x3U
#define PE_CMD_CHIP_ERASE       0x4U
#define PE_CMD_PAGE_ERASE       0x5U
#define PE_CMD_BLANK_CHECK      0x6U
#define PE_CMD_EXEC_VERSION     0x7U
#define PE_CMD_GET_CRC          0x8U
#define PE_CMD_PROGRAM_CLUSTER  0x9U
#define PE_CMD_GET_DEVICEID     0xAU
#define PE_CMD_CHANGE_CFG       0xBU
#define PE_CMD_GET_CHECKSUM     0xCU
#define PE_CMD_QUAD_WORD_PGRM   0xDU

/* PE Response Codes (Low 16 bits of status word) */
#define PE_RESP_PASS            0x0U
#define PE_RESP_FAIL            0x2U
#define PE_RESP_NACK            0x3U

/* Default row size: 128 bytes = 32 words (e.g. PIC32MX250F128B) */
#ifndef PIC32_PE_ROW_SIZE_WORDS
#define PIC32_PE_ROW_SIZE_WORDS 32U
#endif

typedef struct {
    uint16_t last_cmd;  /* Echoed opcode, or lower address fragment during PROGRAM */
    uint16_t code;      /* PE_RESP_PASS, PE_RESP_FAIL, or PE_RESP_NACK */
} pic32_pe_status_t;

typedef struct {
    pic32_pe_status_t status;
    uint16_t crc;       /* 16-bit CRC result */
} pic32_pe_crc_t;

/* Core JTAG/PE primitives */
uint32_t          GetPEResponse(void);

/* Flash operations */
pic32_pe_status_t PIC32_ChipErase(void);
uint32_t          PIC32_GetDeviceID(void);
pic32_pe_status_t PIC32_PageErase(uint32_t address, uint16_t num_pages);
pic32_pe_status_t PIC32_Read(uint32_t address, uint32_t *buffer, uint16_t word_count);
pic32_pe_status_t PIC32_RowProgram(uint32_t address, const uint32_t *row_data);
pic32_pe_status_t PIC32_WordProgram(uint32_t address, uint32_t data);
pic32_pe_status_t PIC32_Program(uint32_t address, const uint32_t *data, uint32_t length_bytes);
pic32_pe_crc_t    PIC32_GetCRC(uint32_t address, uint32_t length_bytes);

#endif /* PIC32_PE_H */
