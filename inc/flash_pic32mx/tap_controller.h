#ifndef TAP_CONTROLLER_H
#define TAP_CONTROLLER_H

#include <stdint.h>
/* Section 20.0 "TAP Controllers", PIC32 Flash Programming Specification.
 * Single source of truth for every TAP-level instruction/command constant
 * used by pseudo_operations.c and the programming-flow steps built on it
 * (enter_enhanced_icsp.c, check_device_status.c, erase_device.c,
 * enter_serial_exec.c). Previously these were split across
 * pseudo_operations.h and a separate mtap_commands.h; both were
 * consolidated into this single file. */

/* Table 20-1: MCHP TAP Instructions (5-bit). Selects which TAP register
 * XferData()/XferInstruction() talk to; NOT the same namespace as the
 * MTAP_COMMAND DR Commands below, which are 8-bit op-codes shifted into the
 * register MTAP_COMMAND selects. */
#define MTAP_COMMAND 0x07U /* TDI/TDO connected to MCHP Command Shift register (Table 20-2) */
#define MTAP_SW_MTAP 0x04U /* Switch TAP controller to the MCHP TAP controller */
#define MTAP_SW_ETAP 0x05U /* Switch TAP controller to the EJTAG TAP controller */
#define MTAP_IDCODE  0x01U /* Select Device Identification Data register */

/* Table 20-4: EJTAG TAP Instructions (5-bit). */
#define ETAP_ADDRESS   0x08U /* Select Address register */
#define ETAP_DATA      0x09U /* Select Data register */
#define ETAP_CONTROL   0x0AU /* Select EJTAG Control register (Register 20-1) */
#define ETAP_EJTAGBOOT 0x0CU /* Set EjtagBrk, ProbEn and ProbTrap to '1' as the Reset value */
#define ETAP_FASTDATA  0x0EU /* Select the Data and Fastdata registers */

/* Not in the Table 20-4 excerpt above (which only lists the EJTAG
 * instructions Section 20.2 documents), but part of the wider EJTAG TAP
 * instruction set referenced elsewhere in the spec (Section 6, "Pseudo
 * Operations"); kept here since pseudo_operations.c/.h already relied on
 * them before this file existed. */
#define ETAP_ALL        0x0BU
#define ETAP_NORMALBOOT 0x0DU

/* Table 20-2: MTAP_COMMAND DR Commands. 8-bit op-codes shifted into the MCHP
 * Command Shift register (via XferData()) once MTAP_COMMAND has selected
 * it. */
#define MCHP_STATUS        0x00UL /* NOP and return Status (Table 20-3) */
#define MCHP_ASSERT_RST    0xD1UL /* Request the reset controller to assert device Reset */
#define MCHP_DE_ASSERT_RST 0xD0UL /* Remove the request for device Reset */
#define MCHP_ERASE         0xFCUL /* Cause the Flash controller to perform a Chip Erase */
#define MCHP_FLASH_ENABLE  0xFEUL /* Enable fetches/loads to Flash from the processor (not required for PIC32MK/MZ) */
#define MCHP_FLASH_DISABLE 0xFDUL /* Disable fetches/loads to Flash from the processor (not required for PIC32MK/MZ) */

/* Table 20-3: MCHP Status Value bit fields — the word XferData(MCHP_STATUS)
 * (or any other MTAP_COMMAND DR command) returns. */
#define MCHP_STATUS_CPS_MASK    (1UL << 7) /* Code-Protect State: 1 = not code-protected */
#define MCHP_STATUS_NVMERR_MASK (1UL << 5) /* NVMCON error during last NVM op (not on PIC32MX320/340/360/420/440/460) */
#define MCHP_STATUS_CFGRDY_MASK (1UL << 3) /* Configuration has been read and CP is valid */
#define MCHP_STATUS_FCBUSY_MASK (1UL << 2) /* Flash Controller Busy (1 = erase in progress) */
#define MCHP_STATUS_FAEN_MASK   (1UL << 1) /* Flash Access Enable, mirrors CFGCON.FAEN (not on PIC32MK/MZ) */
#define MCHP_STATUS_DEVRST_MASK (1UL << 0) /* Device Reset is active */

/* Common device status structure for MTAP polling */
typedef struct {
    uint32_t status_val; /* Raw MCHP_STATUS register value */
    uint8_t ready;       /* 1 if (CFGRDY == 1 && FCBUSY == 0), otherwise 0 */
} device_status_t;

#endif /* TAP_CONTROLLER_H */
