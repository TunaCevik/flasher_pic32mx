#ifndef PSEUDO_OPERATIONS_H
#define PSEUDO_OPERATIONS_H

#include <stdint.h>
#include "flash_pic32mx/tap_controller.h"

/* EJTAG / MTAP pseudo operations, per PIC32 Flash Programming Specification,
 * section 6 ("Pseudo Operations"). The TAP instruction/command constants
 * Required definitions live in tap_controller.h. */

void SetMode(uint32_t mode, uint8_t num_bits);
void SendCommand(uint8_t command);
uint32_t XferData(uint32_t iData);
uint32_t XferFastData(uint32_t iData);
void XferInstruction(uint32_t instruction);
uint32_t ReadFromAddress(uint32_t address);
void Synchronize(void);

#endif /* PSEUDO_OPERATIONS_H */
