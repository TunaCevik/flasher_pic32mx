Hardware-software handshake primitive for circular double-buffered UART reception (`NextByte()`). Resolves byte streaming from `s_buf1` / `s_buf2`, tracks consumer offset, manages target buffer toggling via DMA `CT` bit, and transmits ACK (`0x06`) pacing tokens.

- **Source File:** `src/uart_dma_function_manager.c` (`NextByte()`)
    
- **Hardware Binding:** [[HardwareBindings/STM32_DMA1_STREAM0]]
    
- **HSI Definition:** [[HSI/DMA_PINGPONG_INTERFACE]]
    

SUB_MACHINE: DMA_BYTE_SOURCE

INPUTS:  None
OUTPUTS: byte (uint8_t)

[CHECK_BUFFER_BOUNDARY].
;When offset is 0, consumer is at buffer boundary and must verify DMA completion;
<s_offset == 0>
YES -> [POLL_DMA_TARGET_FLIP]
NO  -> [FETCH_BYTE_FROM_BUFFER]

[POLL_DMA_TARGET_FLIP].
;Read DMA1_Stream0 Current Target (CT) bit;
;CT=0: DMA filling s_buf1 | CT=1: DMA filling s_buf2;
_hw_: dma_active_buf = (READ(DMA1_Stream0->CR.CT) == 1)

<⌛ POLLING: dma_active_buf == s_current_buf>
YES -> ⌛ CONTINUE -> [POLL_DMA_TARGET_FLIP] ;Spin-poll until hardware flips to opposite buffer;
NO  -> [FETCH_BYTE_FROM_BUFFER]

[FETCH_BYTE_FROM_BUFFER].
;Consume single byte from valid memory address and increment index;
_sw_: byte = s_current_buf ? s_buf2[s_offset] : s_buf1[s_offset]
_sw_: s_offset = s_offset + 1

<s_offset == 128>
YES -> [FLIP_BUFFER_AND_ACK]
NO  -> RETURN(byte)

[FLIP_BUFFER_AND_ACK].
;Send pacing ACK (0x06) to Host PC;
;Toggle buffer boolean flag and reset offset cursor;
_sw_: UART5.SendChar(0x06)
_sw_: s_current_buf = !s_current_buf
_sw_: s_offset = 0
-> RETURN(byte)