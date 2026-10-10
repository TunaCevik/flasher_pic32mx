#ifndef ICSP_BITBANG_H
#define ICSP_BITBANG_H

#include "stm32f4xx_ll_gpio.h"
#include "hardware.h"

// BitBang operations provider
void ICSP_Set_PGC(uint8_t state);
void ICSP_Set_PGD_Output(uint8_t state);
void ICSP_Set_PGD_Input(void);
uint8_t ICSP_Read_PGD(void);

#endif /* ICSP_BITBANG_H */
