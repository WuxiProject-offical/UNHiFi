#ifndef _I2S_H_
#define _I2S_H_

#include "gd32f30x.h"

void i2s_config(void);
void i2s_DMAConfig32(uint32_t *srcbuf, uint32_t len);
void i2s_DMAConfig16(uint16_t *srcbuf, uint32_t len);
void i2s_DMAStart();
void i2s_DMAStop();

#endif
