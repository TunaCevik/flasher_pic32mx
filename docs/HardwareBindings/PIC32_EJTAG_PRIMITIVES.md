Implements Microchip DS60001145U Section 6.0 ("EJTAG Pseudo-Operations") over a 2-wire serial link (PGC/PGD).

- **Source File:** `src/flash_pic32mx/pseudo_operations.c`
    
- **Datasheet:** DS60001145U, Section 5.0 (4-Phase Bit Protocol) & Section 6.0 (Pseudo-Ops)
    
- **Physical Driver:** [[HardwareBindings/PIC32_ICSP_BITBANG]]
    

## 1. 2-Wire 4-Phase Bit Cell Protocol (Section 5.0)

Every JTAG bit transaction (`ICSP_TransferBit`) multiplexes TDI, TMS, and TDO over PGD using 4 distinct clock phases on PGC:

|**Phase**|**Clock (PGC)**|**Data (PGD) Direction**|**Action / Meaning**|
|---|---|---|---|
|**Phase 1**|Clock High -> Low|STM32 Drive (Output)|**TDI:** Master drives input data bit to Target|
|**Phase 2**|Clock High -> Low|STM32 Drive (Output)|**TMS:** Master drives JTAG state machine control bit|
|**Phase 3**|Clock Pulse (High/Low)|High-Z (Master Input)|Target samples TDI/TMS; Master releases PGD bus|
|**Phase 4**|Clock Pulse (High/Low)|Target Drive (Input)|**TDO:** Master samples output data bit driven by Target|

## 2. Low-Level Control Commands

- **`SetMode(mode, num_bits)` (Section 6.1):** Shifts `num_bits` into TMS (TDI held at 0) to navigate the TAP controller states without sending data.
    
- **`SendCommand(command)` (Section 6.2):** Selects an EJTAG IR register (5-bit command). Drives TMS header `1100` (`Run-Test-Idle` -> `Shift-IR`), shifts 5 bits, exits via TMS `10` (`Update-IR` -> `Run-Test-Idle`).
    
- **`XferData(iData)` (Section 6.3):** 32-bit bidirectional data shift into the selected TAP register. Drives TMS header `100`, shifts 32 bits, exits via TMS `10`.
    
- **`Synchronize()` (Section 6.7):** Holds PGED High (`1`) for 24 clock pulses to force the EJTAG state machine into `Test-Logic-Reset`.
    

## 3. Associated SubMachines

Polling and instruction execution state machines are modeled separately in:

- [[SubMachines/EJTAG_XFER_FASTDATA]]
    
- [[SubMachines/EJTAG_XFER_INSTRUCTION]`