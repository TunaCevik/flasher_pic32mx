#ifndef ENTER_ENHANCED_ICSP_H
#define ENTER_ENHANCED_ICSP_H

#include "stm32f4xx_ll_gpio.h"
#include "hardware.h"

/**
 * @brief Enters PIC32 2-Wire Enhanced ICSP mode.
 * @note  Asserts MCLR pulse, clocks the 32-bit key sequence, and latches VIH.
 */
void EnterEnhancedICSP(void);

#endif /* ENTER_ENHANCED_ICSP_H */
