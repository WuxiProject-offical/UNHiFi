#include "tpa6130.h"
#include "i2c.h"
#include <stdio.h>
#include <stdint.h>

#define BUFFER_SIZE 256

uint8_t tpa6130_Init(void)
{
    tpa6130_RegWrite(1, 0xc0); // enable amp
    tpa6130_RegWrite(2, 0x00); // -59.5dB
    return 0;
}

uint8_t tpa6130_RegRead(uint8_t regAddr)
{
    uint8_t state = I2C_START;
    uint8_t read_cycle = 0;
    uint16_t timeout = 0;
    uint8_t i2c_timeout_flag = 0;
    uint8_t buffer = 0xff;

    /* enable acknowledge */
    i2c_ack_config(TPA6130_I2CX, I2C_ACK_ENABLE);

    while (!(i2c_timeout_flag))
    {
        switch (state)
        {
        case I2C_START:
            if (RESET == read_cycle)
            {
                /* i2c master sends start signal only when the bus is idle */
                while (i2c_flag_get(TPA6130_I2CX, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT))
                {
                    timeout++;
                }
                if (timeout >= I2C_TIME_OUT)
                {
                    i2c_bus_reset();
                    timeout = 0;
                    state = I2C_START;
                    printf("i2c bus is busy in READ!\n");
                }
            }
            /* send the start signal */
            i2c_start_on_bus(TPA6130_I2CX);
            timeout = 0;
            state = I2C_SEND_ADDRESS;
            break;
        case I2C_SEND_ADDRESS:
            /* i2c master sends START signal successfully */
            while ((!i2c_flag_get(TPA6130_I2CX, I2C_FLAG_SBSEND)) && (timeout < I2C_TIME_OUT))
            {
                timeout++;
            }
            if (timeout < I2C_TIME_OUT)
            {
                if (RESET == read_cycle)
                {
                    i2c_master_addressing(TPA6130_I2CX, TPA6130_ADDR, I2C_TRANSMITTER);
                    state = I2C_CLEAR_ADDRESS_FLAG;
                }
                else
                {
                    i2c_master_addressing(TPA6130_I2CX, TPA6130_ADDR, I2C_RECEIVER);
                    /* disable acknowledge */
                    i2c_ack_config(TPA6130_I2CX, I2C_ACK_DISABLE);
                    state = I2C_CLEAR_ADDRESS_FLAG;
                }
                timeout = 0;
            }
            else
            {
                timeout = 0;
                state = I2C_START;
                read_cycle = 0;
                printf("i2c master sends start signal timeout in READ!\n");
            }
            break;
        case I2C_CLEAR_ADDRESS_FLAG:
            /* address flag set means i2c slave sends ACK */
            while ((!i2c_flag_get(TPA6130_I2CX, I2C_FLAG_ADDSEND)) && (timeout < I2C_TIME_OUT))
            {
                timeout++;
            }
            if (timeout < I2C_TIME_OUT)
            {
                i2c_flag_clear(TPA6130_I2CX, I2C_FLAG_ADDSEND);
                if ((SET == read_cycle))
                {
                    /* send a stop condition to I2C bus */
                    i2c_stop_on_bus(TPA6130_I2CX);
                }
                timeout = 0;
                state = I2C_TRANSMIT_DATA;
            }
            else
            {
                timeout = 0;
                state = I2C_START;
                read_cycle = 0;
                printf("i2c master clears address flag timeout in READ!\n");
            }
            break;
        case I2C_TRANSMIT_DATA:
            if (RESET == read_cycle)
            {
                /* wait until the transmit data buffer is empty */
                while ((!i2c_flag_get(TPA6130_I2CX, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT))
                {
                    timeout++;
                }
                if (timeout < I2C_TIME_OUT)
                {
                    i2c_data_transmit(TPA6130_I2CX, regAddr);
                    timeout = 0;
                }
                else
                {
                    timeout = 0;
                    state = I2C_START;
                    read_cycle = 0;
                    printf("i2c master wait data buffer is empty timeout in READ!\n");
                }
                /* wait until BTC bit is set */
                while ((!i2c_flag_get(TPA6130_I2CX, I2C_FLAG_BTC)) && (timeout < I2C_TIME_OUT))
                {
                    timeout++;
                }
                if (timeout < I2C_TIME_OUT)
                {
                    timeout = 0;
                    state = I2C_START;
                    read_cycle++;
                }
                else
                {
                    timeout = 0;
                    state = I2C_START;
                    read_cycle = 0;
                    printf("i2c master sends device's internal address timeout in READ!\n");
                }
            }
            else
            {
                uint8_t flag = 1;
                while (flag)
                {
                    timeout++;
                    /* wait until RBNE bit is set */
                    if (i2c_flag_get(TPA6130_I2CX, I2C_FLAG_RBNE))
                    {
                        buffer = i2c_data_receive(TPA6130_I2CX);
                        flag = 0;
                        timeout = 0;
                    }
                    if (timeout > I2C_TIME_OUT)
                    {
                        timeout = 0;
                        state = I2C_START;
                        read_cycle = 0;
                        printf("i2c master sends data timeout in READ!\n");
                    }
                }
                timeout = 0;
                state = I2C_STOP;
            }
            break;
        case I2C_STOP:
            /* i2c master sends STOP signal successfully */
            while ((I2C_CTL0(TPA6130_I2CX) & I2C_CTL0_STOP) && (timeout < I2C_TIME_OUT))
            {
                timeout++;
            }
            if (timeout < I2C_TIME_OUT)
            {
                timeout = 0;
                state = I2C_END;
                i2c_timeout_flag = I2C_OK;
            }
            else
            {
                timeout = 0;
                state = I2C_START;
                read_cycle = 0;
                printf("i2c master sends stop signal timeout in READ!\n");
            }
            break;
        default:
            state = I2C_START;
            read_cycle = 0;
            i2c_timeout_flag = I2C_OK;
            timeout = 0;
            printf("i2c master sends start signal in READ.\n");
            break;
        }
    }
    return buffer;
}

void tpa6130_RegWrite(uint8_t regAddr, uint8_t regVal)
{
    uint8_t state = I2C_START;
    uint16_t timeout = 0;
    uint8_t i2c_timeout_flag = 0;

    /* enable acknowledge */
    i2c_ack_config(TPA6130_I2CX, I2C_ACK_ENABLE);

    while (!(i2c_timeout_flag))
    {
        switch (state)
        {
        case I2C_START:
            /* i2c master sends start signal only when the bus is idle */
            while (i2c_flag_get(TPA6130_I2CX, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT))
            {
                timeout++;
            }
            if (timeout < I2C_TIME_OUT)
            {
                i2c_start_on_bus(TPA6130_I2CX);
                timeout = 0;
                state = I2C_SEND_ADDRESS;
            }
            else
            {
                timeout = 0;
                state = I2C_START;
                printf("i2c bus is busy in WRITE BYTE!\n");
            }
            break;
        case I2C_SEND_ADDRESS:
            /* i2c master sends START signal successfully */
            while ((!i2c_flag_get(TPA6130_I2CX, I2C_FLAG_SBSEND)) && (timeout < I2C_TIME_OUT))
            {
                timeout++;
            }
            if (timeout < I2C_TIME_OUT)
            {
                i2c_master_addressing(TPA6130_I2CX, TPA6130_ADDR, I2C_TRANSMITTER);
                timeout = 0;
                state = I2C_CLEAR_ADDRESS_FLAG;
            }
            else
            {
                timeout = 0;
                state = I2C_START;
                printf("i2c master sends start signal timeout in WRITE BYTE!\n");
            }
            break;
        case I2C_CLEAR_ADDRESS_FLAG:
            /* address flag set means i2c slave sends ACK */
            while ((!i2c_flag_get(TPA6130_I2CX, I2C_FLAG_ADDSEND)) && (timeout < I2C_TIME_OUT))
            {
                timeout++;
            }
            if (timeout < I2C_TIME_OUT)
            {
                i2c_flag_clear(TPA6130_I2CX, I2C_FLAG_ADDSEND);
                timeout = 0;
                state = I2C_TRANSMIT_DATA;
            }
            else
            {
                timeout = 0;
                state = I2C_START;
                printf("i2c master clears address flag timeout in WRITE BYTE!\n");
            }
            break;
        case I2C_TRANSMIT_DATA:
            /* wait until the transmit data buffer is empty */
            while ((!i2c_flag_get(TPA6130_I2CX, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT))
            {
                timeout++;
            }
            if (timeout < I2C_TIME_OUT)
            {
                i2c_data_transmit(TPA6130_I2CX, regAddr);
                timeout = 0;
            }
            else
            {
                timeout = 0;
                state = I2C_START;
                printf("i2c master sends data timeout in WRITE BYTE!\n");
            }

            /* wait until BTC bit is set */
            while ((!i2c_flag_get(TPA6130_I2CX, I2C_FLAG_BTC)) && (timeout < I2C_TIME_OUT))
            {
                timeout++;
            }
            if (timeout < I2C_TIME_OUT)
            {
                i2c_data_transmit(TPA6130_I2CX, regVal);
                timeout = 0;
            }
            else
            {
                timeout = 0;
                state = I2C_START;
                printf("i2c master sends data timeout in WRITE BYTE!\n");
            }
            /* wait until BTC bit is set */
            while ((!i2c_flag_get(TPA6130_I2CX, I2C_FLAG_BTC)) && (timeout < I2C_TIME_OUT))
            {
                timeout++;
            }
            if (timeout < I2C_TIME_OUT)
            {
                state = I2C_STOP;
                timeout = 0;
            }
            else
            {
                timeout = 0;
                state = I2C_START;
                printf("i2c master sends data timeout in WRITE BYTE!\n");
            }
            break;
        case I2C_STOP:
            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(TPA6130_I2CX);
            /* i2c master sends STOP signal successfully */
            while ((I2C_CTL0(TPA6130_I2CX) & I2C_CTL0_STOP) && (timeout < I2C_TIME_OUT))
            {
                timeout++;
            }
            if (timeout < I2C_TIME_OUT)
            {
                timeout = 0;
                state = I2C_END;
                i2c_timeout_flag = I2C_OK;
            }
            else
            {
                timeout = 0;
                state = I2C_START;
                printf("i2c master sends stop signal timeout in WRITE BYTE!\n");
            }
            break;
        default:
            state = I2C_START;
            i2c_timeout_flag = I2C_OK;
            timeout = 0;
            printf("i2c master sends start signal in WRITE BYTE.\n");
            break;
        }
    }
}

void tpa6130_SetVolume(uint8_t vol)
{
    tpa6130_RegWrite(2, (vol <= 64) ? vol : 0xC0);
}
