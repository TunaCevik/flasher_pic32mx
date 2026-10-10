# UART DMA FUNC MANAGER — Diagram

Visual companion to [[SubMachines/UART_DMA_TRANSFER_MANAGER]] (`UART_DMA_RunTransfer()`).

```mermaid
flowchart TD
    Start(["UART_DMA_TRANSFER_MANAGER(mode, summary)"]) --> Init["INIT_TRANSFER_SESSION<br/>transfer_first_address = 0<br/>transfer_total_bytes = 0<br/>transfer_crc = 0xFFFF<br/>have_first_address = 0"]

    Init --> ReadHeader["READ_64BIT_HEADER<br/>LOOP i=0..7: hdr[i] = DMA_BYTE_SOURCE()<br/>unpack word0, address (little-endian)"]

    ReadHeader --> VerifyCRC["VERIFY_HEADER_CRC<br/>header_crc_calc = Crc8 over hdr[2..7]"]
    VerifyCRC --> CRCDecision{"header_crc_calc ==<br/>header_crc_expected?"}
    CRCDecision -->|NO| HeaderErr["/UART5: 'CRC Error in Header'/"]
    HeaderErr --> RetHeaderErr(["RETURN(HEADER_CRC_ERROR)"])

    CRCDecision -->|YES| EOFCheck{"more_data == 0?"}
    EOFCheck -->|YES| Finalize["FINALIZE_TRANSFER<br/>fill *summary if != NULL"]
    Finalize --> RetOK(["RETURN(OK)"])

    EOFCheck -->|NO| FirstAddrCheck{"have_first_address == 0?"}
    FirstAddrCheck -->|YES| SetFirst["transfer_first_address = address<br/>have_first_address = 1"]
    FirstAddrCheck -->|NO| Dispatch
    SetFirst --> Dispatch["DISPATCH_CHUNK_DESTINATION<br/>remaining = byte_count<br/>data_crc_calc = 0"]

    Dispatch --> ModeSwitch{"mode ?"}

    ModeSwitch -->|MODE_LOAD_PE| StreamPEInit["Driver::PIC32_EJTAG_PRIMITIVES -> SendCommand(ETAP_FASTDATA)<br/>PIC32_PE_LOADER -> SendBlockHeader(address, byte_count/4)"]
    StreamPEInit --> PELoop{"remaining > 0?"}
    PELoop -->|YES| PEWord["word = FetchNextWord(...)<br/>PIC32_PE_LOADER -> SendWord(word)<br/>remaining -= 4"]
    PEWord --> PELoop
    PELoop -->|NO| VerifyData

    ModeSwitch -->|MODE_PROGRAM_FLASH| RouteInit["ROUTE_WORD_TO_SINK<br/>word_index = 0"]
    RouteInit --> FlashLoop{"remaining > 0?"}
    FlashLoop -->|YES| WordProgram["word = FetchNextWord(...)<br/>status = PIC32_PE_OPERATIONS -> WordProgram(address + word_index*4, word)"]
    WordProgram --> PEStatus{"status.code ==<br/>PE_RESP_PASS?"}
    PEStatus -->|NO| PEFail["/UART5: 'PE WORD_PROGRAM failed'/"]
    PEFail --> RetPEFail(["RETURN(PE_FAIL)"])
    PEStatus -->|YES| Advance["word_index += 1<br/>remaining -= 4"]
    Advance --> FlashLoop
    FlashLoop -->|NO| VerifyData

    VerifyData["VERIFY_DATA_CRC"] --> DataCRCDecision{"data_crc_calc ==<br/>data_crc_expected?"}
    DataCRCDecision -->|NO| DataErr["/UART5: 'CRC Error in Data'/"]
    DataErr --> RetDataErr(["RETURN(DATA_CRC_ERROR)"])
    DataCRCDecision -->|YES — chunk done, loop for next header| ReadHeader
```

**Notes**

- This is a single blocking loop: `READ_64BIT_HEADER` → `VERIFY_HEADER_CRC` → (dispatch one chunk) → `VERIFY_DATA_CRC` → back to `READ_64BIT_HEADER`, until a header arrives with `more_data == 0`.
- The two `mode` branches both pull bytes through the same `FetchNextWord` helper (4× [[Diagrams/NEXT BYTE|NextByte]] calls + running CRC-8/CRC-16 update) — they only differ in *where the resulting 32-bit word is sent*: EJTAG FASTDATA for PE bootstrap, or `PIC32_PE_OPERATIONS::WordProgram` for the real firmware payload.
- `transfer_crc` (CRC-16) accumulates across the **entire** transfer regardless of chunk boundaries — it's what [[SubMachines/PIC32_PE_OPERATIONS]]'s `GetCRC` result gets compared against in `STATE_VERIFY_DEVICE` back in [[Diagrams/FLASHER TOP]].
- Packet layout referenced here (`word0` bit-packing, header vs. data CRC placement) is defined in [[HSI/UART_PACKET_FORMAT]].
