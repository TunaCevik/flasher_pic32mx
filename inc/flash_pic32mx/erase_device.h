#ifndef ERASE_DEVICE_H
#define ERASE_DEVICE_H

#include "flash_pic32mx/tap_controller.h"

#define ERASE_DEVICE_POLL_MS    10U  /* 10 ms wait between poll attempts */
#define ERASE_DEVICE_TIMEOUT_MS 500U /* Max execution time limit */

/**
 * @brief Issues chip erase command and waits for operation completion.
 * @pre Target must be in Enhanced ICSP mode (EnterEnhancedICSP).
 */
device_status_t EraseDevice(void);

#endif /* ERASE_DEVICE_H */
