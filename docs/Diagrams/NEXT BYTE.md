# NEXT BYTE — Diagram

Visual companion to [[DMA_BYTE_SOURCE]] (`NextByte()`).

```mermaid
flowchart TD
    Start(["DMA_BYTE_SOURCE()"]) --> CheckBoundary{"s_offset == 0?"}

    CheckBoundary -->|YES| PollFlip["POLL_DMA_TARGET_FLIP<br/>_hw_: dma_active_buf = READ(DMA1_Stream0.CR.CT)"]
    PollFlip --> FlipCheck{"⌛ dma_active_buf == s_current_buf?"}
    FlipCheck -->|YES — DMA still filling same buffer| PollFlip
    FlipCheck -->|NO — DMA flipped targets| FetchByte

    CheckBoundary -->|NO| FetchByte["FETCH_BYTE_FROM_BUFFER<br/>byte = s_current_buf ? s_buf2[s_offset] : s_buf1[s_offset]<br/>s_offset += 1"]

    FetchByte --> BoundaryCheck{"s_offset == 128?"}
    BoundaryCheck -->|NO| ReturnByte(["RETURN(byte)"])
    BoundaryCheck -->|YES| FlipAck["FLIP_BUFFER_AND_ACK<br/>_sw_: Driver::UART5 -> SendChar(0x06)<br/>s_current_buf = !s_current_buf<br/>s_offset = 0"]
    FlipAck --> ReturnByte
```

**Notes**

- Entry condition `s_offset == 0` is the buffer-boundary check: the consumer only needs to confirm the DMA engine has moved on to the *other* buffer when it's about to start reading a fresh one.
- `POLL_DMA_TARGET_FLIP` is a tight spin-poll — it blocks until hardware (`DMA1_Stream0->CR.CT`) reports the opposite half of the ping-pong buffer is now the active DMA target, guaranteeing the half we're about to read is fully written and stable.
- The `0x06` (ACK) byte sent in `FLIP_BUFFER_AND_ACK` is the host-side pacing token — see [[HSI/DMA_PINGPONG_INTERFACE]] for the full handshake contract.
