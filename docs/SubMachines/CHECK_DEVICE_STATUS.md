Implements Figure 3-1 Step 3 and Section 8.2 (Figure 8-1) "Check Device Status (2-Wire)". Polls Microchip TAP (MTAP) status bits to verify flash controller availability before erasure or programming.

- **Source File:** `src/flash_pic32mx/check_device_status.c`
    
- **Target Spec:** DS60001145U, Section 8.2 (Figure 8-1)
    
- **Parent Machine:** [[FLASHER_TOP]]
    
- **Low-Level Primitives:** [[HardwareBindings/PIC32_EJTAG_PRIMITIVES]]

## 1. Register & Status Bit Definitions

|**Signal / Field**|**Mask**|**Expected Value**|**Description**|
|---|---|---|---|
|**`CFGRDY`**|`MCHP_STATUS_CFGRDY_MASK`|`1`|Configuration Ready: Device internal config loaded|
|**`FCBUSY`**|`MCHP_STATUS_FCBUSY_MASK`|`0`|Flash Controller Busy: 0 indicates flash controller is idle|
|**Timeout Bound**|—|`10000 µs` (10 ms)|Fail threshold defined in Section 8.2 note|
|**Poll Period**|—|`100 µs`|Sleep interval between read retries|


SUB_MACHINE: CHECK_DEVICE_STATUS
INPUTS:  None
OUTPUTS: res (device_status_t: status_val, ready)

[SWITCH_TO_MTAP].
  ;Step 1-2: Force TAP reset and switch from EJTAG to MTAP;
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SetMode(0x1F, 6)
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SendCommand(MTAP_SW_MTAP)
  _sw_: elapsed_us = 0
  -> [SELECT_COMMAND_REG]

[SELECT_COMMAND_REG].
  ;Step 3-4: Reset TAP into Run-Test/Idle and select MTAP Command register;
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SetMode(0x1F, 6)
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SendCommand(MTAP_COMMAND)
  -> [POLL_DEVICE_READY]

[POLL_DEVICE_READY].
  ;Step 5: Shift out 32-bit status register;
  _sw_: status_val = [[PIC32_EJTAG_PRIMITIVES]].XferData(MCHP_STATUS)
  _sw_: is_cfgrdy = (status_val & CFGRDY_MASK) != 0
  _sw_: is_fcidle = (status_val & FCBUSY_MASK) == 0
  <is_cfgrdy == 1 && is_fcidle == 1>
    YES -> 
      _sw_: res.ready = 1, res.status_val = status_val
      -> RETURN(res)
    NO  -> [RETRY_OR_TIMEOUT]

[RETRY_OR_TIMEOUT].
  ;Step 6: Check 10 ms bound, wait and retry Step 4;
  <elapsed_us >= 10000>
    YES -> 
      _sw_: res.ready = 0, res.status_val = status_val
      -> RETURN(res) ;Timeout failure;
    NO  -> 
      _sw_: [[PIC32_EJTAG_PRIMITIVES]].SendCommand(MTAP_COMMAND)
      ⌛ DELAY(100us)
      _sw_: elapsed_us += 100
      -> [POLL_DEVICE_READY]