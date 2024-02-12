#include "gd32f30x.h"
#include <stdio.h>
#include <string.h>

// Middlewares
#include "systick.h"
#include "i2c.h"
#include "spi.h"
#include "sdcard.h"
#include "ff.h"
#include "diskio.h"
#include "i2s.h"

// BSPs
#include "tpa6130.h"
#include "lcd096.h"
#include "mysd.h"
#include "spiflash.h"

uint16_t audio_buffer[256];

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

void audio_test(void)
{
    FATFS fs;
    FIL file;
    FRESULT res = FR_INVALID_PARAMETER;
    res = f_setcp(936);
    if (res)
    {
        printf("Set codepage failed: code %d\r\n", res);
        return;
    }
    res = disk_initialize(0);
    if (res)
    {
        printf("DiskInit failed: code %d\r\n", res);
        return;
    }
    res = f_mount(&fs, "", 1);
    if (res)
    {
        printf("Mount failed: code %d\r\n", res);
        return;
    }
    res = f_open(&file, "0:/¡¾™ÑÛà46¡¿2023.05.30 ¡¸Start over!¡¹ .wav", FA_READ);
    if (res)
    {
        printf("Open wave file failed: code %d\r\n", res);
        return;
    }
    f_lseek(&file, 44);
    i2s_config();
    tpa6130_RegWrite(1, 0xc0);
    tpa6130_RegWrite(2, 0x15); // -19.6dB
    uint8_t line[256], usingbuf = 0;
    uint32_t len1 = 256, len2 = 256;
    res = f_read(&file, &line, len1, &len2);
    if (res)
    {
        printf("Read wave file failed: code %d\r\n", res);
        return;
    }
    for (int i = 0; i < len2; i += 2)
    {
        audio_buffer[i / 2] = (line[i + 1] << 8) + line[i];
    }
    i2s_DMAConfig16(audio_buffer, len2 / 2);
    i2s_DMAStart();
    len1 = len2 = 256;
    while (len1 == len2)
    {
        res = f_read(&file, &line, len1, &len2);
        if (res)
        {
            printf("Read wave file failed: code %d\r\n", res);
            return;
        }
        if (usingbuf == 0)
        {
            for (int i = 0; i < len2; i += 2)
            {
                audio_buffer[128 + i / 2] = (line[i + 1] << 8) + line[i];
            }
            while (RESET == dma_flag_get(DMA0, DMA_CH4, DMA_INTF_FTFIF))
                ;
            i2s_DMAConfig16(audio_buffer + 128, len2 / 2);
            i2s_DMAStart();
            usingbuf = 1;
        }
        else
        {
            // using 1, fill 0 here.
            for (int i = 0; i < len2; i += 2)
            {
                audio_buffer[i / 2] = (line[i + 1] << 8) + line[i];
            }
            while (RESET == dma_flag_get(DMA0, DMA_CH4, DMA_INTF_FTFIF))
                ;
            i2s_DMAConfig16(audio_buffer, len2 / 2);
            i2s_DMAStart();
            usingbuf = 0;
        }
    }
    res = f_close(&file);
    if (res)
    {
        printf("Close text file failed: code %d\r\n", res);
        return;
    }
    res = f_mount(NULL, "", 0);
    if (res)
    {
        printf("Unmount failed: code %d\r\n", res);
        return;
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

    audio_test();

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
}

int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART0, (uint8_t)ch);
    while (RESET == usart_flag_get(USART0, USART_FLAG_TBE))
        ;

    return ch;
}
