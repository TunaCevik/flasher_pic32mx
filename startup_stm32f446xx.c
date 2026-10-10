#include <stdint.h>

#define SRAM_START  0x20000000U
#define SRAM_SIZE   (128U * 1024U)
#define STACK_START (SRAM_START + SRAM_SIZE)

/* External variables from the linker script */
extern uint32_t _sidata; // End of text section in flash
extern uint32_t _sdata;  // Start of data section in RAM
extern uint32_t _edata;  // End of data section in RAM
extern uint32_t _sbss;   // Start of bss section in RAM
extern uint32_t _ebss;   // End of bss section in RAM

/* External declaration for the main function */
extern int main(void);

extern void SystemInit(void);

// /* External declaration for libc initialization (if used) */
extern void __libc_init_array(void);

/* Macro to make function declarations cleaner */
#define WEAK_ALIAS __attribute__((weak, alias("Default_Handler")))

/* System Exception Handlers */
void Reset_Handler(void);
void Default_Handler(void);
void NMI_Handler(void)          WEAK_ALIAS;
void HardFault_Handler(void)    WEAK_ALIAS;
void MemManage_Handler(void)    WEAK_ALIAS;
void BusFault_Handler(void)     WEAK_ALIAS;
void UsageFault_Handler(void)   WEAK_ALIAS;
void SVC_Handler(void)          WEAK_ALIAS;
void DebugMon_Handler(void)     WEAK_ALIAS;
void PendSV_Handler(void)       WEAK_ALIAS;
void SysTick_Handler(void)      WEAK_ALIAS;

/* External Interrupt Handlers */
void WWDG_IRQHandler(void)                   WEAK_ALIAS;
void PVD_IRQHandler(void)                    WEAK_ALIAS;
void TAMP_STAMP_IRQHandler(void)             WEAK_ALIAS;
void RTC_WKUP_IRQHandler(void)               WEAK_ALIAS;
void FLASH_IRQHandler(void)                  WEAK_ALIAS;
void RCC_IRQHandler(void)                    WEAK_ALIAS;
void EXTI0_IRQHandler(void)                  WEAK_ALIAS;
void EXTI1_IRQHandler(void)                  WEAK_ALIAS;
void EXTI2_IRQHandler(void)                  WEAK_ALIAS;
void EXTI3_IRQHandler(void)                  WEAK_ALIAS;
void EXTI4_IRQHandler(void)                  WEAK_ALIAS;
void DMA1_Stream0_IRQHandler(void)           WEAK_ALIAS;
void DMA1_Stream1_IRQHandler(void)           WEAK_ALIAS;
void DMA1_Stream2_IRQHandler(void)           WEAK_ALIAS;
void DMA1_Stream3_IRQHandler(void)           WEAK_ALIAS;
void DMA1_Stream4_IRQHandler(void)           WEAK_ALIAS;
void DMA1_Stream5_IRQHandler(void)           WEAK_ALIAS;
void DMA1_Stream6_IRQHandler(void)           WEAK_ALIAS;
void ADC_IRQHandler(void)                    WEAK_ALIAS;
void CAN1_TX_IRQHandler(void)                WEAK_ALIAS;
void CAN1_RX0_IRQHandler(void)               WEAK_ALIAS;
void CAN1_RX1_IRQHandler(void)               WEAK_ALIAS;
void CAN1_SCE_IRQHandler(void)               WEAK_ALIAS;
void EXTI9_5_IRQHandler(void)                WEAK_ALIAS;
void TIM1_BRK_TIM9_IRQHandler(void)          WEAK_ALIAS;
void TIM1_UP_TIM10_IRQHandler(void)          WEAK_ALIAS;
void TIM1_TRG_COM_TIM11_IRQHandler(void)     WEAK_ALIAS;
void TIM1_CC_IRQHandler(void)                WEAK_ALIAS;
void TIM2_IRQHandler(void)                   WEAK_ALIAS;
void TIM3_IRQHandler(void)                   WEAK_ALIAS;
void TIM4_IRQHandler(void)                   WEAK_ALIAS;
void I2C1_EV_IRQHandler(void)                WEAK_ALIAS;
void I2C1_ER_IRQHandler(void)                WEAK_ALIAS;
void I2C2_EV_IRQHandler(void)                WEAK_ALIAS;
void I2C2_ER_IRQHandler(void)                WEAK_ALIAS;
void SPI1_IRQHandler(void)                   WEAK_ALIAS;
void SPI2_IRQHandler(void)                   WEAK_ALIAS;
void USART1_IRQHandler(void)                 WEAK_ALIAS;
void USART2_IRQHandler(void)                 WEAK_ALIAS;
void USART3_IRQHandler(void)                 WEAK_ALIAS;
void EXTI15_10_IRQHandler(void)              WEAK_ALIAS;
void RTC_Alarm_IRQHandler(void)              WEAK_ALIAS;
void OTG_FS_WKUP_IRQHandler(void)            WEAK_ALIAS;
void TIM8_BRK_TIM12_IRQHandler(void)         WEAK_ALIAS;
void TIM8_UP_TIM13_IRQHandler(void)          WEAK_ALIAS;
void TIM8_TRG_COM_TIM14_IRQHandler(void)     WEAK_ALIAS;
void TIM8_CC_IRQHandler(void)                WEAK_ALIAS;
void DMA1_Stream7_IRQHandler(void)           WEAK_ALIAS;
void FMC_IRQHandler(void)                    WEAK_ALIAS;
void SDIO_IRQHandler(void)                   WEAK_ALIAS;
void TIM5_IRQHandler(void)                   WEAK_ALIAS;
void SPI3_IRQHandler(void)                   WEAK_ALIAS;
void UART4_IRQHandler(void)                  WEAK_ALIAS;
void UART5_IRQHandler(void)                  WEAK_ALIAS;
void TIM6_DAC_IRQHandler(void)               WEAK_ALIAS;
void TIM7_IRQHandler(void)                   WEAK_ALIAS;
void DMA2_Stream0_IRQHandler(void)           WEAK_ALIAS;
void DMA2_Stream1_IRQHandler(void)           WEAK_ALIAS;
void DMA2_Stream2_IRQHandler(void)           WEAK_ALIAS;
void DMA2_Stream3_IRQHandler(void)           WEAK_ALIAS;
void DMA2_Stream4_IRQHandler(void)           WEAK_ALIAS;
void CAN2_TX_IRQHandler(void)                WEAK_ALIAS;
void CAN2_RX0_IRQHandler(void)               WEAK_ALIAS;
void CAN2_RX1_IRQHandler(void)               WEAK_ALIAS;
void CAN2_SCE_IRQHandler(void)               WEAK_ALIAS;
void OTG_FS_IRQHandler(void)                 WEAK_ALIAS;
void DMA2_Stream5_IRQHandler(void)           WEAK_ALIAS;
void DMA2_Stream6_IRQHandler(void)           WEAK_ALIAS;
void DMA2_Stream7_IRQHandler(void)           WEAK_ALIAS;
void USART6_IRQHandler(void)                 WEAK_ALIAS;
void I2C3_EV_IRQHandler(void)                WEAK_ALIAS;
void I2C3_ER_IRQHandler(void)                WEAK_ALIAS;
void OTG_HS_EP1_OUT_IRQHandler(void)         WEAK_ALIAS;
void OTG_HS_EP1_IN_IRQHandler(void)          WEAK_ALIAS;
void OTG_HS_WKUP_IRQHandler(void)            WEAK_ALIAS;
void OTG_HS_IRQHandler(void)                 WEAK_ALIAS;
void DCMI_IRQHandler(void)                   WEAK_ALIAS;
void FPU_IRQHandler(void)                    WEAK_ALIAS;
void SPI4_IRQHandler(void)                   WEAK_ALIAS;
void SAI1_IRQHandler(void)                   WEAK_ALIAS;
void SAI2_IRQHandler(void)                   WEAK_ALIAS;
void QUADSPI_IRQHandler(void)                WEAK_ALIAS;
void CEC_IRQHandler(void)                    WEAK_ALIAS;
void SPDIF_RX_IRQHandler(void)               WEAK_ALIAS;
void FMPI2C1_Event_IRQHandler(void)          WEAK_ALIAS;
void FMPI2C1_Error_IRQHandler(void)          WEAK_ALIAS;


/* The Vector Table */
uint32_t vectors[] __attribute__((section(".isr_vector"))) = {
    (uint32_t)STACK_START,
    (uint32_t)&Reset_Handler,
    (uint32_t)&NMI_Handler,
    (uint32_t)&HardFault_Handler,
    (uint32_t)&MemManage_Handler,
    (uint32_t)&BusFault_Handler,
    (uint32_t)&UsageFault_Handler,
    0, // Reserved
    0, // Reserved
    0, // Reserved
    0, // Reserved
    (uint32_t)&SVC_Handler,
    (uint32_t)&DebugMon_Handler,
    0, // Reserved
    (uint32_t)&PendSV_Handler,
    (uint32_t)&SysTick_Handler,
    (uint32_t)&WWDG_IRQHandler,
    (uint32_t)&PVD_IRQHandler,
    (uint32_t)&TAMP_STAMP_IRQHandler,
    (uint32_t)&RTC_WKUP_IRQHandler,
    (uint32_t)&FLASH_IRQHandler,
    (uint32_t)&RCC_IRQHandler,
    (uint32_t)&EXTI0_IRQHandler,
    (uint32_t)&EXTI1_IRQHandler,
    (uint32_t)&EXTI2_IRQHandler,
    (uint32_t)&EXTI3_IRQHandler,
    (uint32_t)&EXTI4_IRQHandler,
    (uint32_t)&DMA1_Stream0_IRQHandler,
    (uint32_t)&DMA1_Stream1_IRQHandler,
    (uint32_t)&DMA1_Stream2_IRQHandler,
    (uint32_t)&DMA1_Stream3_IRQHandler,
    (uint32_t)&DMA1_Stream4_IRQHandler,
    (uint32_t)&DMA1_Stream5_IRQHandler,
    (uint32_t)&DMA1_Stream6_IRQHandler,
    (uint32_t)&ADC_IRQHandler,
    (uint32_t)&CAN1_TX_IRQHandler,
    (uint32_t)&CAN1_RX0_IRQHandler,
    (uint32_t)&CAN1_RX1_IRQHandler,
    (uint32_t)&CAN1_SCE_IRQHandler,
    (uint32_t)&EXTI9_5_IRQHandler,
    (uint32_t)&TIM1_BRK_TIM9_IRQHandler,
    (uint32_t)&TIM1_UP_TIM10_IRQHandler,
    (uint32_t)&TIM1_TRG_COM_TIM11_IRQHandler,
    (uint32_t)&TIM1_CC_IRQHandler,
    (uint32_t)&TIM2_IRQHandler,
    (uint32_t)&TIM3_IRQHandler,
    (uint32_t)&TIM4_IRQHandler,
    (uint32_t)&I2C1_EV_IRQHandler,
    (uint32_t)&I2C1_ER_IRQHandler,
    (uint32_t)&I2C2_EV_IRQHandler,
    (uint32_t)&I2C2_ER_IRQHandler,
    (uint32_t)&SPI1_IRQHandler,
    (uint32_t)&SPI2_IRQHandler,
    (uint32_t)&USART1_IRQHandler,
    (uint32_t)&USART2_IRQHandler,
    (uint32_t)&USART3_IRQHandler,
    (uint32_t)&EXTI15_10_IRQHandler,
    (uint32_t)&RTC_Alarm_IRQHandler,
    (uint32_t)&OTG_FS_WKUP_IRQHandler,
    (uint32_t)&TIM8_BRK_TIM12_IRQHandler,
    (uint32_t)&TIM8_UP_TIM13_IRQHandler,
    (uint32_t)&TIM8_TRG_COM_TIM14_IRQHandler,
    (uint32_t)&TIM8_CC_IRQHandler,
    (uint32_t)&DMA1_Stream7_IRQHandler,
    (uint32_t)&FMC_IRQHandler,
    (uint32_t)&SDIO_IRQHandler,
    (uint32_t)&TIM5_IRQHandler,
    (uint32_t)&SPI3_IRQHandler,
    (uint32_t)&UART4_IRQHandler,
    (uint32_t)&UART5_IRQHandler,
    (uint32_t)&TIM6_DAC_IRQHandler,
    (uint32_t)&TIM7_IRQHandler,
    (uint32_t)&DMA2_Stream0_IRQHandler,
    (uint32_t)&DMA2_Stream1_IRQHandler,
    (uint32_t)&DMA2_Stream2_IRQHandler,
    (uint32_t)&DMA2_Stream3_IRQHandler,
    (uint32_t)&DMA2_Stream4_IRQHandler,
    0, // Reserved
    0, // Reserved
    (uint32_t)&CAN2_TX_IRQHandler,
    (uint32_t)&CAN2_RX0_IRQHandler,
    (uint32_t)&CAN2_RX1_IRQHandler,
    (uint32_t)&CAN2_SCE_IRQHandler,
    (uint32_t)&OTG_FS_IRQHandler,
    (uint32_t)&DMA2_Stream5_IRQHandler,
    (uint32_t)&DMA2_Stream6_IRQHandler,
    (uint32_t)&DMA2_Stream7_IRQHandler,
    (uint32_t)&USART6_IRQHandler,
    (uint32_t)&I2C3_EV_IRQHandler,
    (uint32_t)&I2C3_ER_IRQHandler,
    (uint32_t)&OTG_HS_EP1_OUT_IRQHandler,
    (uint32_t)&OTG_HS_EP1_IN_IRQHandler,
    (uint32_t)&OTG_HS_WKUP_IRQHandler,
    (uint32_t)&OTG_HS_IRQHandler,
    (uint32_t)&DCMI_IRQHandler,
    0, // Reserved
    0, // Reserved
    (uint32_t)&FPU_IRQHandler,
    0, // Reserved
    0, // Reserved
    (uint32_t)&SPI4_IRQHandler,
    0, // Reserved
    0, // Reserved
    (uint32_t)&SAI1_IRQHandler,
    0, // Reserved
    0, // Reserved
    0, // Reserved
    (uint32_t)&SAI2_IRQHandler,
    (uint32_t)&QUADSPI_IRQHandler,
    (uint32_t)&CEC_IRQHandler,
    (uint32_t)&SPDIF_RX_IRQHandler,
    (uint32_t)&FMPI2C1_Event_IRQHandler,
    (uint32_t)&FMPI2C1_Error_IRQHandler
};


void Default_Handler(void) {
    while(1) {
    }
}

// cpu .data section to SRAM
// Init. the .bss section to zero in SRAM
// call main() after setup
void Reset_Handler(void) {
    uint32_t size = (uint32_t)&_edata - (uint32_t)&_sdata;
    
    // 1. Copy .data from Flash to SRAM
    uint8_t *pDst = (uint8_t*)&_sdata;
    uint8_t *pSrc = (uint8_t*)&_sidata;

    for (uint32_t i = 0; i < size; i++) {
        *pDst++ = *pSrc++;
    }

    // 2. Fill .bss with Zeros in SRAM
    size = (uint32_t)&_ebss - (uint32_t)&_sbss;
    pDst = (uint8_t*)&_sbss;

    for (uint32_t i = 0; i < size; i++) {
        *pDst++ = 0;
    }

    // 3. Initialize the system (clock, memory, etc.)
    SystemInit(); 
    
    // 4. Initialize the C standard Library
    __libc_init_array();

    // 5. Start the main application.
    main();
}