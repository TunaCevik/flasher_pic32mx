#include <stdbool.h>
#include "flash_engine.h"
#include "flash_pic32mx/enter_enhanced_icsp.h"
#include "flash_pic32mx/check_device_status.h"
#include "flash_pic32mx/erase_device.h"
#include "flash_pic32mx/enter_serial_exec.h"
#include "flash_pic32mx/download_pe.h"
#include "flash_pic32mx/pic32_pe.h"
#include "flash_pic32mx/exit_programming_mode.h"
#include "uart_dma_function_manager.h"
#include "init/init_usart.h"
#include "stm32f4xx_ll_dma.h"

typedef enum {
    FLASH_STATE_ENTER_ENHANCED_ICSP,
    FLASH_STATE_CHECK_DEVICE_STATUS,
    FLASH_STATE_ERASE_DEVICE,
    FLASH_STATE_ENTER_SERIAL_EXEC_MODE,
    FLASH_STATE_DOWNLOAD_PE,
    FLASH_STATE_DOWNLOAD_FIRMWARE,
    FLASH_STATE_VERIFY_DEVICE,
    FLASH_STATE_WIPE_ON_VERIFY_FAIL,
    FLASH_STATE_EXIT_PROGRAMMING_MODE,
    FLASH_STATE_FAIL_ABORT,
    FLASH_STATE_IDLE,
} flash_engine_state_t;

/* Ensure hardware peripherals and target pins are left in a safe state */
static void AbortAndCleanup(void)
{
    /* Disable DMA stream to halt background transfers */
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_0);

    /* Release target from EJTAG/Reset so it doesn't freeze in ICSP mode */
    ExitProgrammingMode();
    UART5_SendString("Drop the VDD low\r\n");
}

void FlashEngine_Run(void)
{
    flash_engine_state_t state = FLASH_STATE_ENTER_ENHANCED_ICSP;
    uart_dma_transfer_summary_t firmware_summary = {0};
    bool serial_exec_erase_retried = false;

    for (;;) {
        switch (state) {

        case FLASH_STATE_ENTER_ENHANCED_ICSP:
            EnterEnhancedICSP();
            UART5_SendString("STATUS: Entered Enhanced ICSP\r\n");
            state = FLASH_STATE_CHECK_DEVICE_STATUS;
            break;

        case FLASH_STATE_CHECK_DEVICE_STATUS: {
            device_status_t result = CheckDeviceStatus();
            if (result.ready) {
                UART5_SendString("STATUS: Device Ready\r\n");
                state = FLASH_STATE_ERASE_DEVICE;
            } else {
                UART5_SendString("ERR: Device Not Ready\r\n");
                state = FLASH_STATE_FAIL_ABORT;
            }
            break;
        }

        case FLASH_STATE_ERASE_DEVICE: {
            device_status_t result = EraseDevice();
            if (result.ready) {
                UART5_SendString("STATUS: Erase Complete\r\n");
                state = FLASH_STATE_ENTER_SERIAL_EXEC_MODE;
            } else {
                UART5_SendString("ERR: Erase Failed\r\n");
                state = FLASH_STATE_FAIL_ABORT;
            }
            break;
        }

        case FLASH_STATE_ENTER_SERIAL_EXEC_MODE: {
            enter_serial_exec_status_t result = EnterSerialExecMode();
            if (result == ENTER_SERIAL_EXEC_OK) {
                UART5_SendString("STATUS: Serial Exec Active\r\n");
                state = FLASH_STATE_DOWNLOAD_PE;
            } else if ((result == ENTER_SERIAL_EXEC_CODE_PROTECTED) && !serial_exec_erase_retried) {
                /* Figure 10-1: "Cannot Enter. Must Erase First." Erase once
                 * and retry entry; if it's still protected afterward, treat
                 * it as a real failure instead of erasing forever. */
                UART5_SendString("STATUS: Code-Protected, Erasing and Retrying\r\n");
                serial_exec_erase_retried = true;
                state = FLASH_STATE_ERASE_DEVICE;
            } else {
                UART5_SendString("ERR: Failed to Enter Serial Exec\r\n");
                state = FLASH_STATE_FAIL_ABORT;
            }
            break;
        }

        case FLASH_STATE_DOWNLOAD_PE: {
            DownloadPE_InitBMX();
            DownloadPE_LoadLoader();

            uart_dma_result_t result = UART_DMA_RunTransfer(UART_DMA_MODE_LOAD_PE, NULL);
            if (result == UART_DMA_RESULT_OK) {
                DownloadPE_JumpToPE();
                UART5_SendString("STATUS: PE Bootstrap Running\r\n");
                state = FLASH_STATE_DOWNLOAD_FIRMWARE;
            } else {
                UART5_SendString("ERR: PE Load Aborted\r\n");
                state = FLASH_STATE_FAIL_ABORT;
            }
            break;
        }

        case FLASH_STATE_DOWNLOAD_FIRMWARE: {
            uart_dma_result_t result =
                UART_DMA_RunTransfer(UART_DMA_MODE_PROGRAM_FLASH, &firmware_summary);

            if (result == UART_DMA_RESULT_OK) {
                UART5_SendString("STATUS: Firmware Flashed\r\n");
                state = FLASH_STATE_VERIFY_DEVICE;
            } else {
                UART5_SendString("ERR: Firmware Flashing Aborted\r\n");
                state = FLASH_STATE_FAIL_ABORT;
            }
            break;
        }

        case FLASH_STATE_VERIFY_DEVICE: {
            pic32_pe_crc_t result =
                PIC32_GetCRC(firmware_summary.start_address, firmware_summary.total_bytes);

            if ((result.status.code == PE_RESP_PASS) && (result.crc == firmware_summary.crc)) {
                UART5_SendString("STATUS: CRC Verification OK\r\n");
                state = FLASH_STATE_EXIT_PROGRAMMING_MODE;
            } else {
                UART5_SendString("ERR: Flash CRC Verification Failed\r\n");
                state = FLASH_STATE_WIPE_ON_VERIFY_FAIL;
            }
            break;
        }

        case FLASH_STATE_WIPE_ON_VERIFY_FAIL: {
            /* Verification failed, so flash now holds an unverified/unknown
             * image -- not the original firmware (every WordProgram already
             * returned PASS to get here, so that's already gone). Erase it
             * so the target ends up blank/inert rather than possibly
             * booting something incorrect once MCLR is released below.
             * This is a safety wipe, not a retry: abort either way. */
            device_status_t result = EraseDevice();
            if (result.ready) {
                UART5_SendString("STATUS: Unverified Image Erased\r\n");
            } else {
                UART5_SendString("ERR: Post-Verify-Fail Erase Incomplete\r\n");
            }
            state = FLASH_STATE_FAIL_ABORT;
            break;
        }

        case FLASH_STATE_EXIT_PROGRAMMING_MODE:
            ExitProgrammingMode();
            UART5_SendString("STATUS: Programming Complete, Target Running\r\n");
            state = FLASH_STATE_IDLE;
            break;

        case FLASH_STATE_FAIL_ABORT:
            AbortAndCleanup();
            UART5_SendString("STATUS: Session Aborted. Safe State Restored.\r\n");
            state = FLASH_STATE_IDLE;
            break;

        case FLASH_STATE_IDLE:
        default:
            /* Terminal state: send the idle notice once, then halt here for
             * good. Never actually return -- main() has nothing after this
             * call, and Reset_Handler's `bx lr` tail call would jump to
             * whatever garbage is sitting in LR if we did, since it was
             * entered from the reset vector rather than a real call. */
            UART5_SendString("STATUS: Sitting at IDLE state\r\n");
            for (;;) {
                /* Await next trigger or reboot */
            }
        }
    }
}
