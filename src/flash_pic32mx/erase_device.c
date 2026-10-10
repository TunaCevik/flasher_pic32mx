#include "flash_pic32mx/erase_device.h"
#include "flash_pic32mx/pseudo_operations.h"
#include "timer.h"

device_status_t EraseDevice(void)
{
    device_status_t result = {0};
    uint32_t elapsed_ms = 0;

    /* [ENTER_MTAP_COMMAND_MODE] */
    SendCommand(MTAP_SW_MTAP);
    SetMode(0x1FUL, 6);
    SendCommand(MTAP_COMMAND);

    /* [ISSUE_CHIP_ERASE] */
    (void)XferData(MCHP_ERASE);

    for (;;) {
        /* [WAIT_ERASE_INTERVAL] */
        (void)XferData(MCHP_DE_ASSERT_RST);
        timer_delay_ms(ERASE_DEVICE_POLL_MS);
        elapsed_ms += ERASE_DEVICE_POLL_MS;

        /* [POLL_ERASE_STATUS] */
        result.status_val = XferData(MCHP_STATUS);
        uint8_t is_ready = (uint8_t)(((result.status_val & MCHP_STATUS_CFGRDY_MASK) != 0) &&
                                     ((result.status_val & MCHP_STATUS_FCBUSY_MASK) == 0));
        uint8_t is_timeout = (uint8_t)(elapsed_ms >= ERASE_DEVICE_TIMEOUT_MS);

        if (is_ready || is_timeout) {
            result.ready = is_ready;
            return result;
        }
    }
}
