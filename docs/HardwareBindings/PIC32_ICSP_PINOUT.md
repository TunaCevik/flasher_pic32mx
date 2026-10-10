| **Signal Name** | **STM32 Pin** | **Mode**                 | **Speed / Drive**    | **Target Pin** | **Target Function**         |
| --------------- | ------------- | ------------------------ | -------------------- | -------------- | --------------------------- |
| **`PGC`**       | `PB13`        | Output Push-Pull         | High Speed (No-Pull) | `PGEC1/2`      | Serial Programming Clock    |
| **`PGD`**       | `PB14`        | Output Push-Pull / Input | High Speed (No-Pull) | `PGED1/2`      | Bidirectional Serial Data   |
| **`MCLR`**      | `PB15`        | Output Push-Pull         | High Speed (No-Pull) | `MCLR`         | Target Active-Low Reset     |
| **`UART5_RX`**  | `PD2`         | AF8 (Alternate Function) | High Speed (Pull-Up) | PC USB-UART    | Stream data reception       |
| **`UART5_TX`**  | `PC12`        | AF8 (Alternate Function) | High Speed (Pull-Up) | PC USB-UART    | ACK (0x06) and log transmit |