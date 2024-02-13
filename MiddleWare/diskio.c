/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"		/* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */
#include "sdcard.h"
#include "mysd.h"

/* Definitions of physical drive number for each drive */
#define DEV_SD 0 /* Map SDCard to physical drive 0 */

volatile DSTATUS sdStat = STA_NOINIT;
sd_card_info_struct sd_cardinfo;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	int result;

	switch (pdrv)
	{
	case DEV_SD:
		(void)result; // mute warnings.
		return sdStat;
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	int result;

	switch (pdrv)
	{
	case DEV_SD:
	{
		sd_error_enum sderr = mysd_InitCard(&sd_cardinfo);
		if (sderr == SD_OK)
			sdStat = 0x00; // init done.
		else
			sdStat = STA_NOINIT;
		(void)result; // mute warnings.
		return sdStat;
	}
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
	BYTE pdrv,	  /* Physical drive nmuber to identify the drive */
	BYTE *buff,	  /* Data buffer to store read data */
	LBA_t sector, /* Start sector in LBA */
	UINT count	  /* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv)
	{
	case DEV_SD:
	{
		sd_error_enum sderr;
		if (count == 1)
		{
			sderr = sd_block_read_SectorAddr((uint32_t *)buff, (uint32_t)sector, 512);
		}
		else
		{
			sderr = sd_multiblocks_read_SectorAddr((uint32_t *)buff, (uint32_t)sector, 512, count);
		}
		(void)result;
		if (sderr == SD_OK)
			res = RES_OK;
		else if (sderr == SD_PARAMETER_INVALID)
			res = RES_PARERR;
		else if (sderr == SD_CMD_RESP_TIMEOUT || sderr == SD_DATA_TIMEOUT)
			res = RES_NOTRDY;
		else
			res = RES_ERROR;
		return res;
	}
	}
	return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
	BYTE pdrv,		  /* Physical drive nmuber to identify the drive */
	const BYTE *buff, /* Data to be written */
	LBA_t sector,	  /* Start sector in LBA */
	UINT count		  /* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv)
	{
	case DEV_SD:
		// translate the arguments here

		result = RAM_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	}

	return RES_PARERR;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
	BYTE pdrv, /* Physical drive nmuber (0..) */
	BYTE cmd,  /* Control code */
	void *buff /* Buffer to send/receive control data */
)
{
	DRESULT res = RES_ERROR;
	int result;

	switch (pdrv)
	{
	case DEV_SD:
	{
		switch (cmd)
		{
		case CTRL_SYNC: /* Wait for end of internal write process of the drive */
		{
			uint8_t cardstate;
			sd_error_enum sderr = sd_card_state_get(&cardstate);
			if ((SD_OK == sderr) && (SD_CARDSTATE_PROGRAMMING != cardstate) && (SD_CARDSTATE_RECEIVING != cardstate))
			{
				res = RES_OK;
			}
		}
		break;

		case GET_SECTOR_COUNT: /* Get drive capacity in unit of sector (DWORD) */
			*(LBA_t *)buff = (sd_cardinfo.card_csd.c_size + 1) << 10;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE: /* Get erase block size in unit of sector (DWORD) */
			*(DWORD *)buff = 512;
			res = RES_OK;
			break;

		case CTRL_TRIM: /* Erase a block of sectors (used when _USE_ERASE == 1) */
		{
			sd_error_enum sderr = sd_erase(((DWORD *)buff)[0] * 64ULL, ((DWORD *)buff)[1] * 64ULL);
			if (sderr == SD_OK)
				res = RES_OK;
			else if (sderr == SD_FUNCTION_UNSUPPORTED || sderr == SD_PARAMETER_INVALID)
				res = RES_PARERR;
			else if (sderr == SD_CMD_RESP_TIMEOUT || sderr == SD_DATA_TIMEOUT)
				res = RES_NOTRDY;
			else
				res = RES_ERROR;
		}
		break;
		}
	}
		return res;
	}
	return RES_PARERR;
}
