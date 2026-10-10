Implements Figure 3-1 Step 5 and Section 10.2 (Figure 10-1) "Entering Serial Execution Mode (2-Wire)". Verifies code protection status, asserts debug boot vectors via EJTAGBOOT, releases CPU reset, and enables flash execution.

- **Source File:** `src/flash_pic32mx/enter_serial_exec.c`
    
- **Target Spec:** DS60001145U, Section 10.2 (Figure 10-1)
    
- **Parent Machine:** [[FLASHER_TOP]]
    
- **Low-Level Primitives:** [[HardwareBindings/PIC32_EJTAG_PRIMITIVES]]
    

## 1. Parameters & Status Fields

|**Symbol / Command**|**Value / Mask**|**Description**|
|---|---|---|
|**`CPS`**|`MCHP_STATUS_CPS_MASK` (Bit 7)|Code Protect Status: `1` = Not Protected, `0` = Protected|
|**`MCHP_ASSERT_RST`**|`0x00000001` (Command)|Holds the PIC32 CPU core in reset|
|**`ETAP_EJTAGBOOT`**|`0x0C` (5-bit IR Command)|Forces CPU to boot from EJTAG memory space on reset release|
|**`MCHP_DE_ASSERT_RST`**|`0x00000000` (Command)|Releases CPU core from reset into Serial Exec Mode|
|**`MCHP_FLASH_ENABLE`**|`0x00000006` (Command)|Enables flash memory subsystem access for PIC32MX devices|

## 2. Sequence State Machine (DSL)


SUB_MACHINE: ENTER_SERIAL_EXEC
INPUTS:  None
OUTPUTS: status (ENTER_SERIAL_EXEC_OK | ENTER_SERIAL_EXEC_CODE_PROTECTED)

[CHECK_CODE_PROTECTION].
  ;Steps 1-3: Switch to MTAP and select Command register;
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SendCommand(MTAP_SW_MTAP)
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SetMode(0x1F, 6)
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SendCommand(MTAP_COMMAND)
  ;Step 4: Read status word;
  _sw_: status_val = [[PIC32_EJTAG_PRIMITIVES]].XferData(MCHP_STATUS)
  ;Step 5: Verify CPS bit (Bit 7);
  <status_val.bit7_CPS == 1>
    YES -> [ARM_EJTAG_BOOT]
    NO  -> RETURN(ENTER_SERIAL_EXEC_CODE_PROTECTED)

[ARM_EJTAG_BOOT].
  ;Step 6: Assert CPU reset;
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].XferData(MCHP_ASSERT_RST)
  ;Steps 7-9: Switch to ETAP and program EJTAGBOOT command;
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SendCommand(MTAP_SW_ETAP)
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SetMode(0x1F, 6)
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SendCommand(ETAP_EJTAGBOOT)
  -> [RELEASE_RESET_AND_ENABLE_FLASH]

[RELEASE_RESET_AND_ENABLE_FLASH].
  ;Steps 10-12: Switch back to MTAP command mode;
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SendCommand(MTAP_SW_MTAP)
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SetMode(0x1F, 6)
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SendCommand(MTAP_COMMAND)
  ;Step 13: De-assert reset (CPU vector routes to EJTAGBOOT);
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].XferData(MCHP_DE_ASSERT_RST)
  ;Step 14: Enable target flash access;
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].XferData(MCHP_FLASH_ENABLE)
  -> [ENGAGE_ETAP]

[ENGAGE_ETAP].
  ;Steps 15-16: Put controller permanently in ETAP ready state;
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SendCommand(MTAP_SW_ETAP)
  _sw_: [[PIC32_EJTAG_PRIMITIVES]].SetMode(0x1F, 6)
  -> RETURN(ENTER_SERIAL_EXEC_OK)