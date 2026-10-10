| **Symbol** | **Type / Location** | **Size**  | **Producer**          | **Consumer**        | **Description** |
| ---------- | ------------------- | --------- | --------------------- | ------------------- | --------------- |
| `s_buf1`   | SRAM (`M0AR`)       | 128 Bytes | `_hw_` (DMA1_Stream0) | `_sw_` (`NextByte`) | Ping buffer     |
| `s_buf2`   | SRAM (`M1AR`)       | 128 Bytes | `_hw_` (DMA1_Stream0) | `_sw_` (`NextByte`) | Pong buffer     |
## Control Signals & Registers

- **`DMA_SxCR.CT` (Current Target):**
    
    - `0`: DMA hardware is actively writing into `s_buf1`. Software can safely consume `s_buf2`.
        
    - `1`: DMA hardware is actively writing into `s_buf2`. Software can safely consume `s_buf1`.
        
- **`s_offset`:** Software index (`0..127`) inside the currently active buffer.
    
- **`s_current_buf`:** Software tracking flag (`0` or `1`) indicating which buffer is being consumed.
    

## Pacing Protocol

- **Credit Value:** 2 buffers (128 bytes each).
    
- **`UART_DMA_ACK_BYTE` (0x06):** Emitted over UART5 TX as soon as `s_offset == 128`. Signals the PC host that the drained buffer is freed for subsequent DMA filling.