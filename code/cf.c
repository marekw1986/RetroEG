#include "cf.h"

#define CFREG0 (*(volatile uint8_t*)(CFBASE+0))		// DATA PORT
#define CFREG1 (*(volatile uint8_t*)(CFBASE+1))		// READ: ERROR CODE, WRITE: FEATURE
#define CFREG2 (*(volatile uint8_t*)(CFBASE+2))		// NUMBER OF SECTORS TO TRANSFER
#define CFREG3 (*(volatile uint8_t*)(CFBASE+3))		// SECTOR ADDRESS LBA 0 [0:7]
#define CFREG4 (*(volatile uint8_t*)(CFBASE+4))		// SECTOR ADDRESS LBA 1 [8:15]
#define CFREG5 (*(volatile uint8_t*)(CFBASE+5))		// SECTOR ADDRESS LBA 2 [16:23]
#define CFREG6 (*(volatile uint8_t*)(CFBASE+6))		// SECTOR ADDRESS LBA 3 [24:27 (LSB)]
#define CFREG7 (*(volatile uint8_t*)(CFBASE+7))		// READ: STATUS, WRITE: COMMAND

#define CMD_RST 			0x04
#define CMD_READ_SECTOR		0x20
#define CMD_WRITE_SECTOR	0x30
#define CMD_INFO_COMMAND	0xEC

#define MASK_ERROR 			0x01
#define MASK_BUSY 			0x80
#define MASK_DRQ			0x08

#define cfWait()	while(CFREG7 & MASK_BUSY) {}

uint8_t cfInit(void) {
	CFREG7 = CMD_RST;
	cfWait();
	CFREG6 = 0xE0;				// LBA3=0, MASTER, MODE=LBA
	CFREG1 = 0x01;				// 8-BIT TRANSFERS
	CFREG7 = 0xEF;				// SET FEATURE COMMAND
	cfWait();
	// check for error now and return
	if (CFREG7 & MASK_ERROR) return 0;
	return 1;
}

uint8_t cfReadBlocks(uint8_t *buffer, uint32_t lba, uint8_t blocksToRead) {
	uint8_t tmp;
	
	//Set LBA
	CFREG3 = (uint8_t)(lba & 0x000000FF);				// [7:0]
	CFREG4 = (uint8_t)((lba >> 8) & 0x000000FF);		// [15:8]
	CFREG5 = (uint8_t)((lba >> 16) & 0x000000FF);		// [23:16]
	tmp = (uint8_t)((lba >> 24) & 0x000000FF); 			// [27:24]
	tmp &= 0x0F; 										// Filter out LBA bits
	tmp |= 0xE0;										// Mode LBA, Master Dev
	CFREG6 = tmp;	
	//Set amount of blocks to read
	CFREG2 = blocksToRead;
	cfWait();
	CFREG7 = CMD_READ_SECTOR;
	cfWait();
	while (CFREG7 & MASK_DRQ) {
		*buffer = CFREG0;
		buffer++;
		cfWait();
	}
	if (CFREG7 & MASK_ERROR) return 0;
	return 1;
}

uint8_t cfWriteBlocks(uint8_t *buffer, uint32_t lba, uint8_t blocksToWrite) {
	uint8_t tmp;
	
	//Set LBA
	CFREG3 = (uint8_t)(lba & 0x000000FF);				// [7:0]
	CFREG4 = (uint8_t)((lba >> 8) & 0x000000FF);		// [15:8]
	CFREG5 = (uint8_t)((lba >> 16) & 0x000000FF);		// [23:16]
	tmp = (uint8_t)((lba >> 24) & 0x000000FF); 			// [27:24]
	tmp &= 0x0F; 										// Filter out LBA bits
	tmp |= 0xE0;										// Mode LBA, Master Dev
	CFREG6 = tmp;		
	//Set amount of blocks to write
	CFREG2 = blocksToWrite;
	cfWait();
	CFREG7 = CMD_WRITE_SECTOR;
	cfWait();
	while (CFREG7 & MASK_DRQ) {
		CFREG0 = *buffer;
		buffer++;
		cfWait();
	}
	if (CFREG7 & MASK_ERROR) return 0;
	return 1;
}

uint8_t cfGetSizeInfo(uint32_t *availableBlocks, uint16_t *sizeOfBlock) {
	uint16_t i=0;
	uint8_t tmp; 
	
	*availableBlocks = 0x00000000;
	cfWait();
	CFREG7 = CMD_INFO_COMMAND;
	cfWait();
	while (CFREG7 & MASK_DRQ) {
		tmp = CFREG0;
		if (i == 120) { *availableBlocks |= (uint32_t)tmp; }
		else if (i == 121) {*availableBlocks |= ((uint32_t)tmp << 8); }
		else if (i == 122) {*availableBlocks |= ((uint32_t)tmp << 16); }
		else if (i == 123) {*availableBlocks |= ((uint32_t)tmp << 24); }
		i++;
		cfWait();
	}
	*sizeOfBlock = 512;
	return 1;
}

