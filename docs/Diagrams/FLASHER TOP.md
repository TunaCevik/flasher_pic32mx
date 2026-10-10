# FLASHER TOP — Diagram

Visual companion to [[../FLASHER_TOP]] (`PIC32_FLASH_ENGINE`, Microchip DS60001145U Figure 3-1).

```mermaid
flowchart TD
    Idle(["STATE_IDLE<br/>await trigger_flash"]) --> TriggerCheck{"trigger_flash == 1?"}
    TriggerCheck -->|NO| Idle
    TriggerCheck -->|YES| EnterICSP

    EnterICSP["STATE_ENTER_ICSP<br/>Drive 2-wire Enhanced ICSP unlock (0x4D434850)<br/>SubMachine: ENTER_ENHANCED_ICSP"] --> ICSPCheck{"icsp_status ==<br/>ENTER_ICSP_OK?"}
    ICSPCheck -->|NO| FailICSP["UART5: 'FAIL: ICSP Entry'<br/>flash_result = FAIL_ICSP"] --> Abort
    ICSPCheck -->|YES| CheckStatus

    CheckStatus["STATE_CHECK_STATUS<br/>Verify CFGRDY=1, FCBUSY=0<br/>SubMachine: CHECK_DEVICE_STATUS"] --> StatusCheck{"dev_status.ready == 1?"}
    StatusCheck -->|NO| FailStatus["UART5: 'FAIL: Device Not Ready'<br/>flash_result = FAIL_STATUS"] --> Abort
    StatusCheck -->|YES| EraseDevice

    EraseDevice["STATE_ERASE_DEVICE<br/>Send MCHP_ERASE<br/>SubMachine: ERASE_DEVICE"] --> EraseCheck{"erase_status.ready == 1?"}
    EraseCheck -->|NO| FailErase["UART5: 'FAIL: Device Erase Timeout'<br/>flash_result = FAIL_ERASE"] --> Abort
    EraseCheck -->|YES| SerialExec

    SerialExec["STATE_ENTER_SERIAL_EXEC<br/>Check code protection, hook ETAP_EJTAGBOOT<br/>SubMachine: ENTER_SERIAL_EXEC"] --> ExecCheck{"exec_status ==<br/>ENTER_SERIAL_EXEC_OK?"}
    ExecCheck -->|NO| FailExec["UART5: 'FAIL: Target Code-Protected'<br/>flash_result = FAIL_EXEC"] --> Abort
    ExecCheck -->|YES| DownloadPE

    DownloadPE["STATE_DOWNLOAD_PE<br/>PIC32_PE_LOADER: InitBMX, LoadLoader<br/>UART_DMA_TRANSFER_MANAGER(MODE_LOAD_PE)"] --> PECheck{"pe_stream_status ==<br/>UART_DMA_RESULT_OK?"}
    PECheck -->|NO| FailPE["UART5: 'FAIL: PE Download Stream'<br/>flash_result = FAIL_PE"] --> Abort
    PECheck -->|YES| JumpPE["PIC32_PE_LOADER -> JumpToPE()"] --> ProgramFlash

    ProgramFlash["STATE_PROGRAM_FLASH<br/>UART_DMA_TRANSFER_MANAGER(MODE_PROGRAM_FLASH, &fw_summary)"] --> ProgramCheck{"fw_stream_status ==<br/>UART_DMA_RESULT_OK?"}
    ProgramCheck -->|NO| FailProgram["UART5: 'FAIL: Firmware Stream / Write'<br/>flash_result = FAIL_PROGRAM"] --> Abort
    ProgramCheck -->|YES| VerifyDevice

    VerifyDevice["STATE_VERIFY_DEVICE<br/>PIC32_PE_OPERATIONS -> GetCRC(start, total_bytes)"] --> CRCCheck{"pic_crc.status == PASS<br/>&& pic_crc.crc == fw_summary.crc?"}
    CRCCheck -->|NO| FailCRC["UART5: 'FAIL: CRC Mismatch'<br/>flash_result = FAIL_CRC"] --> Abort
    CRCCheck -->|YES| Success["UART5: 'SUCCESS: Target Verified'<br/>flash_result = SUCCESS"] --> ExitICSP

    ExitICSP["STATE_EXIT_ICSP<br/>Set_PGC(0), Set_PGD_Input()<br/>MCLR = 0 -> delay(100us) -> MCLR = 1"] --> Complete

    Abort["STATE_ABORT<br/>Set_PGD_Input()<br/>MCLR = 0 -> delay(100us) -> MCLR = 1"] --> Return(["RETURN(flash_result)"])

    Complete(["STATE_COMPLETE<br/>RETURN(SUCCESS)"])
```

**Notes**

- This is the master orchestration state machine — every box here delegates to one of the [[FLASHER_TOP|child SubMachines]] rather than doing hardware work directly, except for the final MCLR reset pulse shared by `STATE_EXIT_ICSP` and `STATE_ABORT`.
- `STATE_DOWNLOAD_PE` and `STATE_PROGRAM_FLASH` both call into [[Diagrams/UART DMA FUNC MANAGER]] — same engine, different `mode` argument and different destination for the streamed words (EJTAG FASTDATA vs. flash word-program).
- Every failure path converges on `STATE_ABORT`, which always tristates the data line and pulses `MCLR` regardless of *which* step failed — this is what keeps a half-programmed or half-unlocked target from being left in a stuck/undefined bus state.
- `STATE_VERIFY_DEVICE`'s CRC-16 compare is the other end of the running `transfer_crc` accumulated inside [[Diagrams/UART DMA FUNC MANAGER]] during `STATE_PROGRAM_FLASH`.
