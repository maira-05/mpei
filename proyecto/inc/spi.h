#ifndef __SPI_H__
#define __SPI_H__
#include <stdint.h>

void spi2_init(void);              /* SPI2 maestro, modo 3, 8 bits, pines PB13/PB15 */
void spi2_enviar(uint8_t dato);    /* envía un byte */
void spi2_esperar(void);           /* espera a que termine de enviar */

#endif