/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs MAI */

/* Example: Declarations of the platform and disk functions in the project */
#include "sd.h"

#define DEV_SD 0 // Physical drive number for SD card
static DSTATUS estado = STA_NOINIT; // 
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	if (pdrv != DEV_SD) return STA_NOINIT; 
	return estado;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	if (pdrv != DEV_SD) return STA_NOINIT;
	if(sd_init()==0){
		estado=0;
	} else {
		estado = STA_NOINIT;
	}
	return estado;

}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	if (pdrv !=  DEV_SD || count ==  0) return RES_PARERR; //Verificar que el número de unidad sea 0 (SD) y que la cantidad de sectores a leer sea mayor que 0
	if (estado & STA_NOINIT) return RES_NOTRDY;//Verificar que la tarjeta SD esté inicializada
	for (UINT i =  0; i < count; i++)//Leer cada sector solicitado
	{
		if (sd_leer_sector(sector + i, buff + i * 512) != 0)//Leer un sector y almacenar los datos en el buffer
			return RES_ERROR;//Si hay un error al leer el sector, devolver RES_ERROR
		else
			continue;//Si se leyó correctamente, continuar con el siguiente sector
	}
	return RES_OK;//Devolver RES_OK si se leyeron todos los sectores correctamente
}



//habia una función de escritura, pero como el sistema de archivos es de solo lectura, no es necesaria.

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,
	BYTE cmd,
	void *buff
)
{
	(void)buff;   // buff no se usa: no atendemos comandos que lo necesiten

	if (pdrv !=  DEV_SD) return RES_PARERR; //Verificar que el número de unidad sea 0 (SD)
	if (estado & STA_NOINIT) return RES_NOTRDY; //Verificar que la tarjeta SD esté inicializada
	if (cmd == CTRL_SYNC) return RES_OK; //Si el comando es CTRL_SYNC, devolver RES_OK
	else return RES_PARERR; //Si el comando no es reconocido, devolver RES_PARERR

}

