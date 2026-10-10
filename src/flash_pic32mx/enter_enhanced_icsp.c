#include "flash_pic32mx/enter_enhanced_icsp.h"
#include "icsp_bitbang.h"
#include "timer.h"

#define ICSP_ENTRY_MIN_DELAY_US   1U   /* clears P6, P18, P1/P1A/P1B, P19, P7 */
#define ICSP_ENTRY_MCLR_PULSE_US 100U  /* P20: MCLR High Time, max 500 us */

static inline void MCLR_Set(uint8_t state)
{
    if (state) {
        LL_GPIO_SetOutputPin(MCLR_PORT, MCLR_PIN);
    } else {
        LL_GPIO_ResetOutputPin(MCLR_PORT, MCLR_PIN);
    }
}

static inline void ICSP_ClockKeyBit(uint8_t bit)
{
    ICSP_Set_PGD_Output(bit);
    timer_delay_us(ICSP_ENTRY_MIN_DELAY_US);

    ICSP_Set_PGC(1);
    timer_delay_us(ICSP_ENTRY_MIN_DELAY_US);

    ICSP_Set_PGC(0);
    timer_delay_us(ICSP_ENTRY_MIN_DELAY_US);
}

void EnterEnhancedICSP(void)
{
    /* [RESET_BUS_LINES] */
    ICSP_Set_PGC(0);
    ICSP_Set_PGD_Output(0);
    timer_delay_us(ICSP_ENTRY_MIN_DELAY_US);

    /* [DRIVE_INITIAL_MCLR_PULSE] */
    MCLR_Set(1);
    timer_delay_us(ICSP_ENTRY_MCLR_PULSE_US);
    MCLR_Set(0);
    timer_delay_us(ICSP_ENTRY_MIN_DELAY_US);

    /* [SHIFT_KEY_SEQUENCE] */
    for (int8_t bit_index = 31; bit_index >= 0; bit_index--) {
        uint8_t key_bit = (uint8_t)((ICSP_KEY_SEQUENCE >> bit_index) & 0x01UL);
        ICSP_ClockKeyBit(key_bit);
    }

    /* [LATCH_ENHANCED_MODE] */
    timer_delay_us(ICSP_ENTRY_MIN_DELAY_US);
    MCLR_Set(1);
    timer_delay_us(ICSP_ENTRY_MIN_DELAY_US);
}
