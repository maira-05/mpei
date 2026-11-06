#ifndef SD_H
#define SD_H
#include <stdint.h>

#define SD_CS 6   /* PB6 */

uint8_t sd_init(void);   /* por ahora: devuelve la respuesta al CMD0 (debe ser 0x01) */
uint8_t sd_leer_sector(uint32_t sector, uint8_t *buffer);   /* 0 = OK, otro = error */

#endif