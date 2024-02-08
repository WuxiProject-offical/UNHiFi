#include "spiflash.h"
#include <stdio.h>
#include <stdint.h>

#define SPIFLASH_CS_EN() (GPIO_BC(GPIOB) = (uint32_t)(GPIO_PIN_9))
#define SPIFLASH_CS_DIS() (GPIO_BOP(GPIOB) = (uint32_t)(GPIO_PIN_9))

uint8_t spiflash_Init(void)
{
    return 0;
}
