| **Register**       | **Bit / Field** | **Value**    | **Purpose**                          |
| ------------------ | --------------- | ------------ | ------------------------------------ |
| **`RCC->AHB1ENR`** | `DMA1EN`        | `1`          | Enables DMA1 peripheral bus clock    |
| **`DMA_SxCR`**     | `CHSEL[2:0]`    | `100b` (4)   | Selects UART5_RX trigger line        |
| **`DMA_SxCR`**     | `PL[1:0]`       | `10b`        | High priority stream arbitration     |
| **`DMA_SxCR`**     | `DIR[1:0]`      | `00b`        | Peripheral-to-memory transfer        |
| **`DMA_SxCR`**     | `CIRC`          | `1`          | Circular mode enabled                |
| **`DMA_SxCR`**     | `DBM`           | `1`          | Double Buffer Mode enabled           |
| **`DMA_SxCR`**     | `MINC`          | `1`          | Memory address increments by 1 byte  |
| **`DMA_SxCR`**     | `PINC`          | `0`          | Peripheral address fixed (UART5->DR) |
| **`DMA_SxNDTR`**   | `NDTR[15:0]`    | `128`        | Transaction counter reload value     |
| **`DMA_SxPAR`**    | `PAR[31:0]`     | `&UART5->DR` | Peripheral data register source      |
| **`DMA_SxM0AR`**   | `M0AR[31:0]`    | `&s_buf1`    | Target memory buffer 0 (Ping)        |
| **`DMA_SxM1AR`**   | `M1AR[31:0]`    | `&s_buf2`    | Target memory buffer 1 (Pong)        |