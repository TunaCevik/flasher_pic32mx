Fast 32-bit transfer machine with PrAcc CPU polling handshake (Section 6.4). Retries transfer from Run-Test-Idle if target processor is busy.

- **Source File:** `src/flash_pic32mx/pseudo_operations.c` (`XferFastData()`)
    
- **Hardware Binding:** [[HardwareBindings/PIC32_EJTAG_PRIMITIVES]]


SUB_MACHINE: EJTAG_XFER_FASTDATA
INPUTS:  iData (uint32_t)
OUTPUTS: oData (uint32_t)

[NAVIGATE_TO_SHIFT_DR].
  ;TMS Header = 100 (Run-Test-Idle -> Select-DR-Scan -> Capture-DR -> Shift-DR);
  _sw_: ICSP_TransferBit(TDI=0, TMS=1)
  _sw_: ICSP_TransferBit(TDI=0, TMS=0)
  _sw_: oPrAcc = ICSP_TransferBit(TDI=0, TMS=0)
  <oPrAcc == 1>
    YES -> [TRANSFER_FASTDATA_BODY]
    NO  -> [ABORT_AND_RETRY]

[ABORT_AND_RETRY].
  ;CPU busy (PrAcc == 0). Exit to Run-Test-Idle and retry;
  _sw_: ICSP_TransferBit(TDI=0, TMS=1) ;Exit to Update-DR;
  _sw_: ICSP_TransferBit(TDI=0, TMS=0) ;Return to Run-Test-Idle;
  ⌛ DELAY(1us)
  -> [NAVIGATE_TO_SHIFT_DR]

[TRANSFER_FASTDATA_BODY].
  ;Clock out first bit;
  _sw_: oData[0] = ICSP_TransferBit(TDI=0, TMS=0)
  ;Stream 31 data bits (LSB-first, TMS=0);
  LOOP (i = 0 to 30):
    _sw_: oData[i+1] = ICSP_TransferBit(TDI=iData[i], TMS=0)
  ;Clock MSB with TMS=1 to transition to Exit1-DR;
  _sw_: ICSP_TransferBit(TDI=iData[31], TMS=1)
  ;TMS Footer = 10 (Update-DR -> Run-Test-Idle);
  _sw_: ICSP_TransferBit(TDI=0, TMS=1)
  _sw_: ICSP_TransferBit(TDI=0, TMS=0)
  -> RETURN(oData)