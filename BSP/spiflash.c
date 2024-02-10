#include "spiflash.h"
#include <stdio.h>
#include <stdint.h>

#define SPIFLASH_CS_EN() (GPIO_BC(GPIOB) = (uint32_t)(GPIO_PIN_9))
#define SPIFLASH_CS_DIS() (GPIO_BOP(GPIOB) = (uint32_t)(GPIO_PIN_9))

uint8_t spiflash_Init(void)
{
    return 0;
}

void spiflash_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint8_t readSeq[4] = {0x03, (addr >> 16) & 0x000000ff, (addr >> 8) & 0x000000ff, (addr >> 0) & 0x000000ff};
    SPIFLASH_CS_EN();
    for (uint8_t i = 0; i < 4; i++)
    {
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
        {
            __NOP();
        }
        spi_i2s_data_transmit(SPI2, readSeq[i]);
    }
    for (uint32_t i = 0; i < len; i++)
    {
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
        {
            __NOP();
        }
        spi_i2s_data_transmit(SPI2, 0xff);
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE))
        {
            __NOP();
        }
        *(buf + i) = (uint8_t)spi_i2s_data_receive(SPI2);
    }
    while (SET == spi_i2s_flag_get(SPI2, SPI_FLAG_TRANS))
    {
        __NOP();
    }
    SPIFLASH_CS_DIS();
}
