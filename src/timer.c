#include "timer.h"

volatile uint32_t sys_tick_count = 0;

void timer_init(void) {
    LL_Init1msTick(180000000);
    LL_SYSTICK_EnableIT();

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void SysTick_Handler(void) {
    sys_tick_count++;
}

void timer_delay_ms(uint32_t ms) {
    uint32_t start_time = sys_tick_count;
    while ((sys_tick_count - start_time) < ms);
}

void timer_delay_us(uint32_t us) {
    uint32_t ticks = us * (SystemCoreClock / 1000000U);
    uint32_t start = DWT->CYCCNT;
    
    while ((DWT->CYCCNT - start) < ticks);
}
