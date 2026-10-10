Stream-chunking protocol ingestion engine (`UART_DMA_RunTransfer()`). Decodes 64-bit packet headers, validates Header CRC-8, detects End-of-File, routes 32-bit words to Flash target or PE Loader, computes streaming Data CRC-8, and accumulates total image CRC-16.

- **Source File:** `src/uart_dma_function_manager.c` (`UART_DMA_RunTransfer()`)
    
- **Packet Protocol:** [[HSI/UART_PACKET_FORMAT]]
    
- **Byte Source:** [[DMA_BYTE_SOURCE]]
    
- **Target Sinks:** [[SubMachines/PIC32_PE_LOADER]],[[SubMachines/PIC32_PE_OPERATIONS]]

---
SUB_MACHINE: UART_DMA_TRANSFER_MANAGER

INPUTS:  mode (UART_DMA_MODE_LOAD_PE | UART_DMA_MODE_PROGRAM_FLASH), summary (uart_dma_transfer_summary_t*)
OUTPUTS: status (UART_DMA_RESULT_OK | UART_DMA_RESULT_HEADER_CRC_ERROR | UART_DMA_RESULT_DATA_CRC_ERROR | UART_DMA_RESULT_PE_FAIL)

[INIT_TRANSFER_SESSION].
_sw_: transfer_first_address = 0
_sw_: transfer_total_bytes = 0
_sw_: transfer_crc = 0xFFFF
_sw_: have_first_address = 0
-> [READ_64BIT_HEADER]

[READ_64BIT_HEADER].
;Consume 8 header bytes via NextByte();
LOOP (i = 0 to 7):
  _sw_: hdr[i] = [[DMA_BYTE_SOURCE]]()

;Unpack word0 and address (Little-Endian);
_sw_: word0   = hdr[0] | (hdr[1] << 8) | (hdr[2] << 16) | (hdr[3] << 24)
_sw_: address = hdr[4] | (hdr[5] << 8) | (hdr[6] << 16) | (hdr[7] << 24)

_sw_: data_crc_expected   = word0 & 0xFF
_sw_: header_crc_expected = (word0 >> 8) & 0xFF
_sw_: byte_count          = (word0 >> 16) & 0x7FFF
_sw_: more_data           = (word0 >> 31) & 0x01
-> [VERIFY_HEADER_CRC]

[VERIFY_HEADER_CRC].
;Calculate CRC-8 over hdr[2..7];
_sw_: header_crc_calc = 0
LOOP (i = 2 to 7):
  _sw_: header_crc_calc = Crc8Update(header_crc_calc, hdr[i])

<header_crc_calc != header_crc_expected>
YES ->
  _sw_: UART5.SendString("CRC Error in Header")
  -> RETURN(UART_DMA_RESULT_HEADER_CRC_ERROR)
NO  -> [CHECK_EOF_FLAG]

[CHECK_EOF_FLAG].
<more_data == 0>
YES -> [FINALIZE_TRANSFER]
NO  ->
  <have_first_address == 0>
  YES ->
    _sw_: transfer_first_address = address
    _sw_: have_first_address = 1
  NO  -> ⌛ CONTINUE
  -> [DISPATCH_CHUNK_DESTINATION]

[DISPATCH_CHUNK_DESTINATION].
_sw_: remaining = byte_count
_sw_: data_crc_calc = 0

SWITCH (mode):
  CASE UART_DMA_MODE_LOAD_PE:
    _sw_: [[PIC32_EJTAG_PRIMITIVES]].SendCommand(ETAP_FASTDATA)
    _sw_: [[PIC32_PE_LOADER]].SendBlockHeader(address, byte_count / 4)
    -> [STREAM_PE_TO_FASTDATA]

  CASE UART_DMA_MODE_PROGRAM_FLASH:
    _sw_: word_index = 0
    -> [ROUTE_WORD_TO_SINK]

[STREAM_PE_TO_FASTDATA].
<remaining > 0>
YES ->
  _sw_: word = FetchNextWord(&data_crc_calc, &transfer_crc, &transfer_total_bytes)
  _sw_: [[PIC32_PE_LOADER]].SendWord(word)
  _sw_: remaining -= 4
  -> [STREAM_PE_TO_FASTDATA]
NO  -> [VERIFY_DATA_CRC]

[ROUTE_WORD_TO_SINK].
<remaining > 0>
YES ->
  _sw_: word = FetchNextWord(&data_crc_calc, &transfer_crc, &transfer_total_bytes)
  _sw_: status = [[PIC32_PE_OPERATIONS]].WordProgram(address + word_index * 4, word)
  <status.code != PE_RESP_PASS>
  YES ->
    _sw_: UART5.SendString("PE WORD_PROGRAM failed")
    -> RETURN(UART_DMA_RESULT_PE_FAIL)
  NO  ->
    _sw_: word_index += 1
    _sw_: remaining -= 4
    -> [ROUTE_WORD_TO_SINK]
NO  -> [VERIFY_DATA_CRC]

[VERIFY_DATA_CRC].
<data_crc_calc != data_crc_expected>
YES ->
  _sw_: UART5.SendString("CRC Error in Data")
  -> RETURN(UART_DMA_RESULT_DATA_CRC_ERROR)
NO  ->
  ;Chunk processed successfully; return to process the next 64-bit header;
  -> [READ_64BIT_HEADER]

[FINALIZE_TRANSFER].
<summary != NULL>
YES ->
  _sw_: summary->start_address = transfer_first_address
  _sw_: summary->total_bytes   = transfer_total_bytes
  _sw_: summary->crc           = transfer_crc
NO  -> ⌛ CONTINUE
-> RETURN(UART_DMA_RESULT_OK)