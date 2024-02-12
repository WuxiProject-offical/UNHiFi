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
    while (RESET != spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE))
    {
        (void)spi_i2s_data_receive(SPI2);
    }
    SPIFLASH_CS_EN();
    uint32_t timeout = SPIFLASH_SPI_TIMEOUT;
    for (uint8_t i = 0; i < 4; i++)
    {
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
        {
            if (--timeout == 0)
                break;
        }
        spi_i2s_data_transmit(SPI2, readSeq[i]);
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE))
        {
            if (--timeout == 0)
                break;
        }
        (void)spi_i2s_data_receive(SPI2);
    }
    for (uint32_t i = 0; i < len; i++)
    {
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
        {
            if (--timeout == 0)
                break;
        }
        spi_i2s_data_transmit(SPI2, 0xff);
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE))
        {
            if (--timeout == 0)
                break;
        }
        *(buf + i) = (uint8_t)spi_i2s_data_receive(SPI2);
    }
    timeout = SPIFLASH_SPI_TIMEOUT;
    while (SET == spi_i2s_flag_get(SPI2, SPI_FLAG_TRANS))
    {
        if (--timeout == 0)
            break;
    }
    SPIFLASH_CS_DIS();
}

void spiflash_Unlock(void)
{
    SPIFLASH_CS_EN();
    uint32_t timeout = SPIFLASH_SPI_TIMEOUT;
    timeout = SPIFLASH_SPI_TIMEOUT;
    while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
    {
        if (--timeout == 0)
            break;
    }
    spi_i2s_data_transmit(SPI2, 0x06); // 0x06 WRE
    timeout = SPIFLASH_SPI_TIMEOUT;
    while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE))
    {
        if (--timeout == 0)
            break;
    }
    (void)spi_i2s_data_receive(SPI2);
    timeout = SPIFLASH_SPI_TIMEOUT;
    while (SET == spi_i2s_flag_get(SPI2, SPI_FLAG_TRANS))
    {
        if (--timeout == 0)
            break;
    }
    SPIFLASH_CS_DIS();
}

void spiflash_Lock(void)
{
    SPIFLASH_CS_EN();
    uint32_t timeout = SPIFLASH_SPI_TIMEOUT;
    timeout = SPIFLASH_SPI_TIMEOUT;
    while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
    {
        if (--timeout == 0)
            break;
    }
    spi_i2s_data_transmit(SPI2, 0x04); // 0x04 WRD
    timeout = SPIFLASH_SPI_TIMEOUT;
    while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE))
    {
        if (--timeout == 0)
            break;
    }
    (void)spi_i2s_data_receive(SPI2);
    timeout = SPIFLASH_SPI_TIMEOUT;
    while (SET == spi_i2s_flag_get(SPI2, SPI_FLAG_TRANS))
    {
        if (--timeout == 0)
            break;
    }
    SPIFLASH_CS_DIS();
}

void spiflash_Wait(void)
{
    SPIFLASH_CS_EN();
    uint32_t timeout = SPIFLASH_SPI_TIMEOUT;
    timeout = SPIFLASH_SPI_TIMEOUT;
    while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
    {
        if (--timeout == 0)
            break;
    }
    spi_i2s_data_transmit(SPI2, 0x05); // 0x05 ReadStatusReg1
    timeout = SPIFLASH_SPI_TIMEOUT;
    while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE))
    {
        if (--timeout == 0)
            break;
    }
    (void)spi_i2s_data_receive(SPI2);
    uint8_t busy = 0x01;
    while (busy)
    {
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
        {
            if (--timeout == 0)
                break;
        }
        spi_i2s_data_transmit(SPI2, 0xff);
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE))
        {
            if (--timeout == 0)
                break;
        }
        busy = (uint8_t)spi_i2s_data_receive(SPI2);
        busy &= 0x01;
    }
    timeout = SPIFLASH_SPI_TIMEOUT;
    while (SET == spi_i2s_flag_get(SPI2, SPI_FLAG_TRANS))
    {
        if (--timeout == 0)
            break;
    }
    SPIFLASH_CS_DIS();
}

void spiflash_Erase4k(uint32_t addr)
{
    uint8_t eraseSeq[4] = {0x20, (addr >> 16) & 0x000000ff, (addr >> 8) & 0x000000ff, (addr >> 0) & 0x000000ff};
    SPIFLASH_CS_EN();
    uint32_t timeout = SPIFLASH_SPI_TIMEOUT;
    for (uint8_t i = 0; i < 4; i++)
    {
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
        {
            if (--timeout == 0)
                break;
        }
        spi_i2s_data_transmit(SPI2, eraseSeq[i]);
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE))
        {
            if (--timeout == 0)
                break;
        }
        (void)spi_i2s_data_receive(SPI2);
    }
    timeout = SPIFLASH_SPI_TIMEOUT;
    while (SET == spi_i2s_flag_get(SPI2, SPI_FLAG_TRANS))
    {
        if (--timeout == 0)
            break;
    }
    SPIFLASH_CS_DIS();
}

void spiflash_Erase32k(uint32_t addr)
{
    uint8_t eraseSeq[4] = {0x52, (addr >> 16) & 0x000000ff, (addr >> 8) & 0x000000ff, (addr >> 0) & 0x000000ff};
    SPIFLASH_CS_EN();
    uint32_t timeout = SPIFLASH_SPI_TIMEOUT;
    for (uint8_t i = 0; i < 4; i++)
    {
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
        {
            if (--timeout == 0)
                break;
        }
        spi_i2s_data_transmit(SPI2, eraseSeq[i]);
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE))
        {
            if (--timeout == 0)
                break;
        }
        (void)spi_i2s_data_receive(SPI2);
    }
    timeout = SPIFLASH_SPI_TIMEOUT;
    while (SET == spi_i2s_flag_get(SPI2, SPI_FLAG_TRANS))
    {
        if (--timeout == 0)
            break;
    }
    SPIFLASH_CS_DIS();
}

void spiflash_WriteSector(uint32_t addr, uint8_t *buf, uint16_t len)
{
    if (len > 256)
        return;
    uint8_t writeSeq[4] = {0x02, (addr >> 16) & 0x000000ff, (addr >> 8) & 0x000000ff, (addr >> 0) & 0x000000ff};
    SPIFLASH_CS_EN();
    uint32_t timeout = SPIFLASH_SPI_TIMEOUT;
    for (uint8_t i = 0; i < 4; i++)
    {
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
        {
            if (--timeout == 0)
                break;
        }
        spi_i2s_data_transmit(SPI2, writeSeq[i]);
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE))
        {
            if (--timeout == 0)
                break;
        }
        (void)spi_i2s_data_receive(SPI2);
    }
    for (uint32_t i = 0; i < len; i++)
    {
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
        {
            if (--timeout == 0)
                break;
        }
        spi_i2s_data_transmit(SPI2, buf[i]);
        timeout = SPIFLASH_SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE))
        {
            if (--timeout == 0)
                break;
        }
        (void)spi_i2s_data_receive(SPI2);
    }
    timeout = SPIFLASH_SPI_TIMEOUT;
    while (SET == spi_i2s_flag_get(SPI2, SPI_FLAG_TRANS))
    {
        if (--timeout == 0)
            break;
    }
    SPIFLASH_CS_DIS();
}
