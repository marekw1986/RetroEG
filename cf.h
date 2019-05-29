#ifndef _CF_H_
#define _CF_F_

#include <inttypes.h>

#define CFBASE 0x0000	//Compact Flash card base address

uint8_t cfInit(void);
uint8_t cfReadBlocks(void *buffer, uint32_t lba, uint32_t blocksToRead);
uint8_t cfWriteBlocks(void *buffer, uint32_t lba, uint32_t blocksToWrite);
uint8_t cfGetSizeInfo(uint32_t *availableBlocks, uint16_t *sizeOfBlock);

#endif //_CF_H_
