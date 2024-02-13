#ifndef _I2S_H_
#define _I2S_H_

#include "gd32f30x.h"

#define I2S_FSOPT_44K1 ((uint32_t)(6 + 0x100))
#define I2S_FSOPT_48K ((uint32_t)(6))
#define I2S_FSOPT_96K ((uint32_t)(3))
#define I2S_FSOPT_192K ((uint32_t)(1 + 0x100))

void i2s_config(void);
void i2s_SetFs(uint32_t fsopt);
void i2s_SetFrameFormat(uint32_t i2s_frameformat);
void i2s_DMASetSrc(void *srcaddr, uint32_t len);
void i2s_DMAConfig32(uint32_t *srcbuf, uint32_t len);
void i2s_DMAConfig16(uint16_t *srcbuf, uint32_t len);
void i2s_DMAStart();
void i2s_DMAStop();

#endif
