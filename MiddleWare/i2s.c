#include "gd32f30x.h"
#include "i2s.h"
#include <stdio.h>
#include "systick.h"

void i2s_config(void)
{
    // Clock prepare
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_DMA0); // for SPI1
    rcu_periph_clock_enable(RCU_SPI1);
    rcu_periph_clock_enable(RCU_AF);

    // GPIO config
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_15);
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);

    // SPI/I2S config
    spi_i2s_deinit(SPI1);
    i2s_init(SPI1, I2S_MODE_MASTERTX, I2S_STD_PHILLIPS, I2S_CKPL_HIGH);
    i2s_psc_config(SPI1, I2S_AUDIOSAMPLE_48K, I2S_FRAMEFORMAT_DT16B_CH16B, I2S_MCKOUT_ENABLE);
    i2s_enable(SPI1);
}

void i2s_SetFs(uint32_t fsopt)
{
    SPI_I2SPSC(SPI1) = (uint32_t)(fsopt | I2S_MCKOUT_ENABLE);
}

void i2s_SetFrameFormat(uint32_t i2s_frameformat)
{
    SPI_I2SCTL(SPI1) &= (uint32_t)(~(SPI_I2SCTL_DTLEN | SPI_I2SCTL_CHLEN));
    SPI_I2SCTL(SPI1) |= (uint32_t)i2s_frameformat;
}

void i2s_DMASetSrc(void *srcaddr, uint32_t len)
{
    DMA_CHMADDR(DMA0, DMA_CH4) = (uint32_t)srcaddr;
    DMA_CHCNT(DMA0, DMA_CH4) = (((uint32_t)len) & DMA_CHANNEL_CNT_MASK);
}

void i2s_DMAConfig32(uint32_t *srcbuf, uint32_t len)
{
    dma_parameter_struct dma_init_struct;
    /* SPI1 transmit dma config: DMA0,DMA_CH4  */
    dma_deinit(DMA0, DMA_CH4);
    dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI1);
    dma_init_struct.memory_addr = (uint32_t)srcbuf;
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_32BIT;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_init_struct.number = len;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init(DMA0, DMA_CH4, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH4);
    dma_memory_to_memory_disable(DMA0, DMA_CH4);
}

void i2s_DMAConfig16(uint16_t *srcbuf, uint32_t len)
{
    dma_parameter_struct dma_init_struct;
    /* SPI1 transmit dma config: DMA0,DMA_CH4  */
    dma_deinit(DMA0, DMA_CH4);
    dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI1);
    dma_init_struct.memory_addr = (uint32_t)srcbuf;
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_init_struct.number = len;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init(DMA0, DMA_CH4, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH4);
    dma_memory_to_memory_disable(DMA0, DMA_CH4);
}

void i2s_DMAStart()
{
    DMA_CHCTL(DMA0, DMA_CH4) |= DMA_CHXCTL_CHEN;
    SPI_CTL1(SPI1) |= (uint32_t)SPI_CTL1_DMATEN;
}

void i2s_DMAStop()
{
    DMA_CHCTL(DMA0, DMA_CH4) &= ~DMA_CHXCTL_CHEN;
    SPI_CTL1(SPI1) &= (uint32_t)(~SPI_CTL1_DMATEN);
}