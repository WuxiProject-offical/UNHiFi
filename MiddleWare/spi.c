#include "gd32f30x.h"
#include "spi.h"
#include <stdio.h>
#include "systick.h"

void spi2_config(void)
{
    // Clock prepare
    rcu_periph_clock_enable(RCU_GPIOB);
    //rcu_periph_clock_enable(RCU_DMA1); // for SPI2
    rcu_periph_clock_enable(RCU_SPI2);
    rcu_periph_clock_enable(RCU_AF);

    // GPIO config
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_5);
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_9);
    gpio_bit_set(GPIOB, GPIO_PIN_6 | GPIO_PIN_9); // LCD&FLASH CS H

    spi_parameter_struct spi_init_struct;
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.prescale = SPI_PSC_4; // 36.864M
    spi_init_struct.endian = SPI_ENDIAN_MSB;
    spi_init(SPI2, &spi_init_struct);
    spi_enable(SPI2);
}

