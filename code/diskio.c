/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/* Note that Tiny-FatFs supports only single drive and always            */
/* accesses drive number 0.                                              */


//user's include:
#include <inttypes.h>
#include "cf.h"

/* Definitions of physical drive number for each drive */
#define COMPACT_FLASH		0	/* Example: Map CF memory to physical drive 0 */

static DSTATUS diskStatus = STA_NOINIT;

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
	switch (drv) {
		case COMPACT_FLASH:
			if( cfInit() ) { diskStatus = 0; }
			else { diskStatus = STA_NOINIT; }
			return(diskStatus);
		break;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	switch (drv) {
		case COMPACT_FLASH:
			return diskStatus;
		break;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
	switch (drv) {
		case COMPACT_FLASH:
			if ( cfReadBlocks(buff,sector,count) ) { return RES_OK; }
			else { return RES_ERROR; }
		break;
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	switch (drv) {
		case COMPACT_FLASH:
			if ( cfWriteBlocks((void*)buff,sector,count) ) { return RES_OK; }
			else { return RES_ERROR; }
		break;
	}
	return RES_PARERR;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	static uint32_t maxLba;
	static uint16_t blockSize;
	
	switch (drv) {
		case COMPACT_FLASH:
			switch(ctrl) {
				case GET_SECTOR_COUNT:
					cfGetSizeInfo(&maxLba,&blockSize);
					buff = (void*)&maxLba;
					return RES_OK;
				case GET_BLOCK_SIZE:
					cfGetSizeInfo(&maxLba,&blockSize);
					buff = (void*)&blockSize;
					return RES_OK;
				case CTRL_SYNC:
					return RES_OK;
				default:
					return STA_NOINIT;
			}
		break;
	}
}

