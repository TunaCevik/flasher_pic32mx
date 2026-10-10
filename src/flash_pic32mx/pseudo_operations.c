#include "flash_pic32mx/pseudo_operations.h"
#include "icsp_bitbang.h"
#include "timer.h"

/* Minimum clock half-period / setup-hold margin for the bit-banged PGC/PGD
 * link. Generous on purpose since these pins are driven by a fast MCU
 * talking to a comparatively slow PIC32 TAP controller. */
#define ICSP_DELAY_US 1U

static void ICSP_ClockPulse(void)
{
    ICSP_Set_PGC(1);
    timer_delay_us(ICSP_DELAY_US);
    ICSP_Set_PGC(0);
    timer_delay_us(ICSP_DELAY_US);
}

static void ICSP_ClockHigh(void)
{
    ICSP_Set_PGC(1);
    timer_delay_us(ICSP_DELAY_US);
}

static void ICSP_ClockLow(void)
{
    ICSP_Set_PGC(0);
    timer_delay_us(ICSP_DELAY_US);
}

/* One 2-wire "4-phase" bit cell: drive TDI, drive TMS, then release PGD and
 * sample TDO — each phase gated by its own PGC clock pulse. This is the
 * primitive every pseudo operation below is built from. */
static uint8_t ICSP_TransferBit(uint8_t tdi, uint8_t tms)
{
    uint8_t tdo;

    // Phase 1: TDI
    ICSP_Set_PGD_Output(tdi & 0x01U);
    ICSP_ClockPulse();

    // Phase 2: TMS
    ICSP_Set_PGD_Output(tms & 0x01U);
    ICSP_ClockPulse();

    // Phase 3: Execute
    ICSP_ClockPulse();

    // Phase 4: TDO
    ICSP_Set_PGD_Input();
    ICSP_ClockHigh();
    tdo = ICSP_Read_PGD();
    ICSP_ClockLow();

    return tdo;
}

/* 6.1 SetMode(mode) — clock num_bits mode bits (LSb first) into the EJTAG
 * TAP state machine on TMS, holding TDI at 0. TDO is ignored. */
void SetMode(uint32_t mode, uint8_t num_bits)
{
    for (uint8_t i = 0; i < num_bits; i++) {
        uint8_t tms_bit = (uint8_t)((mode >> i) & 0x01U);
        (void)ICSP_TransferBit(0, tms_bit);
    }
}

/* 6.2 SendCommand(command) — select a TAP register (5-bit command). */
void SendCommand(uint8_t command)
{
    /* TMS Header = 1100: Run-Test-Idle -> ... -> Shift-IR */
    (void)ICSP_TransferBit(0, 1);
    (void)ICSP_TransferBit(0, 1);
    (void)ICSP_TransferBit(0, 0);
    (void)ICSP_TransferBit(0, 0);

    /* Command bits, LSb first, TMS held low ... */
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t tdi_bit = (uint8_t)((command >> i) & 0x01U);
        (void)ICSP_TransferBit(tdi_bit, 0);
    }
    /* ... except the MSb, clocked in with TMS = 1 to exit Shift-IR. */
    uint8_t msb = (uint8_t)((command >> 4) & 0x01U);
    (void)ICSP_TransferBit(msb, 1);

    /* TMS Footer = 10: Update-IR -> Run-Test-Idle */
    (void)ICSP_TransferBit(0, 1);
    (void)ICSP_TransferBit(0, 0);
}

/* 6.3 XferData(iData) — clock 32 bits to/from the register selected by the
 * last SendCommand. */
uint32_t XferData(uint32_t iData)
{
    uint32_t oData = 0;
    uint8_t tdo;

    /* TMS Header = 100: Run-Test-Idle -> ... -> Shift-DR */
    (void)ICSP_TransferBit(0, 1);
    (void)ICSP_TransferBit(0, 0);
    (void)ICSP_TransferBit(0, 0);

    /* Data bits, LSb first, TMS held low ... */
    for (uint8_t i = 0; i < 31; i++) {
        tdo = ICSP_TransferBit((uint8_t)((iData >> i) & 0x01U), 0);
        oData |= ((uint32_t)tdo << i);
    }
    /* ... except the MSb, clocked in with TMS = 1 to exit Shift-DR. */
    tdo = ICSP_TransferBit((uint8_t)((iData >> 31) & 0x01U), 1);
    oData |= ((uint32_t)tdo << 31);

    /* TMS Footer = 10: Update-DR -> Run-Test-Idle */
    (void)ICSP_TransferBit(0, 1);
    (void)ICSP_TransferBit(0, 0);

    return oData;
}

/* 6.4 XferFastData(iData) — fast 32-bit transfer to/from the Fastdata
 * register. The Fastdata register must already be selected via
 * SendCommand(ETAP_FASTDATA). The PE's PrAcc handshake bit is checked on
 * every attempt; the whole shift is retried (back through Run-Test-Idle)
 * until the Processor Access bit reads back as 1. */
uint32_t XferFastData(uint32_t iData)
{
    uint32_t oData = 0;
    uint8_t oPrAcc;
    uint8_t tdo;

    do {
        /* TMS Header = 100: Run-Test-Idle -> ... -> Shift-DR.
         * On the last header clock, TDO returns the PrAcc bit. */
        (void)ICSP_TransferBit(0, 1);
        (void)ICSP_TransferBit(0, 0);
        oPrAcc = ICSP_TransferBit(0, 0);

        /* If Processor Access bit is 0, CPU is busy. 
         * Immediately abort the shift, go to Update-DR, and retry. */
        if (oPrAcc != 1) {
            (void)ICSP_TransferBit(0, 1); // Exit to Update-DR with TMS=1
            (void)ICSP_TransferBit(0, 0); // Return to Run-Test-Idle
            continue;                     // Jump straight to the next loop iteration
        }

        /* --- Only runs if PrAcc == 1 (CPU is ready) --- */
        oData = 0;

        /* PrAcc input bit is always 0; TDO here is output data bit 0. */
        tdo = ICSP_TransferBit(0, 0);
        oData |= ((uint32_t)tdo << 0);

        /* 31 bits of data, TMS held low; TDO shifts out bits 1..31. */
        for (uint8_t i = 0; i < 31; i++) {
            tdo = ICSP_TransferBit((uint8_t)((iData >> i) & 0x01U), 0);
            oData |= ((uint32_t)tdo << (i + 1));
        }

        /* MSb of input data, clocked in with TMS = 1 to exit Shift-DR. */
        (void)ICSP_TransferBit((uint8_t)((iData >> 31) & 0x01U), 1);

        /* TMS Footer = 10: Update-DR -> Run-Test-Idle */
        (void)ICSP_TransferBit(0, 1);
        (void)ICSP_TransferBit(0, 0);

    } while (oPrAcc != 1);

    return oData;
}

/* 6.5 XferInstruction(instruction) — send a 32-bit MIPS instruction for the
 * CPU to execute (device must already be in Debug mode). */
void XferInstruction(uint32_t instruction)
{
    uint32_t controlVal;

    /* Select Control Register, wait until CPU is ready
     * (Processor Access bit 18 set). */
    SendCommand(ETAP_CONTROL);
    do {
        controlVal = XferData(0x0004C000U);
    } while (((controlVal >> 18) & 0x01U) != 1U);

    /* Select Data Register, send the instruction. */
    SendCommand(ETAP_DATA);
    (void)XferData(instruction);

    /* Tell the CPU to execute the instruction. */
    SendCommand(ETAP_CONTROL);
    (void)XferData(0x0000C000U);
}

/* 6.6 ReadFromAddress(address) — read a 32-bit word from device memory
 * (PIC32MX/MZ/MK variant, per the Flash Programming Specification). */
uint32_t ReadFromAddress(uint32_t address)
{
    uint32_t instruction;
    uint32_t oData;

    /* lui s3, <FAST_DATA_REG_ADDRESS(31:16)> - set address of fast
     * data register */
    instruction = 0x3c130000U;
    instruction |= (0xff200000U >> 16) & 0x0000ffffU;
    XferInstruction(instruction);

    /* lui t0, <address(31:16)> - set address of data */
    instruction = 0x3c080000U;
    instruction |= (address >> 16) & 0x0000ffffU;
    XferInstruction(instruction);

    /* ori t0, <address(15:0)> - set address of data */
    instruction = 0x35080000U;
    instruction |= (address & 0x0000ffffU);
    XferInstruction(instruction);

    /* lw t1, 0(t0) */
    XferInstruction(0x8d090000U);

    /* sw t1, 0(s3) - store data to fast data register */
    XferInstruction(0xae690000U);

    /* nop */
    XferInstruction(0x00000000U);

    /* Shift out the data */
    SendCommand(ETAP_FASTDATA);
    oData = XferFastData(0x00000000U);

    return oData;
}

/* 6.7 Synchronize() — reset the EJTAG state machine into Test-Logic-Reset by
 * holding PGEDx high for 24 PGECx clocks. */
void Synchronize(void)
{
    ICSP_Set_PGD_Output(1);
    timer_delay_us(ICSP_DELAY_US);

    for (uint8_t i = 0; i < 24; i++) {
        ICSP_ClockPulse();
    }
}
