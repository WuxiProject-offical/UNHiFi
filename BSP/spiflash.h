#ifndef _SPIFLASH_H_
#define _SPIFLASH_H_

#define SPIFLASH_SPI_TIMEOUT 51

#include "gd32f30x.h"

uint8_t spiflash_Init(void);
void spiflash_Unlock(void);
void spiflash_Lock(void);
void spiflash_Wait(void);
void spiflash_Erase4k(uint32_t addr);
void spiflash_Erase32k(uint32_t addr);
void spiflash_Read(uint32_t addr, uint8_t *buf, uint32_t len);
void spiflash_WriteSector(uint32_t addr, uint8_t *buf, uint16_t len);

#endif
