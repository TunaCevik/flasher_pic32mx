Implements Figure 3-1 Step 2 and Section 7.0 (Figure 7-1) "Entering 2-Wire Enhanced ICSP Mode". Executes the reset key sequence and validates AC timing margins from Table 21-1.

- **Source File:** `src/flash_pic32mx/enter_enhanced_icsp.c`
    
- **Datasheet:** DS60001145U, Section 7.0 (Figure 7-1 & Table 21-1 AC Characteristics)
    
- **Parent Machine:** [[FLASHER_TOP]]
    
- **Hardware Drivers:** [[HardwareBindings/PIC32_ICSP_BITBANG]]
    

## 1. Timing Specification Constraints (Table 21-1)

|**Parameter**|**Symbol**|**Description**|**Spec Limit**|**Implemented Margin**|
|---|---|---|---|---|
|**P20**|$T_{\text{MCLRH}}$|MCLR Initial High Pulse Width|Max 500 µs|**100 µs** (Pulse to reset target)|
|**P6**|$T_{\text{SET2}}$|$V_{\text{DD}}$ setup time to MCLR rise|Min 100 ns|Structural (Target powered beforehand)|
|**P18**|$T_{\text{KEY1}}$|Delay: MCLR $\downarrow$ to first PGEC $\uparrow$|Min 40 ns|**1 µs**|
|**P1B**|$T_{\text{PGCH}}$|Serial Clock High Time|Min 40 ns|**1 µs**|
|**P1A**|$T_{\text{PGCL}}$|Serial Clock Low Time|Min 40 ns|**1 µs**|
|**P19**|$T_{\text{KEY2}}$|Delay: Last PGEC $\downarrow$ to second MCLR $\uparrow$|Min 40 ns|**1 µs**|
|**P7**|$T_{\text{HLD2}}$|Data Hold Time after MCLR $\uparrow$|Min 500 ns|**1 µs** (Guard band before EJTAG traffic)|


SUB_MACHINE: ENTER_ENHANCED_ICSP
INPUTS:  None
OUTPUTS: status (ENTER_ICSP_OK)

[RESET_BUS_LINES].
  ;Force clock and data lines low;
  _hw_: PGEC = 0
  _hw_: [[PIC32_ICSP_BITBANG]].Set_PGD_Output(0)
  ⌛ DELAY(1us)
  -> [DRIVE_INITIAL_MCLR_PULSE]

[DRIVE_INITIAL_MCLR_PULSE].
  ;Step 1: MCLR high pulse to initialize target logic;
  _hw_: MCLR = 1
  ⌛ DELAY(100us) ;P20: max 500us;
  _hw_: MCLR = 0
  ⌛ DELAY(1us)   ;P18: setup to key clock;
  _sw_: bit_index = 31
  -> [SHIFT_KEY_SEQUENCE]

[SHIFT_KEY_SEQUENCE].
  ;Step 2: Stream 32-bit key 0x4D434850 MSb-first;
  _sw_: key_bit = (0x4D434850 >> bit_index) & 0x01
  _hw_: [[PIC32_ICSP_BITBANG]].Set_PGD_Output(key_bit)
  ⌛ DELAY(1us)
  _hw_: PGEC = 1
  ⌛ DELAY(1us)   ;P1B;
  _hw_: PGEC = 0
  ⌛ DELAY(1us)   ;P1A;
  <bit_index == 0>
    YES -> [LATCH_ENHANCED_MODE]
    NO  -> 
      _sw_: bit_index -= 1
      -> [SHIFT_KEY_SEQUENCE]

[LATCH_ENHANCED_MODE].
  ;Step 3: Release MCLR to latch target into Enhanced ICSP mode;
  ⌛ DELAY(1us)   ;P19;
  _hw_: MCLR = 1
  ⌛ DELAY(1us)   ;P7: hold time before EJTAG sync;
  -> RETURN(ENTER_ICSP_OK)