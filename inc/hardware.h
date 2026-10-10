// inc/hardware.h
#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>
#include "stm32f446xx.h"

// Common LL modules nearly every file will need
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_cortex.h"

// icsp_bitbang
// PD0/PD1/PD3 are not bonded out on the STM32F446RE's LQFP64 package used on
// the Nucleo-64 board -- confirmed absent from both the Arduino and Morpho
// header pinout tables (UM1724). Reassigned to PB13/PB14/PB15 (Morpho CN10),
// which are free and physically adjacent for easy wiring.
#define ICSP_PORT         GPIOB
#define ICSP_PIN_PGC      LL_GPIO_PIN_13
#define ICSP_PIN_PGD      LL_GPIO_PIN_14

// enter_enhanced_icsp
#define MCLR_PIN  LL_GPIO_PIN_15
#define MCLR_PORT           GPIOB
/* PIC32 Enhanced ICSP Entry Key: "MCHP" (0x4D434850) */
#define ICSP_KEY_SEQUENCE 0x4D434850UL


#define SYSTEM_CORE_CLOCK 180000000U // 180 MHz

#endif // HARDWARE_H
