#include "flash_pic32mx/enter_serial_exec.h"
#include "flash_pic32mx/pseudo_operations.h"
#include "flash_pic32mx/tap_controller.h"

enter_serial_exec_status_t EnterSerialExecMode(void)
{
    uint32_t status_val;

    /* [CHECK_CODE_PROTECTION] */
    SendCommand(MTAP_SW_MTAP);
    SetMode(0x1FUL, 6);
    SendCommand(MTAP_COMMAND);

    status_val = XferData(MCHP_STATUS);
    if ((status_val & MCHP_STATUS_CPS_MASK) == 0) {
        return ENTER_SERIAL_EXEC_CODE_PROTECTED;
    }

    /* [ARM_EJTAG_BOOT] */
    (void)XferData(MCHP_ASSERT_RST);

    SendCommand(MTAP_SW_ETAP);
    SetMode(0x1FUL, 6);
    SendCommand(ETAP_EJTAGBOOT);

    /* [RELEASE_RESET_AND_ENABLE_FLASH] */
    SendCommand(MTAP_SW_MTAP);
    SetMode(0x1FUL, 6);
    SendCommand(MTAP_COMMAND);

    (void)XferData(MCHP_DE_ASSERT_RST);
    (void)XferData(MCHP_FLASH_ENABLE);

    /* [ENGAGE_ETAP] */
    SendCommand(MTAP_SW_ETAP);
    SetMode(0x1FUL, 6);

    return ENTER_SERIAL_EXEC_OK;
}
