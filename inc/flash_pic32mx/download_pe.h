#ifndef DOWNLOAD_PE_H
#define DOWNLOAD_PE_H

#include <stdint.h>
#include <stddef.h>

void DownloadPE_InitBMX(void);
void DownloadPE_LoadLoader(void);
void DownloadPE_SendBlockHeader(uint32_t address, uint32_t word_count);
void DownloadPE_SendWord(uint32_t word);
void DownloadPE_SendImage(const uint32_t *pe_image, size_t word_count);
void DownloadPE_JumpToPE(void);
void DownloadPE(const uint32_t *pe_image, size_t word_count);

#endif /* DOWNLOAD_PE_H */
