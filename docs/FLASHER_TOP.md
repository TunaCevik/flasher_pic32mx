Master orchestration state machine implementing Microchip DS60001145U Figure 3-1 ("Programming Flow"). Coordinates sub-machines from physical bus entry to post-programming CRC-16 hardware verification.

- **Source File:** `src/flash_pic32mx/flash_engine.c`
    
- **Datasheet Specification:** DS60001145U, Figure 3-1 & Sections 7.0–17.0
    
- **Child SubMachines:**
    
    - [[SubMachines/ENTER_ENHANCED_ICSP]]
        
    - [[SubMachines/CHECK_DEVICE_STATUS]]
        
    - [[SubMachines/ERASE_DEVICE]]
        
    - [[SubMachines/ENTER_SERIAL_EXEC]]
        
    - [[SubMachines/PIC32_PE_LOADER]]
        
    - [[SubMachines/UART_DMA_TRANSFER_MANAGER]]
        
    - [[SubMachines/PIC32_PE_OPERATIONS]]

## 1. Engine State Enumeration

|**Engine State**|**Figure 3-1 Step**|**Primary Function / Action**|**Success Transition**|**Error Action**|
|---|---|---|---|---|
|**`STATE_IDLE`**|Step 1: Connect|Awaits flash command trigger|`STATE_ENTER_ICSP`|Stay in IDLE|
|**`STATE_ENTER_ICSP`**|Step 2: Enter ICSP|Drives 32-bit key `0x4D434850`|`STATE_CHECK_STATUS`|`STATE_ABORT`|
|**`STATE_CHECK_STATUS`**|Step 3: Status Check|Verifies `CFGRDY=1`, `FCBUSY=0`|`STATE_ERASE_DEVICE`|`STATE_ABORT`|
|**`STATE_ERASE_DEVICE`**|Step 4: Erase|Sends `MCHP_ERASE` command|`STATE_ENTER_SERIAL_EXEC`|`STATE_ABORT`|
|**`STATE_ENTER_SERIAL_EXEC`**|Step 5: Serial Exec|Injects `ETAP_EJTAGBOOT` vector|`STATE_DOWNLOAD_PE`|`STATE_ABORT`|
|**`STATE_DOWNLOAD_PE`**|Step 6: Download PE|Boots Table 11-2 loader & streams PE|`STATE_PROGRAM_FLASH`|`STATE_ABORT`|
|**`STATE_PROGRAM_FLASH`**|Step 7-8: Stream & Write|Streams firmware payload words|`STATE_VERIFY_DEVICE`|`STATE_ABORT`|
|**`STATE_VERIFY_DEVICE`**|Step 10: Verify|Hardware CRC check vs stream CRC|`STATE_EXIT_ICSP`|`STATE_ABORT`|
|**`STATE_EXIT_ICSP`**|Step 11: Exit Mode|MCLR reset assertion, bus release|`STATE_COMPLETE`|—|

## 2. Flash Engine DSL Execution Model

**Notation:** `_sw_`/`_hw_` marks a software (TAP/protocol-level) vs. hardware (direct pin-level) action. `[[NAME]]()` calls a linked sub-machine; `X.Method()` calls into a driver/primitive. `=` is the only assignment operator. `->` is reserved for state transitions only.

ENGINE: PIC32_FLASH_ENGINE
INPUTS:  None
OUTPUTS: flash_result (SUCCESS | FAIL_ICSP | FAIL_STATUS | FAIL_ERASE | FAIL_EXEC | FAIL_PE | FAIL_PROGRAM | FAIL_CRC)

[STATE_IDLE].
  ;System initialized. Wait for flasher activation event;
  <trigger_flash == 1>
    YES -> [STATE_ENTER_ICSP]
    NO  -> ⌛ WAIT -> [STATE_IDLE]

[STATE_ENTER_ICSP].
  ;Figure 3-1 Step 2: Drive 2-wire Enhanced ICSP unlock sequence;
  _sw_: icsp_status = [[ENTER_ENHANCED_ICSP]]()
  <icsp_status == ENTER_ICSP_OK>
    YES -> [STATE_CHECK_STATUS]
    NO  -> 
      _sw_: UART5.SendString("FAIL: ICSP Entry")
      _sw_: flash_result = FAIL_ICSP
      -> [STATE_ABORT]

[STATE_CHECK_STATUS].
  ;Figure 3-1 Step 3: Check configuration & flash controller ready;
  _sw_: dev_status = [[CHECK_DEVICE_STATUS]]()
  <dev_status.ready == 1>
    YES -> [STATE_ERASE_DEVICE]
    NO  -> 
      _sw_: UART5.SendString("FAIL: Device Not Ready")
      _sw_: flash_result = FAIL_STATUS
      -> [STATE_ABORT]

[STATE_ERASE_DEVICE].
  ;Figure 3-1 Step 4: Issue chip erase and wait for completion;
  _sw_: erase_status = [[ERASE_DEVICE]]()
  <erase_status.ready == 1>
    YES -> [STATE_ENTER_SERIAL_EXEC]
    NO  -> 
      _sw_: UART5.SendString("FAIL: Device Erase Timeout")
      _sw_: flash_result = FAIL_ERASE
      -> [STATE_ABORT]

[STATE_ENTER_SERIAL_EXEC].
  ;Figure 3-1 Step 5: Check code protection and hook EJTAGBOOT vector;
  _sw_: exec_status = [[ENTER_SERIAL_EXEC]]()
  <exec_status == ENTER_SERIAL_EXEC_OK>
    YES -> [STATE_DOWNLOAD_PE]
    NO  -> 
      _sw_: UART5.SendString("FAIL: Target Code-Protected")
      _sw_: flash_result = FAIL_EXEC
      -> [STATE_ABORT]

[STATE_DOWNLOAD_PE].
  ;Figure 3-1 Step 6: Configure BMX, inject Table 11-2 loader, stream PE binary;
  _sw_: [[PIC32_PE_LOADER]].InitBMX()
  _sw_: [[PIC32_PE_LOADER]].LoadLoader()
  _sw_: pe_stream_status = [[UART_DMA_TRANSFER_MANAGER]](MODE_LOAD_PE, NULL)
  <pe_stream_status == UART_DMA_RESULT_OK>
    YES -> 
      _sw_: [[PIC32_PE_LOADER]].JumpToPE()
      -> [STATE_PROGRAM_FLASH]
    NO  -> 
      _sw_: UART5.SendString("FAIL: PE Download Stream")
      _sw_: flash_result = FAIL_PE
      -> [STATE_ABORT]

[STATE_PROGRAM_FLASH].
  ;Figure 3-1 Step 7 & 8: Stream Intel HEX binary chunks and program words;
  _sw_: fw_stream_status = [[UART_DMA_TRANSFER_MANAGER]](MODE_PROGRAM_FLASH, &fw_summary)
  <fw_stream_status == UART_DMA_RESULT_OK>
    YES -> [STATE_VERIFY_DEVICE]
    NO  -> 
      _sw_: UART5.SendString("FAIL: Firmware Stream / Write")
      _sw_: flash_result = FAIL_PROGRAM
      -> [STATE_ABORT]

[STATE_VERIFY_DEVICE].
  ;Figure 3-1 Step 10: Run PE on-chip CRC-16 and compare with stream summary;
  _sw_: pic_crc = [[PIC32_PE_OPERATIONS]].GetCRC(fw_summary.start_address, fw_summary.total_bytes)
  <pic_crc.status.code == PE_RESP_PASS && pic_crc.crc == fw_summary.crc>
    YES -> 
      _sw_: UART5.SendString("SUCCESS: Target Verified")
      _sw_: flash_result = SUCCESS
      -> [STATE_EXIT_ICSP]
    NO  -> 
      _sw_: UART5.SendString("FAIL: CRC Mismatch")
      _sw_: flash_result = FAIL_CRC
      -> [STATE_ABORT]

[STATE_EXIT_ICSP].
  ;Figure 3-1 Step 11: Release target from programming mode;
  _hw_: [[PIC32_ICSP_BITBANG]].Set_PGC(0)
  _hw_: [[PIC32_ICSP_BITBANG]].Set_PGD_Input()
  _hw_: MCLR = 0
  ⌛ DELAY(100us)
  _hw_: MCLR = 1
  -> [STATE_COMPLETE]

[STATE_ABORT].
  ;Safe shutdown on error: tristate data line and pulse reset;
  _hw_: [[PIC32_ICSP_BITBANG]].Set_PGD_Input()
  _hw_: MCLR = 0
  ⌛ DELAY(100us)
  _hw_: MCLR = 1
  -> RETURN(flash_result)

[STATE_COMPLETE].
  -> RETURN(SUCCESS)