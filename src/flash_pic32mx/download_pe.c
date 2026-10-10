#include "flash_pic32mx/download_pe.h"
#include "flash_pic32mx/pseudo_operations.h"
#include "flash_pic32mx/tap_controller.h"

static const uint32_t PE_LOADER_OPCODES[] = {
    0x3c07deadUL, /* lui   a3, 0xdead         */
    0x3c06ff20UL, /* lui   a2, 0xff20         */
    0x3c05ff20UL, /* lui   a1, 0xff20         */
    0x8cc40000UL, /* here1: lw   a0, 0(a2)    */
    0x8cc30000UL, /*       lw   v1, 0(a2)    */
    0x1067000bUL, /*       beq  v1, a3, here3*/
    0x00000000UL, /*       nop                */
    0x1060fffbUL, /*       beqz v1, here1     */
    0x00000000UL, /*       nop                */
    0x8ca20000UL, /* here2: lw   v0, 0(a1)    */
    0x2463ffffUL, /*       addiu v1, v1, -1   */
    0xac820000UL, /*       sw   v0, 0(a0)     */
    0x24840004UL, /*       addiu a0, a0, 4    */
    0x1460fffbUL, /*       bnez v1, here2     */
    0x00000000UL, /*       nop                */
    0x1000fff3UL, /*       b    here1         */
    0x00000000UL, /*       nop                */
    0x3c02a000UL, /* here3: lui  v0, 0xa000    */
    0x34420900UL, /*       ori  v0, v0, 0x900 */
    0x00400008UL, /*       jr   v0            */
    0x00000000UL  /*       nop                */
};

#define PE_LOADER_WORD_COUNT (sizeof(PE_LOADER_OPCODES) / sizeof(PE_LOADER_OPCODES[0]))

void DownloadPE_InitBMX(void)
{
    /* [INIT_BMX_BUS_MATRIX] */
    XferInstruction(0x3c04bf88UL);
    XferInstruction(0x34842000UL);
    XferInstruction(0x3c05001fUL);
    XferInstruction(0x34a50040UL);
    XferInstruction(0xac850000UL);

    XferInstruction(0x34050800UL);
    XferInstruction(0xac850010UL);

    XferInstruction(0x8c850040UL);
    XferInstruction(0xac850020UL);
    XferInstruction(0xac850030UL);
}

void DownloadPE_LoadLoader(void)
{
    /* [INJECT_LOADER_OPCODES] */
    XferInstruction(0x3c04a000UL);
    XferInstruction(0x34840800UL);

    for (size_t i = 0; i < PE_LOADER_WORD_COUNT; i++) {
        uint32_t word = PE_LOADER_OPCODES[i];
        uint32_t hi = (word >> 16) & 0xFFFFUL;
        uint32_t lo = word & 0xFFFFUL;

        XferInstruction(0x3c060000UL | hi);
        XferInstruction(0x34c60000UL | lo);
        XferInstruction(0xac860000UL);
        XferInstruction(0x24840004UL);
    }

    XferInstruction(0x3c19a000UL);
    XferInstruction(0x37390800UL);
    XferInstruction(0x03200008UL);
    XferInstruction(0x00000000UL);
}

void DownloadPE_SendBlockHeader(uint32_t address, uint32_t word_count)
{
    (void)XferFastData(address);
    (void)XferFastData(word_count);
}

void DownloadPE_SendWord(uint32_t word)
{
    (void)XferFastData(word);
}

void DownloadPE_SendImage(const uint32_t *pe_image, size_t word_count)
{
    /* [STREAM_PE_IMAGE] */
    size_t i = 0;

    SendCommand(ETAP_FASTDATA);

    while (i < word_count) {
        uint32_t address = pe_image[i++];
        uint32_t block_word_count = pe_image[i++];

        DownloadPE_SendBlockHeader(address, block_word_count);

        for (uint32_t w = 0; w < block_word_count; w++) {
            DownloadPE_SendWord(pe_image[i++]);
        }
    }
}

void DownloadPE_JumpToPE(void)
{
    /* [SENTINEL_TRIGGER] */
    (void)XferFastData(0x00000000UL);
    (void)XferFastData(0xDEAD0000UL);
}

void DownloadPE(const uint32_t *pe_image, size_t word_count)
{
    DownloadPE_InitBMX();
    DownloadPE_LoadLoader();
    DownloadPE_SendImage(pe_image, word_count);
    DownloadPE_JumpToPE();
}
