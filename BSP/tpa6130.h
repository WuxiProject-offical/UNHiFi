#ifndef _TPA6130_H_
#define _TPA6130_H_

#include "gd32f30x.h"

typedef enum
{
    I2C_START = 0,
    I2C_SEND_ADDRESS,
    I2C_CLEAR_ADDRESS_FLAG,
    I2C_TRANSMIT_DATA,
    I2C_STOP,
} i2c_process_enum;

#define I2C_TIME_OUT (uint16_t)(5000)
#define TPA6130_I2CX I2C1
#define TPA6130_ADDR 0xC0 // 8bit addr

#define I2C_OK 1
#define I2C_FAIL 0
#define I2C_END 1

uint8_t tpa6130_Init(void);
uint8_t tpa6130_RegRead(uint8_t regAddr);
void tpa6130_RegWrite(uint8_t regAddr, uint8_t regVal);
void tpa6130_SetVolume(uint8_t vol);

#endif /* AT24CXX_H */
