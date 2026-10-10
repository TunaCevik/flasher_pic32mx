#include "flash_pic32mx/check_device_status.h"
#include "flash_pic32mx/pseudo_operations.h"
#include "timer.h"

device_status_t CheckDeviceStatus(void)
{
    device_status_t result = {0};
    uint32_t elapsed_us = 0;

    /* [SWITCH_TO_MTAP] */
    SetMode(0x1FUL, 6);
    SendCommand(MTAP_SW_MTAP);

    /* [SELECT_COMMAND_REG] */
    SetMode(0x1FUL, 6);
    SendCommand(MTAP_COMMAND);

    /* [POLL_DEVICE_READY] */
    for (;;) {
        result.status_val = XferData(MCHP_STATUS);
        result.ready = (uint8_t)(((result.status_val & MCHP_STATUS_CFGRDY_MASK) != 0) &&
                                 ((result.status_val & MCHP_STATUS_FCBUSY_MASK) == 0));

        if (result.ready || elapsed_us >= CHECK_DEVICE_STATUS_TIMEOUT_US) {
            break;
        }

        /* [RETRY_OR_TIMEOUT] */
        SendCommand(MTAP_COMMAND);
        timer_delay_us(CHECK_DEVICE_STATUS_POLL_US);
        elapsed_us += CHECK_DEVICE_STATUS_POLL_US;
    }

    return result;
}
