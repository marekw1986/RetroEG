#ifndef _CF_H_
#define _CF_F_

#include <inttypes.h>

#define CFBASE 0x6580	//Compact Flash card base address

uint8_t cfInit(void);
uint8_t cfReadBlocks(uint8_t *buffer, uint32_t lba, uint8_t blocksToRead);
uint8_t cfWriteBlocks(uint8_t *buffer, uint32_t lba, uint8_t blocksToWrite);
uint8_t cfGetSizeInfo(uint32_t *availableBlocks, uint16_t *sizeOfBlock);

#endif //_CF_H_
