#include "cf.h"

#define CFREG0 (*(uint8_t*)(CFBASE+0))		// DATA PORT
#define CFREG1 (*(uint8_t*)(CFBASE+1))		// READ: ERROR CODE, WRITE: FEATURE
#define CFREG2 (*(uint8_t*)(CFBASE+2))		// NUMBER OF SECTORS TO TRANSFER
#define CFREG3 (*(uint8_t*)(CFBASE+3))		// SECTOR ADDRESS LBA 0 [0:7]
#define CFREG4 (*(uint8_t*)(CFBASE+4))		// SECTOR ADDRESS LBA 1 [8:15]
#define CFREG5 (*(uint8_t*)(CFBASE+5))		// SECTOR ADDRESS LBA 2 [16:23]
#define CFREG6 (*(uint8_t*)(CFBASE+6))		// SECTOR ADDRESS LBA 3 [24:27 (LSB)]
#define CFREG7 (*(uint8_t*)(CFBASE+7))		// READ: STATUS, WRITE: COMMAND

#define CMD_RST 	0x04

#define MASK_ERROR 	0x01
#define MASK_BUSY 	0x08

void cfWait(void);

uint8_t cfInit(void) {
	CFREG7 = CMD_RST;
	cfWait();
	CFREG6 = 0xE0;
	CFREG1 = 0x01;
	CFREG7 = 0xEF;
	cfWait();
	// check for error now and return
	if (CFREG7 && MASK_ERROR) return 1;
	return 0;
}

uint8_t cfReadBlocks(void *buffer, uint32_t lba, uint32_t blocksToRead) {
	return 1;
}

uint8_t cfWriteBlocks(void *buffer, uint32_t lba, uint32_t blocksToWrite) {
	return 1;
}

uint8_t cfGetSizeInfo(uint32_t *availableBlocks, uint16_t *sizeOfBlock) {
	return 1;
}

void cfWait(void) {
	while(CFREG7 && MASK_BUSY) {}
}
