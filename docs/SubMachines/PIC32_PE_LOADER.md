Executes Table 11-1 Steps 1–8: Prepares PIC32 RAM via BMX configuration, injects the fixed Table 11-2 bootstrap loader at `0xA0000800`, coordinates payload ingestion via UART DMA, and triggers PE execution with the `0xDEAD0000` sentinel.

- **Source File:** `src/flash_pic32mx/download_pe.c`
    
- **Datasheet:** DS60001145U, Section 11.0
    
- **Parent Machine:** [[FLASHER_TOP]]
    
- **Stream Manager:** [[SubMachines/UART_DMA_TRANSFER_MANAGER]]
    
- **Low-Level Drivers:** [[SubMachines/EJTAG_XFER_INSTRUCTION]], [[SubMachines/EJTAG_XFER_FASTDATA]]
    

## 1. Table 11-2 Opcode Reference

```
Address       Opcode       Assembly Instruction
--------------------------------------------------
0xA0000800    0x3C07DEAD   lui   a3, 0xdead
0xA0000804    0x3C06FF20   lui   a2, 0xff20
0xA0000808    0x3C05FF20   lui   a1, 0xff20
0xA000080C    0x8CC40000   lw    a0, 0(a2)      ; here1: Read Target Address
0xA0000810    0x8CC30000   lw    v1, 0(a2)      ; Read Word Count
0xA0000814    0x1067000B   beq   v1, a3, here3  ; if count == 0xDEAD0000 goto jump
0xA0000818    0x00000000   nop
0xA000081C    0x1060FFFB   beqz  v1, here1      ; if count == 0 loop back
0xA0000820    0x00000000   nop
0xA0000824    0x8CA20000   lw    v0, 0(a1)      ; here2: Read Data Word from FastData
0xA0000828    0x2463FFFF   addiu v1, v1, -1
0xA000082C    0xAC820000   sw    v0, 0(a0)      ; Store Word to RAM
0xA0000830    0x24840004   addiu a0, a0, 4
0xA0000834    0x1460FFFB   bnez  v1, here2      ; Loop until block completed
0xA0000838    0x00000000   nop
0xA000083C    0x1000FFF3   b     here1
0xA0000840    0x00000000   nop
0xA0000844    0x3C02A000   lui   v0, 0xa000     ; here3: PE Entry Point Setup
0xA0000848    0x34420900   ori   v0, v0, 0x0900 ; Entry Vector = 0xA0000900
0xA000084C    0x00400008   jr    v0             ; Jump to Programming Executive
0xA0000850    0x00000000   nop
```



SUB_MACHINE: PIC32_PE_LOADER
INPUTS:  None
OUTPUTS: status (PE_LOAD_OK | PE_LOAD_FAIL)

[INIT_BMX_BUS_MATRIX].
  ;Table 11-1 Steps 1-3: Configure internal Bus Matrix for RAM kernel execution;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0x3c04bf88) ;lui a0, 0xbf88;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0x34842000) ;ori a0, a0, 0x2000 (&BMXCON);
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0x3c05001f) ;lui a1, 0x1f;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0x34a50040) ;ori a1, a1, 0x0040;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0xac850000) ;sw  a1, 0(a0) -> BMXCON;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0x34050800) ;li  a1, 0x800;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0xac850010) ;sw  a1, 16(a0) -> BMXDKPBA;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0x8c850040) ;lw  a1, 64(a0) -> BMXDRMSZ;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0xac850020) ;sw  a1, 32(a0) -> BMXDUDBA;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0xac850030) ;sw  a1, 48(a0) -> BMXDUPBA;
  -> [INJECT_LOADER_OPCODES]

[INJECT_LOADER_OPCODES].
  ;Table 11-1 Step 4-5: Write PE Loader opcodes to 0xA0000800;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0x3c04a000) ;lui a0, 0xa000;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0x34840800) ;ori a0, a0, 0x0800;
  LOOP (i = 0 to 20):
    _sw_: word = PE_LOADER_OPCODES[i]
    _sw_: [[EJTAG_XFER_INSTRUCTION]](0x3c060000 | (word >> 16))
    _sw_: [[EJTAG_XFER_INSTRUCTION]](0x34c60000 | (word & 0xFFFF))
    _sw_: [[EJTAG_XFER_INSTRUCTION]](0xac860000) ;sw a2, 0(a0);
    _sw_: [[EJTAG_XFER_INSTRUCTION]](0x24840004) ;addiu a0, a0, 4;
  ;Table 11-1 Step 6: Jump to bootstrap loader in RAM;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0x3c19a000) ;lui t9, 0xa000;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0x37390800) ;ori t9, t9, 0x0800;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0x03200008) ;jr  t9;
  _sw_: [[EJTAG_XFER_INSTRUCTION]](0x00000000) ;nop;
  -> [STREAM_VIA_UART_DMA]

[STREAM_VIA_UART_DMA].
  ;Table 11-1 Step 7: Stream PE chunks from PC UART into FastData;
  _sw_: status = [[UART_DMA_TRANSFER_MANAGER]](MODE_LOAD_PE, NULL)
  <status == UART_DMA_RESULT_OK>
    YES -> [SENTINEL_TRIGGER]
    NO  -> RETURN(PE_LOAD_FAIL)

[SENTINEL_TRIGGER].
  ;Table 11-1 Step 8: Magic Sentinel signals loader to branch to 0xA0000900;
  _sw_: [[EJTAG_XFER_FASTDATA]](0x00000000)
  _sw_: [[EJTAG_XFER_FASTDATA]](0xDEAD0000)
  -> RETURN(PE_LOAD_OK)