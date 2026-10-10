#ifndef ENTER_SERIAL_EXEC_H
#define ENTER_SERIAL_EXEC_H

/* Section 10.2: Entering Serial Execution Mode (2-Wire)
 * Verifies Code Protection State (CPS) and boots the CPU into Serial Execution Mode. */

typedef enum {
    ENTER_SERIAL_EXEC_OK = 0,
    ENTER_SERIAL_EXEC_CODE_PROTECTED = 1,
} enter_serial_exec_status_t;

/**
 * @brief Transitions target into EJTAG Serial Execution mode.
 * @pre Target must be in Enhanced ICSP mode (EnterEnhancedICSP).
 */
enter_serial_exec_status_t EnterSerialExecMode(void);

#endif /* ENTER_SERIAL_EXEC_H */
