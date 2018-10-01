/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"        /* FatFs lower layer API */
#include "../drivers/ata.h"

#include <stdio.h>

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
        BYTE pdrv        /* Physical drive nmuber to identify the drive */
) {
    if (pdrv < 3 && ide_devices[pdrv].reserved) {
        return 0;
    }
    return STA_NODISK;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
        BYTE pdrv                /* Physical drive nmuber to identify the drive */
) {
    if (pdrv < 3 && ide_devices[pdrv].reserved) {
        return 0;
    }
    return STA_NODISK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
        BYTE pdrv,        /* Physical drive nmuber to identify the drive */
        BYTE *buff,        /* Data buffer to store read data */
        DWORD sector,    /* Start sector in LBA */
        UINT count        /* Number of sectors to read */
) {
    if (pdrv < 3 && ide_devices[pdrv].reserved) {
        if (ide_ata_access(0, pdrv, sector, (uint8_t) count, 0, (uintptr_t) buff)) {
            return RES_ERROR;
        }
        return RES_OK;
    }
    return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write(
        BYTE pdrv,            /* Physical drive nmuber to identify the drive */
        const BYTE *buff,    /* Data to be written */
        DWORD sector,        /* Start sector in LBA */
        UINT count            /* Number of sectors to write */
) {
    if (pdrv < 3 && ide_devices[pdrv].reserved) {
        if (ide_ata_access(1, pdrv, sector, (uint8_t) count, 0, (uintptr_t) buff)) {
            return RES_ERROR;
        }
        return RES_OK;
    }
    return RES_WRPRT;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
        BYTE pdrv,        /* Physical drive nmuber (0..) */
        BYTE cmd,        /* Control code */
        void *buff        /* Buffer to send/receive control data */
) {
    // TODO
    // printf("called unimplemented function disk_ioctl\n");
    return RES_OK;
}

