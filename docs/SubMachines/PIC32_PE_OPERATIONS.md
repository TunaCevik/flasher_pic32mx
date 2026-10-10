Executes high-level Flash commands via the in-RAM Programming Executive (PE) protocol using the FastData register (DS60001145U Section 17.0).

- **Source File:** `src/flash_pic32mx/pic32_pe.c`
    
- **Target Spec:** DS60001145U, Section 17.0
    
- **Parent Machine:** [[FLASHER_TOP]]
    
- **Transport Driver:** [[SubMachines/EJTAG_XFER_FASTDATA]], [[HardwareBindings/PIC32_EJTAG_PRIMITIVES]]
    

## 1. PE Command Opcodes & Format (Table 17-1)

Every command header word is 32-bit: `[Opcode (31:16)] | [Operand (15:0)]`.

|**Command**|**Opcode (16-bit)**|**Operand**|**Response Words**|**Section**|
|---|---|---|---|---|
|**`PE_CMD_CHIP_ERASE`**|`0x0001`|`0x0000`|1 Word (`Last_Cmd` + `Status`)|17.1|
|**`PE_CMD_PAGE_ERASE`**|`0x0002`|`Num_Pages`|1 Word (`Last_Cmd` + `Status`)|17.2|
|**`PE_CMD_ROW_PROGRAM`**|`0x0003`|`0x0000`|1 Word (`Last_Cmd` + `Status`)|17.4|
|**`PE_CMD_READ`**|`0x0004`|`Word_Count`|1 Word + `Word_Count` Data|17.5|
|**`PE_CMD_PROGRAM`**|`0x0005`|`0x0000`|1 Word per Row (Pipelined)|17.6|
|**`PE_CMD_WORD_PROGRAM`**|`0x0006`|`0x0000`|1 Word (`Last_Cmd` + `Status`)|17.7|
|**`PE_CMD_GET_CRC`**|`0x0007`|`0x0000`|2 Words (Status + 16-bit CRC)|17.8|
|**`PE_CMD_GET_DEVICEID`**|`0x0008`|`0x0000`|1 Word (32-bit Device ID)|17.9|
