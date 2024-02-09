#include "gd32f30x.h"
#include <stdio.h>

// Middlewares
#include "systick.h"
#include "i2c.h"
#include "spi.h"
#include "sdcard.h"

// BSPs
#include "tpa6130.h"
#include "lcd096.h"

#define arraysize 10

uint8_t spi1_send_array[arraysize] = {0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA};
uint8_t spi2_receive_array[arraysize];
void rcu_config(void);
void mydma_config(void);
void myspi_config(void);

sd_card_info_struct sd_cardinfo; /* information of SD card */
uint32_t buf_read[512];          /* store the data read from the card */

void gd_log_com_init()
{
    /* enable COM GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 921600U);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
}

sd_error_enum sd_io_init(void)
{
    sd_error_enum status = SD_OK;
    uint32_t cardstate = 0;
    status = sd_init();
    if (SD_OK == status)
    {
        status = sd_card_information_get(&sd_cardinfo);
    }
    if (SD_OK == status)
    {
        status = sd_card_select_deselect(sd_cardinfo.card_rca);
    }
    status = sd_cardstatus_get(&cardstate);
    if (cardstate & 0x02000000)
    {
        printf("\r\n the card is locked!");
        while (1)
        {
        }
    }
    if ((SD_OK == status) && (!(cardstate & 0x02000000)))
    {
        /* set bus mode */
        status = sd_bus_mode_config(SDIO_BUSMODE_4BIT);
        //        status = sd_bus_mode_config( SDIO_BUSMODE_1BIT );
    }
    if (SD_OK == status)
    {
        /* set data transfer mode */
        //        status = sd_transfer_mode_config( SD_DMA_MODE );
        status = sd_transfer_mode_config(SD_POLLING_MODE);
    }
    return status;
}
void card_info_get(void)
{
    uint8_t sd_spec, sd_spec3, sd_spec4, sd_security;
    uint32_t block_count, block_size;
    uint16_t temp_ccc;
    printf("\r\n Card information:");
    sd_spec = (sd_scr[1] & 0x0F000000) >> 24;
    sd_spec3 = (sd_scr[1] & 0x00008000) >> 15;
    sd_spec4 = (sd_scr[1] & 0x00000400) >> 10;
    if (2 == sd_spec)
    {
        if (1 == sd_spec3)
        {
            if (1 == sd_spec4)
            {
                printf("\r\n## Card version 4.xx ##");
            }
            else
            {
                printf("\r\n## Card version 3.0x ##");
            }
        }
        else
        {
            printf("\r\n## Card version 2.00 ##");
        }
    }
    else if (1 == sd_spec)
    {
        printf("\r\n## Card version 1.10 ##");
    }
    else if (0 == sd_spec)
    {
        printf("\r\n## Card version 1.0x ##");
    }

    sd_security = (sd_scr[1] & 0x00700000) >> 20;
    if (2 == sd_security)
    {
        printf("\r\n## SDSC card ##");
    }
    else if (3 == sd_security)
    {
        printf("\r\n## SDHC card ##");
    }
    else if (4 == sd_security)
    {
        printf("\r\n## SDXC card ##");
    }
    else
    {
        printf("\r\n## SD-%d card ##", sd_security);
    }

    block_count = (sd_cardinfo.card_csd.c_size + 1) * 1024;
    block_size = 512;
    printf("\r\n## Device size is %dKB ##", sd_card_capacity_get());
    printf("\r\n## Block size is %dB ##", block_size);
    printf("\r\n## Block count is %d ##", block_count);

    if (sd_cardinfo.card_csd.read_bl_partial)
    {
        printf("\r\n## Partial blocks for read allowed ##");
    }
    if (sd_cardinfo.card_csd.write_bl_partial)
    {
        printf("\r\n## Partial blocks for write allowed ##");
    }
    temp_ccc = sd_cardinfo.card_csd.ccc;
    printf("\r\n## CardCommandClasses is: %x ##", temp_ccc);
    if ((SD_CCC_BLOCK_READ & temp_ccc) && (SD_CCC_BLOCK_WRITE & temp_ccc))
    {
        printf("\r\n## Block operation supported ##");
    }
    if (SD_CCC_ERASE & temp_ccc)
    {
        printf("\r\n## Erase supported ##");
    }
    if (SD_CCC_WRITE_PROTECTION & temp_ccc)
    {
        printf("\r\n## Write protection supported ##");
    }
    if (SD_CCC_LOCK_CARD & temp_ccc)
    {
        printf("\r\n## Lock unlock supported ##");
    }
    if (SD_CCC_APPLICATION_SPECIFIC & temp_ccc)
    {
        printf("\r\n## Application specific supported ##");
    }
    if (SD_CCC_IO_MODE & temp_ccc)
    {
        printf("\r\n## I/O mode supported ##");
    }
    if (SD_CCC_SWITCH & temp_ccc)
    {
        printf("\r\n## Switch function supported ##");
    }
}

void sd_demo()
{
    sd_error_enum sd_error;
    uint16_t i = 5;

    /* configure the NVIC */
    nvic_irq_enable(SDIO_IRQn, 3, 0);

    /* initialize the card */
    do
    {
        sd_error = sd_io_init();
    } while ((SD_OK != sd_error) && (--i));

    if (i)
    {
        printf("\r\n Card init success!\r\n");
    }
    else
    {
        printf("\r\n Card init failed!\r\n");
        while (1)
        {
        }
    }

    /* get the information of the card and print it out by USART */
    card_info_get();

    printf("\r\n\r\n Card test:");

    /* single block operation test */
    sd_error = sd_block_read(buf_read, 100 * 512, 512);
    if (SD_OK != sd_error)
    {
        printf("\r\n Block read fail!");
        while (1)
        {
        }
    }
    else
    {
        printf("\r\n Block read success!");
    }

    /* lock and unlock operation test */
    if (SD_CCC_LOCK_CARD & sd_cardinfo.card_csd.ccc)
    {
        /* lock the card */
        sd_error = sd_lock_unlock(SD_LOCK);
        if (SD_OK != sd_error)
        {
            printf("\r\n Lock failed!");
            while (1)
            {
            }
        }
        else
        {
            printf("\r\n The card is locked!");
        }

        /* unlock the card */
        sd_error = sd_lock_unlock(SD_UNLOCK);
        if (SD_OK != sd_error)
        {
            printf("\r\n Unlock failed!");
            while (1)
            {
            }
        }
        else
        {
            printf("\r\n The card is unlocked!");
        }

        sd_error = sd_block_read(buf_read, 100 * 512, 512);
        if (SD_OK != sd_error)
        {
            printf("\r\n Block read fail!");
            while (1)
            {
            }
        }
        else
        {
            printf("\r\n Block read success!");
        }
    }

    /* multiple blocks operation test */
    sd_error = sd_multiblocks_read(buf_read, 200 * 512, 512, 3);
    if (SD_OK != sd_error)
    {
        printf("\r\n Multiple block read fail!");
        while (1)
        {
        }
    }
    else
    {
        printf("\r\n Multiple block read success!");
    }
}

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    // NVIC config
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    // GPIO remap config
    rcu_periph_clock_enable(RCU_AF);
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);

    spi2_config();
    i2c_config();

    systick_config();
    gd_log_com_init();

    LCD_Init();

    printf("\r\nCK_SYS is %d", rcu_clock_freq_get(CK_SYS));
    printf("\r\nCK_AHB is %d", rcu_clock_freq_get(CK_AHB));
    printf("\r\nCK_APB1 is %d", rcu_clock_freq_get(CK_APB1));
    printf("\r\nCK_APB2 is %d\r\n", rcu_clock_freq_get(CK_APB2));

    sd_demo();

    while (1)
    {
        LCD_Fill(0, 0, 160, 80, RED);
        delay_1ms(1000);
        LCD_Fill(0, 0, 160, 80, GREEN);
        delay_1ms(1000);
        LCD_Fill(0, 0, 160, 80, BLUE);
        delay_1ms(1000);
        LCD_Fill(0, 0, 160, 80, WHITE);
        delay_1ms(1000);
    }

    // mydma_config();

    // i2s_enable(SPI2);
    // i2s_enable(SPI1);

    // dma_channel_enable(DMA0, DMA_CH4);
    // dma_channel_enable(DMA1, DMA_CH0);

    // spi_dma_enable(SPI2, SPI_DMA_RECEIVE);
    // spi_dma_enable(SPI1, SPI_DMA_TRANSMIT);

    // while (!dma_flag_get(DMA0, DMA_CH4, DMA_INTF_FTFIF))
    //     ;
    // while (!dma_flag_get(DMA1, DMA_CH0, DMA_INTF_FTFIF))
    //     ;
    // while (1)
    //     ;
}

/*!
    \brief      configure different peripheral clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    rcu_periph_clock_enable(RCU_SPI1);
    rcu_periph_clock_enable(RCU_SPI2);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_DMA0);
    rcu_periph_clock_enable(RCU_DMA1);
}

/*!
    \brief      configure the DMA peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void mydma_config(void)
{
    dma_parameter_struct dma_init_struct;

    /* SPI1 transmit dma config: DMA0,DMA_CH4  */
    dma_deinit(DMA0, DMA_CH4);
    dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI1);
    dma_init_struct.memory_addr = (uint32_t)spi1_send_array;
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_LOW;
    dma_init_struct.number = arraysize;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init(DMA0, DMA_CH4, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH4);
    dma_memory_to_memory_disable(DMA0, DMA_CH4);

    /* SPI2 receive dma config: DMA1-DMA_CH0 */
    dma_deinit(DMA1, DMA_CH0);
    dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI2);
    dma_init_struct.memory_addr = (uint32_t)spi2_receive_array;
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_init(DMA1, DMA_CH0, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA1, DMA_CH0);
    dma_memory_to_memory_disable(DMA1, DMA_CH0);
}

/*!
    \brief      configure the SPI peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void myspi_config(void)
{
    spi_i2s_deinit(SPI1);
    spi_i2s_deinit(SPI2);

    i2s_init(SPI1, I2S_MODE_MASTERTX, I2S_STD_PHILLIPS, I2S_CKPL_LOW);
    i2s_psc_config(SPI1, I2S_AUDIOSAMPLE_11K, I2S_FRAMEFORMAT_DT16B_CH16B, I2S_MCKOUT_DISABLE);

    i2s_init(SPI2, I2S_MODE_SLAVERX, I2S_STD_PHILLIPS, I2S_CKPL_LOW);
    i2s_psc_config(SPI2, I2S_AUDIOSAMPLE_11K, I2S_FRAMEFORMAT_DT16B_CH16B, I2S_MCKOUT_DISABLE);
}

int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART0, (uint8_t)ch);
    while (RESET == usart_flag_get(USART0, USART_FLAG_TBE))
        ;

    return ch;
}
