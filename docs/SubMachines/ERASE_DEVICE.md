Implements Figure 3-1 Step 4 and Section 9.0 (Figure 9-1) "Erasing the Device". Executes full bulk chip erase via the Microchip TAP (MTAP) and monitors the flash controller until complete.

- **Source File:** `src/flash_pic32mx/erase_device.c`
    
- **Target Spec:** DS60001145U, Section 9.0 (Figure 9-1)
    
- **Parent Machine:** [[FLASHER_TOP]]
    
- **Low-Level Primitives:** [[HardwareBindings/PIC32_EJTAG_PRIMITIVES]]

## 1. Parameters & Status Fields

| Symbol / Command         | Value / Mask                     | Description                                        |
| ------------------------ | -------------------------------- | -------------------------------------------------- |
| **`MCHP_ERASE`**         | `0x00000004` (Command)           | Opcode to initiate bulk flash erasure              |
| **`MCHP_DE_ASSERT_RST`** | `0x00000000` (Command)           | Re-asserted release sequence per Figure 9-1 Step 5 |
| **`CFGRDY`**             | `MCHP_STATUS_CFGRDY_MASK`        | Must be `1` (Configuration loaded)                 |
| **`FCBUSY`**             | `MCHP_STATUS_FCBUSY_MASK`        | Must be `0` (Flash controller idle / erase done)   |
| **Poll Interval**        | `10 ms` (`ERASE_DEVICE_POLL_MS`) | Physical discharge settling delay between polls    |
| **Timeout Limit**        | `ERASE_DEVICE_TIMEOUT_MS`        | Maximum duration before reporting an erase failure |


SUB_MACHINE: ERASE_DEVICE
INPUTS:  None
OUTPUTS: res (device_status_t: status_val, ready)

[ENTER_MTAP_COMMAND_MODE].
  ;Steps 1-3: Route TAP controller to MTAP command interface;
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SendCommand(MTAP_SW_MTAP)
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SetMode(0x1F, 6)
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SendCommand(MTAP_COMMAND)
  _sw_: elapsed_ms = 0
  -> [ISSUE_CHIP_ERASE]

[ISSUE_CHIP_ERASE].
  ;Step 4: Send the Chip Erase command to MTAP;
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].XferData(MCHP_ERASE)
  -> [WAIT_ERASE_INTERVAL]

[WAIT_ERASE_INTERVAL].
  ;Step 5: De-assert reset (required by spec across families);
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].XferData(MCHP_DE_ASSERT_RST)
  ;Step 6: Hardware erase execution delay;
  ⌛ DELAY(10ms)
  _sw_: elapsed_ms += 10
  -> [POLL_ERASE_STATUS]

[POLL_ERASE_STATUS].
;Step 7: Read erase progress from MCHP_STATUS;
_sw_: status_val = [[PIC32_EJTAG_PRIMITIVES]].XferData(MCHP_STATUS)
_sw_: is_ready = ((status_val & CFGRDY_MASK) != 0) && ((status_val & FCBUSY_MASK) == 0)
_sw_: is_timeout = (elapsed_ms >= ERASE_DEVICE_TIMEOUT_MS)

<(is_ready == 1) || (is_timeout == 1)>

YES ->
_sw_: res.ready = is_ready, res.status_val = status_val
-> RETURN(res)

NO  -> [WAIT_ERASE_INTERVAL]