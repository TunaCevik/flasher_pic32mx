Controls execution of injected 32-bit MIPS opcodes in PIC32 debug pipeline (Section 6.5).

- **Source File:** `src/flash_pic32mx/pseudo_operations.c` (`XferInstruction()`)
    
- **Hardware Binding:** [[HardwareBindings/PIC32_EJTAG_PRIMITIVES]]


SUB_MACHINE: EJTAG_XFER_INSTRUCTION
INPUTS:  instruction (uint32_t MIPS opcode)
OUTPUTS: None

[WAIT_PRACC_READY].
  _sw_: SendCommand(ETAP_CONTROL)
  _sw_: ctrl_val = XferData(0x0004C000)
  <ctrl_val.bit18_PrAcc == 1>
    YES -> [LOAD_INSTRUCTION]
    NO  -> ⌛ DELAY(1us) -> [WAIT_PRACC_READY]

[LOAD_INSTRUCTION].
  ;Load opcode into ETAP_DATA register;
  _sw_: SendCommand(ETAP_DATA)
  _sw_: XferData(instruction)
  -> [EXECUTE_INSTRUCTION]

[EXECUTE_INSTRUCTION].
  ;Trigger MIPS execution pipeline;
  _sw_: SendCommand(ETAP_CONTROL)
  _sw_: XferData(0x0000C000)
  -> RETURN