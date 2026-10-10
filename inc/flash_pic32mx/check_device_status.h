#ifndef CHECK_DEVICE_STATUS_H
#define CHECK_DEVICE_STATUS_H

#include "flash_pic32mx/tap_controller.h"

#define CHECK_DEVICE_STATUS_TIMEOUT_US 10000UL /* 10 ms max per spec recommendation */
#define CHECK_DEVICE_STATUS_POLL_US      100UL /* Polling interval */

/**
 * @brief Selects MTAP and polls status until ready or timeout expires.
 * @pre Target must be in Enhanced ICSP mode (EnterEnhancedICSP).
 */
device_status_t CheckDeviceStatus(void);

#endif /* CHECK_DEVICE_STATUS_H */
