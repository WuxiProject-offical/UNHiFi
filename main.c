#include "gd32f30x.h"
#include <stdio.h>

// Middlewares
#include "systick.h"
#include "i2c.h"
#include "spi.h"

// BSPs
#include "tpa6130.h"
#include "lcd096.h"

#define arraysize 10

uint8_t spi1_send_array[arraysize] = {0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA};
uint8_t spi2_receive_array[arraysize];
void rcu_config(void);
void mydma_config(void);
void myspi_config(void);

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

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* peripheral clock enable */
    rcu_config();
    // GPIO remap config
    rcu_periph_clock_enable(RCU_AF);
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);

    spi2_config();
    // i2c_config();

    systick_config();
    gd_log_com_init();

    LCD_Init();

    printf("\r\nCK_SYS is %d", rcu_clock_freq_get(CK_SYS));
    printf("\r\nCK_AHB is %d", rcu_clock_freq_get(CK_AHB));
    printf("\r\nCK_APB1 is %d", rcu_clock_freq_get(CK_APB1));
    printf("\r\nCK_APB2 is %d\r\n", rcu_clock_freq_get(CK_APB2));

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
