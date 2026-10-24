#ifndef __SPI_H__
#define __SPI_H__
#include <stdint.h>

void spi2_init(void);              /* SPI2 maestro, modo 3, 8 bits, pines PB13/PB15 */
void spi2_enviar(uint8_t dato);    /* envía un byte */
void spi2_esperar(void);           /* espera a que termine de enviar */
void spi1_init(void);              /* SPI1 maestro, modo 3, 8 bits, pines PA5/PA7 */
void spi1_transferir(uint8_t dato);        /* envía un byte y recibe otro */
void spi1_velocidad(uint32_t br);  /* cambia el prescaler */
#endif