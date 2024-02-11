#ifndef _SPIFLASH_H_
#define _SPIFLASH_H_

#define SPIFLASH_SPI_TIMEOUT 51

#include "gd32f30x.h"

uint8_t spiflash_Init(void);
void spiflash_Read(uint32_t addr,uint8_t *buf,uint32_t len);

#endif
