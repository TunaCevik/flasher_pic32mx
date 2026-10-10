#include "flash_pic32mx/exit_programming_mode.h"
#include "flash_pic32mx/enter_enhanced_icsp.h"
#include "flash_pic32mx/pseudo_operations.h"
#include "icsp_bitbang.h"
#include "timer.h"

/* No minimum is given for P16 (Table 21-1, "Delay between Last PGECx-down
 * and MCLR-down": 0 s), so this is just the pulse's own generous margin,
 * matching the 1 us convention used elsewhere in this project. */
#define EXIT_CLOCK_PULSE_US 1U

/* Spec gives no explicit minimum for how long MCLR must stay low before
 * being released back high — this just mirrors the entry-side MCLR pulse
 * width convention (ICSP_ENTRY_MCLR_PULSE_US in enter_enhanced_icsp.c). */
#define EXIT_MCLR_RESET_PULSE_US 100U

void ExitProgrammingMode(void)
{
    /* Step 1: force the Chip TAP controller into Run-Test/Idle. Section
     * 16.2 specifies this as a 5-bit SetMode, unlike the 6-bit
     * SetMode(6'b011111) used everywhere else in this project (Sections
     * 8-10) -- transcribed as given rather than "corrected" to match those. */
    SetMode(0x1FUL, 5);

    /* Steps 2-3 (reordered from the numbered list — see exit_programming_mode.h):
     * issue the final clock pulse on PGECx while MCLR is still at VIH. */
    ICSP_Set_PGC(1);
    timer_delay_us(EXIT_CLOCK_PULSE_US);
    ICSP_Set_PGC(0);
    timer_delay_us(EXIT_CLOCK_PULSE_US);

    /* Then assert (drive low) MCLR, removing VIH and ending the ICSP
     * session. */
    LL_GPIO_ResetOutputPin(MCLR_PORT, MCLR_PIN);
    timer_delay_us(EXIT_MCLR_RESET_PULSE_US);

    /* Release MCLR back high so the target actually comes out of reset and
     * boots the newly programmed code, instead of sitting held low forever
     * (MCLR is a plain push-pull output here, not open-drain/pulled-up --
     * nothing else will raise it if this doesn't). */
    LL_GPIO_SetOutputPin(MCLR_PORT, MCLR_PIN);

    /* Step 4: "Remove power (if the device is powered)" — this project has
     * no target-VDD power-switch GPIO (only MCLR/PGC/PGD are driven), so
     * cycling target power is outside this function's scope and is left to
     * the caller/hardware. */
}
