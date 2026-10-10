Hardware-level pin toggling, pin direction switching (tristate), and input sampling implementations for the PIC32 2-wire serial bus.

- **Source File:** `src/flash_pic32mx/icsp_bitbang.c`
    
- **Target Spec:** DS60001145U, Section 5.0 (2-Wire ICSP Protocol Details)
    
- **Pins Driven:** PGC (Clock), PGD (Data) [[HardwareBindings/PIC32_ICSP_PINOUT]]
    
- **Used by:** [[PIC32_EJTAG_PRIMITIVES]] (`pseudo_operations.c`)

## 1. Pin Control Matrix

|**Function**|**Targeted Register (STM32 LL)**|**Bit Action**|**PIC32 Electrical Behavior**|
|---|---|---|---|
|**`ICSP_Set_PGC(1)`**|`GPIOD->BSRR` (Set)|Sets bit `0`|PGC rising edge: Latch bit into TAP controller|
|**`ICSP_Set_PGC(0)`**|`GPIOD->BSRR` (Reset)|Sets bit `16`|PGC falling edge: Prepares next bit shift|
|**`ICSP_Set_PGD_Output(val)`**|`GPIOD->MODER` + `BSRR`|Mode = `01b` (Output)|Drives logic `0` or `1` from Master to Target|
|**`ICSP_Set_PGD_Input()`**|`GPIOD->MODER` + `PUPDR`|Mode = `00b` (Input)|Tristates Master pin; Target drives line|
|**`ICSP_Read_PGD()`**|`GPIOD->IDR`|Reads bit `1`|Samples incoming bit driven by Target|