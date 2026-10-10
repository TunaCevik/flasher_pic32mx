#include <stddef.h>
#include "flash_pic32mx/pic32_pe.h"
#include "flash_pic32mx/pseudo_operations.h"

/* Pack 16-bit opcode (31:16) and 16-bit operand (15:0) */
static inline uint32_t PackCommandHeader(uint16_t opcode, uint16_t operand)
{
    return ((uint32_t)opcode << 16) | (uint32_t)operand;
}

/* Transmit command header over Fastdata */
static void SendPECommand(uint16_t opcode, uint16_t operand)
{
    SendCommand(ETAP_FASTDATA);
    (void)XferFastData(PackCommandHeader(opcode, operand));
}

/* Poll PrAcc (Control bit 18) until ready, then read response */
uint32_t GetPEResponse(void)
{
    uint32_t control_val;
    uint32_t response;

    /* [POLL_PE_READY] */
    SendCommand(ETAP_CONTROL);
    do {
        control_val = XferData(0x0004C000U);
    } while (((control_val >> 18) & 0x1U) != 0x1U);

    /* [READ_PE_DATA] */
    SendCommand(ETAP_DATA);
    response = XferData(0);

    /* [RESUME_PE_EXECUTION] */
    SendCommand(ETAP_CONTROL);
    (void)XferData(0x0000C000U);

    return response;
}

/* Split response word into {last_cmd, status_code} */
static inline pic32_pe_status_t DecodeStatus(uint32_t word)
{
    pic32_pe_status_t status;
    status.last_cmd = (uint16_t)(word >> 16);
    status.code     = (uint16_t)(word & 0xFFFFU);
    return status;
}

/* Stream a full 128-byte (32-word) row via Fastdata */
static void SendRowWords(const uint32_t *row_data)
{
    for (uint16_t i = 0; i < PIC32_PE_ROW_SIZE_WORDS; i++) {
        (void)XferFastData(row_data[i]);
    }
}

pic32_pe_status_t PIC32_ChipErase(void)
{
    SendPECommand(PE_CMD_CHIP_ERASE, 0);
    return DecodeStatus(GetPEResponse());
}

uint32_t PIC32_GetDeviceID(void)
{
    SendPECommand(PE_CMD_GET_DEVICEID, 0);
    return GetPEResponse();
}

pic32_pe_status_t PIC32_PageErase(uint32_t address, uint16_t num_pages)
{
    SendPECommand(PE_CMD_PAGE_ERASE, num_pages);
    (void)XferFastData(address);
    return DecodeStatus(GetPEResponse());
}

pic32_pe_status_t PIC32_Read(uint32_t address, uint32_t *buffer, uint16_t word_count)
{
    SendPECommand(PE_CMD_READ, word_count);
    (void)XferFastData(address);

    pic32_pe_status_t status = DecodeStatus(GetPEResponse());
    if (status.code == PE_RESP_PASS) {
        for (uint16_t i = 0; i < word_count; i++) {
            buffer[i] = GetPEResponse();
        }
    }
    return status;
}

pic32_pe_status_t PIC32_RowProgram(uint32_t address, const uint32_t *row_data)
{
    SendPECommand(PE_CMD_ROW_PROGRAM, 0);
    (void)XferFastData(address);
    SendRowWords(row_data);
    return DecodeStatus(GetPEResponse());
}

pic32_pe_status_t PIC32_WordProgram(uint32_t address, uint32_t data)
{
    SendPECommand(PE_CMD_WORD_PROGRAM, 0);
    (void)XferFastData(address);
    (void)XferFastData(data);
    return DecodeStatus(GetPEResponse());
}

pic32_pe_status_t PIC32_Program(uint32_t address, const uint32_t *data, uint32_t length_bytes)
{
    uint32_t num_rows = length_bytes / (PIC32_PE_ROW_SIZE_WORDS * 4U);

    SendPECommand(PE_CMD_PROGRAM, 0);
    (void)XferFastData(address);
    (void)XferFastData(length_bytes);

    if (num_rows <= 1U) {
        SendRowWords(data);
        return DecodeStatus(GetPEResponse());
    }

    /* Pipelined transfer: prime with rows 0 and 1 */
    SendRowWords(data);
    SendRowWords(data + PIC32_PE_ROW_SIZE_WORDS);
    uint32_t next_row = 2;

    /* Harvest response for row 0 */
    pic32_pe_status_t status = DecodeStatus(GetPEResponse());

    /* Stream remaining rows; responses lag by one row */
    while ((status.code == PE_RESP_PASS) && (next_row < num_rows)) {
        SendRowWords(data + ((size_t)next_row * PIC32_PE_ROW_SIZE_WORDS));
        next_row++;
        status = DecodeStatus(GetPEResponse());
    }

    /* Collect final row status if no failure occurred */
    if (status.code == PE_RESP_PASS) {
        status = DecodeStatus(GetPEResponse());
    }

    return status;
}

pic32_pe_crc_t PIC32_GetCRC(uint32_t address, uint32_t length_bytes)
{
    pic32_pe_crc_t result;

    SendPECommand(PE_CMD_GET_CRC, 0);
    (void)XferFastData(address);
    (void)XferFastData(length_bytes);

    result.status = DecodeStatus(GetPEResponse());
    result.crc = (result.status.code == PE_RESP_PASS)
                 ? (uint16_t)(GetPEResponse() & 0xFFFFU)
                 : 0U;

    return result;
}
