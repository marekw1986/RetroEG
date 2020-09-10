#include "cf.h"

#define CFREG0 (*(uint8_t*)(CFBASE+0))		// DATA PORT
#define CFREG1 (*(uint8_t*)(CFBASE+1))		// READ: ERROR CODE, WRITE: FEATURE
#define CFREG2 (*(uint8_t*)(CFBASE+2))		// NUMBER OF SECTORS TO TRANSFER
#define CFREG3 (*(uint8_t*)(CFBASE+3))		// SECTOR ADDRESS LBA 0 [0:7]
#define CFREG4 (*(uint8_t*)(CFBASE+4))		// SECTOR ADDRESS LBA 1 [8:15]
#define CFREG5 (*(uint8_t*)(CFBASE+5))		// SECTOR ADDRESS LBA 2 [16:23]
#define CFREG6 (*(uint8_t*)(CFBASE+6))		// SECTOR ADDRESS LBA 3 [24:27 (LSB)]
#define CFREG7 (*(uint8_t*)(CFBASE+7))		// READ: STATUS, WRITE: COMMAND

#define CMD_RST 			0x04
#define CMD_READ_SECTOR		0x20
#define CMD_WRITE_SECTOR	0x30
#define CMD_INFO_COMMAND	0xEC

#define MASK_ERROR 			0x01
#define MASK_BUSY 			0x08

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

uint8_t cfReadBlocks(uint8_t *buffer, uint32_t lba, uint8_t blocksToRead) {
	//Set LBA
	CFREG3 = (uint8_t)lba;
	CFREG4 = (uint8_t)(lba >> 8);
	CFREG5 = (uint8_t)(lba >> 16);
	CFREG6 = ((uint8_t)(lba >> 24) & 0x0F) | 0xE0;
	//Set amount of blocks to read
	CFREG2 = blocksToRead;
	cfWait();
	CFREG7 = CMD_READ_SECTOR;
	cfWait();
	while (CFREG7 & 0x08) {
		*buffer = CFREG0;
		buffer++;
		cfWait();
	}
	if (!CFREG7 & MASK_ERROR) return 0;
	return 1;
}

uint8_t cfWriteBlocks(uint8_t *buffer, uint32_t lba, uint8_t blocksToWrite) {
	//Set LBA
	CFREG3 = (uint8_t)lba;
	CFREG4 = (uint8_t)(lba >> 8);
	CFREG5 = (uint8_t)(lba >> 16);
	CFREG6 = ((uint8_t)(lba >> 24) & 0x0F) | 0xE0;
	//Set amount of blocks to read
	CFREG2 = blocksToWrite;
	cfWait();
	CFREG7 = CMD_WRITE_SECTOR;
	cfWait();
	while (CFREG7 & 0x08) {
		*buffer = CFREG0;
		buffer++;
		cfWait();
	}
	if (!CFREG7 & MASK_ERROR) return 0;
	return 1;
}

uint8_t cfGetSizeInfo(uint32_t *availableBlocks, uint16_t *sizeOfBlock) {
	uint16_t i=0;
	uint8_t tmp; 
	
	cfWait();
	CFREG7 = CMD_INFO_COMMAND;
	cfWait();
	while (CFREG7 & 0x08) {
		tmp = CFREG0;
		if (i == 120) { *availableBlocks = (uint32_t)tmp; }
		else if (i == 121) {*availableBlocks |= ((uint32_t)tmp << 8); }
		else if (i == 122) {*availableBlocks |= ((uint32_t)tmp << 16); }
		else if (i == 123) {*availableBlocks |= ((uint32_t)tmp << 24); }
		i++;
		cfWait();
	}
	*sizeOfBlock = 512;
	return 1;
}

void cfWait(void) {
	while(CFREG7 && MASK_BUSY) {}
}
