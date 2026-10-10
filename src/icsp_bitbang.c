#include "icsp_bitbang.h"

void ICSP_Set_PGC(uint8_t state)
{
    if (state) {
        LL_GPIO_SetOutputPin(ICSP_PORT, ICSP_PIN_PGC);
    } else {
        LL_GPIO_ResetOutputPin(ICSP_PORT, ICSP_PIN_PGC);
    }
}

void ICSP_Set_PGD_Output(uint8_t state)
{
    if (LL_GPIO_GetPinMode(ICSP_PORT, ICSP_PIN_PGD) != LL_GPIO_MODE_OUTPUT) {
        LL_GPIO_SetPinMode(ICSP_PORT, ICSP_PIN_PGD, LL_GPIO_MODE_OUTPUT);
    }

    if (state) {
        LL_GPIO_SetOutputPin(ICSP_PORT, ICSP_PIN_PGD);
    } else {
        LL_GPIO_ResetOutputPin(ICSP_PORT, ICSP_PIN_PGD);
    }
}

void ICSP_Set_PGD_Input(void)
{
    LL_GPIO_SetPinMode(ICSP_PORT, ICSP_PIN_PGD, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(ICSP_PORT, ICSP_PIN_PGD, LL_GPIO_PULL_NO);
}

uint8_t ICSP_Read_PGD(void)
{
    return (LL_GPIO_IsInputPinSet(ICSP_PORT, ICSP_PIN_PGD) ? 1 : 0);
}
