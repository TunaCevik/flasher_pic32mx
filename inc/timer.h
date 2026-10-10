#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"

void timer_init(void);
void timer_delay_ms(uint32_t ms);
void timer_delay_us(uint32_t us);

#endif /* TIMER_H */
