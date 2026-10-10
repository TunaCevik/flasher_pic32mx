#ifndef FLASH_ENGINE_H
#define FLASH_ENGINE_H

/* Top-level programming-flow state machine, driven by dsl/flash_engine.mkd.
 * Ties together the individual PIC32 Flash Programming Specification steps
 * (see the flash_pic32mx sources) into the actual sequence from Figure 3-1, reporting
 * progress/failures over UART5. Only the boxes dsl/flash_engine.mkd has
 * filled in so far are implemented: Enter Enhanced ICSP, Check Device
 * Status, Erase Device, Enter Serial Exec Mode. The remaining boxes
 * (Download the PE, Download a Data Block, Initiate Flash Write, the
 * repeat-until-Done loop, Verify Device, Exit Programming Mode) are listed
 * in that file but still empty, so this switches to [IDLE] once Enter
 * Serial Exec Mode succeeds, pending those being specified. */
void FlashEngine_Run(void);

#endif /* FLASH_ENGINE_H */
