#include "lcd096.h"
#include "gd32f30x.h"
#include "systick.h"
#include "spiflash.h"

#define LCD_CS_EN() (GPIO_BC(GPIOB) = (uint32_t)(GPIO_PIN_6))
#define LCD_CS_DIS() (GPIO_BOP(GPIOB) = (uint32_t)(GPIO_PIN_6))
#define LCD_DC_SET(x) gpio_bit_write(GPIOB, GPIO_PIN_7, (x))
#define LCD_RST_SET(x) gpio_bit_write(GPIOB, GPIO_PIN_8, (x))

void LCD_WR_DATA8(uint8_t dat)
{
	uint32_t timeout = LCD_SPI_TIMEOUT;
	// LCD_CS_EN();
	while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
	{
		if (--timeout == 0)
			break;
	}
	spi_i2s_data_transmit(SPI2, dat);
	timeout = LCD_SPI_TIMEOUT;
	while (SET == spi_i2s_flag_get(SPI2, SPI_FLAG_TRANS))
	{
		if (--timeout == 0)
			break;
	}
	// LCD_CS_DIS();
}

void LCD_WR_DATA(uint16_t dat)
{
	uint32_t timeout = LCD_SPI_TIMEOUT;
	// LCD_CS_EN();
	while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
	{
		if (--timeout == 0)
			break;
	}
	spi_i2s_data_transmit(SPI2, (dat >> 8) & 0x00ff);
	timeout = LCD_SPI_TIMEOUT;
	while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
	{
		if (--timeout == 0)
			break;
	}
	spi_i2s_data_transmit(SPI2, dat & 0x00ff);
	timeout = LCD_SPI_TIMEOUT;
	while (SET == spi_i2s_flag_get(SPI2, SPI_FLAG_TRANS))
	{
		if (--timeout == 0)
			break;
	}
	// LCD_CS_DIS();
}

void LCD_WR_REG(uint8_t dat)
{
	LCD_DC_SET(0);
	// LCD_CS_EN();
	uint32_t timeout = LCD_SPI_TIMEOUT;
	while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
	{
		if (--timeout == 0)
			break;
	}
	spi_i2s_data_transmit(SPI2, dat);
	timeout = LCD_SPI_TIMEOUT;
	while (SET == spi_i2s_flag_get(SPI2, SPI_FLAG_TRANS))
	{
		if (--timeout == 0)
			break;
	}
	// LCD_CS_DIS();
	LCD_DC_SET(1);
}

uint8_t _lcd_direction = 0;

void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_CS_EN();
	if (_lcd_direction == 0)
	{
		LCD_WR_REG(0x2a);
		LCD_WR_DATA(x1 + LCD_XOFFSET);
		LCD_WR_DATA(x2 + LCD_XOFFSET);
		LCD_WR_REG(0x2b);
		LCD_WR_DATA(y1 + LCD_YOFFSET);
		LCD_WR_DATA(y2 + LCD_YOFFSET);
		LCD_WR_REG(0x2c);
	}
	else
	{
		LCD_WR_REG(0x2a);
		LCD_WR_DATA(x1 + LCD_XOFFSET);
		LCD_WR_DATA(x2 + LCD_XOFFSET);
		LCD_WR_REG(0x2b);
		LCD_WR_DATA(y1 + LCD_YOFFSET);
		LCD_WR_DATA(y2 + LCD_YOFFSET);
		LCD_WR_REG(0x2c);
	}
	LCD_CS_DIS();
}

void LCD_Init(void)
{
	// GPIO config
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7 | GPIO_PIN_8);
	gpio_bit_set(GPIOB, GPIO_PIN_7 | GPIO_PIN_8); // LCD DC&RST H

	// Reset LCD
	LCD_RST_SET(0);
	delay_1ms(10);
	LCD_RST_SET(1);
	delay_1ms(130);

	// Init sequences
	LCD_CS_EN();
	LCD_WR_REG(0x11); // Sleep exit
	LCD_CS_DIS();
	delay_1ms(120); // Delay 120ms

	LCD_CS_EN();
	// Frame Rate Control in Normal
	LCD_WR_REG(0xB1);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x3C);

	// Frame Rate Control in Idle
	LCD_WR_REG(0xB2);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x3C);

	// Frame Rate Control in Partial
	LCD_WR_REG(0xB3);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x3C);

#ifdef LCD_REVERSE_COLOR
	LCD_WR_REG(0x21); // COLOR REVERSE
#endif

	// Display inversion
	LCD_WR_REG(0xB4);
	LCD_WR_DATA8(0x00);

	//	LCD_WR_REG(0xB1);
	//	LCD_WR_DATA8(0x01);
	//	LCD_WR_DATA8(0x2C);
	//	LCD_WR_DATA8(0x2D);

	// Power ctrl 1
	LCD_WR_REG(0xC0);
	LCD_WR_DATA8(0x0E);
	LCD_WR_DATA8(0x0E);
	LCD_WR_DATA8(0x04);

	// Power ctrl 2
	LCD_WR_REG(0xC1);
	LCD_WR_DATA8(0xC5);

	// Power ctrl 3 in Normal
	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x0d);
	LCD_WR_DATA8(0x00);

	// Power ctrl 4 in Idle
	LCD_WR_REG(0xC3);
	LCD_WR_DATA8(0x8D);
	LCD_WR_DATA8(0x2A);

	// Power ctrl 5 in Partial
	LCD_WR_REG(0xC4);
	LCD_WR_DATA8(0x8D);
	LCD_WR_DATA8(0xEE);

	// Power ctrl 6
	LCD_WR_REG(0xC5);	// VCOM
	LCD_WR_DATA8(0x06); // 1D  .06

	// Memory Data Access Control
	LCD_WR_REG(0x36); // MX, MY, RGB mode, LtR
	if (_lcd_direction == 0)
		LCD_WR_DATA8(0x78);
	else
		LCD_WR_DATA8(0xA8);

	// Interface Pixel Format
	LCD_WR_REG(0x3A);
	LCD_WR_DATA8(0x55); // 16-bit

	// Gamma +
	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0x0b);
	LCD_WR_DATA8(0x17);
	LCD_WR_DATA8(0x0a);
	LCD_WR_DATA8(0x0d);
	LCD_WR_DATA8(0x1a);
	LCD_WR_DATA8(0x19);
	LCD_WR_DATA8(0x16);
	LCD_WR_DATA8(0x1d);
	LCD_WR_DATA8(0x21);
	LCD_WR_DATA8(0x26);
	LCD_WR_DATA8(0x37);
	LCD_WR_DATA8(0x3c);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x09);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x10);

	// Gamma -
	LCD_WR_REG(0xE1);
	LCD_WR_DATA8(0x0c);
	LCD_WR_DATA8(0x19);
	LCD_WR_DATA8(0x09);
	LCD_WR_DATA8(0x0d);
	LCD_WR_DATA8(0x1b);
	LCD_WR_DATA8(0x19);
	LCD_WR_DATA8(0x15);
	LCD_WR_DATA8(0x1d);
	LCD_WR_DATA8(0x21);
	LCD_WR_DATA8(0x26);
	LCD_WR_DATA8(0x39);
	LCD_WR_DATA8(0x3E);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x09);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x10);
	LCD_CS_DIS();

	delay_1ms(150);
	LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
	LCD_CS_EN();
	LCD_WR_REG(0x29); // Display on
	LCD_CS_DIS();
}

void LCD_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color)
{
	uint16_t i, j;
	LCD_Address_Set(xsta, ysta, xend - 1, yend - 1); //??????
	LCD_CS_EN();
	for (i = ysta; i < yend; i++)
	{
		for (j = xsta; j < xend; j++)
		{
			LCD_WR_DATA(color);
		}
	}
	LCD_CS_DIS();
}

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
	LCD_Address_Set(x, y, x, y); //??????
	LCD_CS_EN();
	LCD_WR_DATA(color);
	LCD_CS_DIS();
}

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	uint16_t t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; //??????
	delta_y = y2 - y1;
	uRow = x1; //??????
	uCol = y1;
	if (delta_x > 0)
		incx = 1; //??????
	else if (delta_x == 0)
		incx = 0; //???
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0; //???
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y)
		distance = delta_x; //?????????
	else
		distance = delta_y;
	for (t = 0; t < distance + 1; t++)
	{
		LCD_DrawPoint(uRow, uCol, color); //??
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}

void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	LCD_DrawLine(x1, y1, x2, y1, color);
	LCD_DrawLine(x1, y1, x1, y2, color);
	LCD_DrawLine(x1, y2, x2, y2, color);
	LCD_DrawLine(x2, y1, x2, y2, color);
}

void Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
	int a, b;
	a = 0;
	b = r;
	while (a <= b)
	{
		LCD_DrawPoint(x0 - b, y0 - a, color); // 3
		LCD_DrawPoint(x0 + b, y0 - a, color); // 0
		LCD_DrawPoint(x0 - a, y0 + b, color); // 1
		LCD_DrawPoint(x0 - a, y0 - b, color); // 2
		LCD_DrawPoint(x0 + b, y0 + a, color); // 4
		LCD_DrawPoint(x0 + a, y0 - b, color); // 5
		LCD_DrawPoint(x0 + a, y0 + b, color); // 6
		LCD_DrawPoint(x0 - b, y0 + a, color); // 7
		a++;
		if ((a * a + b * b) > (r * r)) //??????????
		{
			b--;
		}
	}
}

void LCD_SetDirection(uint8_t rotate)
{
	_lcd_direction = rotate;
}

void LCD_ShowChinese(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
	while (*s != 0)
	{
		/*if (sizey == 12)
			LCD_ShowChinese12x12(x, y, s, fc, bc, sizey, mode);
		else*/
		if (sizey == 16)
			LCD_ShowChinese16x16(x, y, s, fc, bc, mode);
		/*else if (sizey == 24)
			LCD_ShowChinese24x24(x, y, s, fc, bc, sizey, mode);
		else if (sizey == 32)
			LCD_ShowChinese32x32(x, y, s, fc, bc, sizey, mode);*/
		else
			return;
		s += 2;
		x += sizey;
	}
}

uint32_t LCD_GetChineseFontAddr(uint16_t fontSize, uint8_t char1, uint8_t char2)
{
	uint32_t addr;
	uint16_t fontBinSize;
	/*if (fontSize == 12)
		{;}
	else*/
	if (fontSize == 16)
	{
		addr = 0x00000800UL;
		fontBinSize = 32;
		addr += fontBinSize * (0xff - 0x40) * (char1 - 0x81); // High byte
		addr += fontBinSize * (char2 - 0x40);
	}
	/*else if (sizey == 24)
		{;}
	else if (sizey == 32)
		{;}*/
	else
		return 0;
	return addr;
}

// void LCD_ShowChinese12x12(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
// {
// 	uint8_t i, j, m = 0;
// 	uint16_t k;
// 	uint16_t HZnum;		  //????
// 	uint16_t TypefaceNum; //??????????
// 	uint16_t x0 = x;
// 	TypefaceNum = (sizey / 8 + ((sizey % 8) ? 1 : 0)) * sizey;
// 	HZnum = sizeof(tfont12) / sizeof(typFNT_GB12); //??????
// 	for (k = 0; k < HZnum; k++)
// 	{
// 		if ((tfont12[k].Index[0] == *(s)) && (tfont12[k].Index[1] == *(s + 1)))
// 		{
// 			LCD_Address_Set(x, y, x + sizey - 1, y + sizey - 1);
// 			LCD_CS_EN();
// 			for (i = 0; i < TypefaceNum; i++)
// 			{
// 				for (j = 0; j < 8; j++)
// 				{
// 					if (!mode) //?????
// 					{
// 						if (tfont12[k].Msk[i] & (0x01 << j))
// 							LCD_WR_DATA(fc);
// 						else
// 							LCD_WR_DATA(bc);
// 						m++;
// 						if (m % sizey == 0)
// 						{
// 							m = 0;
// 							break;
// 						}
// 					}
// 					else //????
// 					{
// 						if (tfont12[k].Msk[i] & (0x01 << j))
// 						{
// 							LCD_Address_Set(x, y, x, y); //??????
// 							LCD_WR_DATA(fc);
// 						}
// 						x++;
// 						if ((x - x0) == sizey)
// 						{
// 							x = x0;
// 							y++;
// 							break;
// 						}
// 					}
// 				}
// 			}
// 			LCD_CS_EN();
// 		}
// 		continue; //?????????????,??????????????
// 	}
// }

void LCD_ShowChinese16x16(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t mode)
{
	const uint16_t fontBinSize = 32, fontSize = 16;
	uint16_t x0 = x, m = 0;
	uint8_t fontBin[32 /*fontBinSize*/];
	spiflash_Read(LCD_GetChineseFontAddr(fontSize, s[0], s[1]), fontBin, fontBinSize);
	LCD_Address_Set(x, y, x + fontSize - 1, y + fontSize - 1);
	LCD_CS_EN();
	for (uint16_t i = 0; i < fontBinSize; i++)
	{
		for (uint8_t j = 0; j < 8; j++)
		{
			if (!mode) // overwrite off it
			{
				if (fontBin[i] & (0x80 >> j))
					LCD_WR_DATA(fc);
				else
					LCD_WR_DATA(bc);
				m++;
				if (m % fontSize == 0) // avoid out of range
				{
					m = 0;
					break;
				}
			}
			else // overlay on it
			{
				if (fontBin[i] & (0x80 >> j))
				{
					LCD_Address_Set(x, y, x, y);
					LCD_WR_DATA(fc);
				}
				x++;
				if ((x - x0) == fontSize)
				{
					x = x0;
					y++;
					break;
				}
			}
		}
	}
	LCD_CS_DIS();
}

// void LCD_ShowChinese24x24(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
// {
// 	uint8_t i, j, m = 0;
// 	uint16_t k;
// 	uint16_t HZnum;		  //????
// 	uint16_t TypefaceNum; //??????????
// 	uint16_t x0 = x;
// 	TypefaceNum = (sizey / 8 + ((sizey % 8) ? 1 : 0)) * sizey;
// 	HZnum = sizeof(tfont24) / sizeof(typFNT_GB24); //??????
// 	for (k = 0; k < HZnum; k++)
// 	{
// 		if ((tfont24[k].Index[0] == *(s)) && (tfont24[k].Index[1] == *(s + 1)))
// 		{
// 			LCD_Address_Set(x, y, x + sizey - 1, y + sizey - 1);
// 			LCD_CS_EN();
// 			for (i = 0; i < TypefaceNum; i++)
// 			{
// 				for (j = 0; j < 8; j++)
// 				{
// 					if (!mode) //?????
// 					{
// 						if (tfont24[k].Msk[i] & (0x01 << j))
// 							LCD_WR_DATA(fc);
// 						else
// 							LCD_WR_DATA(bc);
// 						m++;
// 						if (m % sizey == 0)
// 						{
// 							m = 0;
// 							break;
// 						}
// 					}
// 					else //????
// 					{
// 						if (tfont24[k].Msk[i] & (0x01 << j))
// 						{
// 							LCD_Address_Set(x, y, x, y); //??????
// 							LCD_WR_DATA(fc);
// 						}
// 						x++;
// 						if ((x - x0) == sizey)
// 						{
// 							x = x0;
// 							y++;
// 							break;
// 						}
// 					}
// 				}
// 			}
// 			LCD_CS_DIS();
// 		}
// 		continue; //?????????????,??????????????
// 	}
// }

// void LCD_ShowChinese32x32(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
// {
// 	uint8_t i, j, m = 0;
// 	uint16_t k;
// 	uint16_t HZnum;		  //????
// 	uint16_t TypefaceNum; //??????????
// 	uint16_t x0 = x;
// 	TypefaceNum = (sizey / 8 + ((sizey % 8) ? 1 : 0)) * sizey;
// 	HZnum = sizeof(tfont32) / sizeof(typFNT_GB32); //??????
// 	for (k = 0; k < HZnum; k++)
// 	{
// 		if ((tfont32[k].Index[0] == *(s)) && (tfont32[k].Index[1] == *(s + 1)))
// 		{
// 			LCD_Address_Set(x, y, x + sizey - 1, y + sizey - 1);
// 			LCD_CS_EN();
// 			for (i = 0; i < TypefaceNum; i++)
// 			{
// 				for (j = 0; j < 8; j++)
// 				{
// 					if (!mode) //?????
// 					{
// 						if (tfont32[k].Msk[i] & (0x01 << j))
// 							LCD_WR_DATA(fc);
// 						else
// 							LCD_WR_DATA(bc);
// 						m++;
// 						if (m % sizey == 0)
// 						{
// 							m = 0;
// 							break;
// 						}
// 					}
// 					else //????
// 					{
// 						if (tfont32[k].Msk[i] & (0x01 << j))
// 						{
// 							LCD_Address_Set(x, y, x, y); //??????
// 							LCD_WR_DATA(fc);
// 						}
// 						x++;
// 						if ((x - x0) == sizey)
// 						{
// 							x = x0;
// 							y++;
// 							break;
// 						}
// 					}
// 				}
// 			}
// 			LCD_CS_DIS();
// 		}
// 		continue; //?????????????,??????????????
// 	}
// }

uint32_t LCD_GetASCIIFontAddr(uint16_t fontSize, uint8_t charcode)
{
	uint32_t addr;
	uint16_t fontBinSize;
	if (charcode > 0x7f)
		return 0;
	/*if (fontSize == 12)
		{;}
	else*/
	if (fontSize == 16)
	{
		addr = 0x00000000UL;
		fontBinSize = 16;
		addr += charcode * fontBinSize;
	}
	/*else if (sizey == 24)
		{;}
	else if (sizey == 32)
		{;}*/
	else
		return 0;
	return addr;
}

void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
	const uint16_t sizex = (sizey / 2);
	const uint16_t fontBinSize = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey, fontSize = sizey;
	uint16_t x0 = x, m = 0;
	uint8_t fontBin[32 /*max fontBinSize*/];
	spiflash_Read(LCD_GetASCIIFontAddr(fontSize, num), fontBin, fontBinSize);
	LCD_Address_Set(x, y, x + sizex - 1, y + sizey - 1);
	LCD_CS_EN();
	for (uint16_t i = 0; i < fontBinSize; i++)
	{
		for (uint8_t j = 0; j < 8; j++)
		{
			if (!mode) // overwrite off it
			{
				if (fontBin[i] & (0x80 >> j))
					LCD_WR_DATA(fc);
				else
					LCD_WR_DATA(bc);
				m++;
				if (m % sizex == 0) // avoid out of range
				{
					m = 0;
					break;
				}
			}
			else // overlay on it
			{
				if (fontBin[i] & (0x80 >> j))
				{
					LCD_Address_Set(x, y, x, y);
					LCD_WR_DATA(fc);
				}
				x++;
				if ((x - x0) == sizex)
				{
					x = x0;
					y++;
					break;
				}
			}
		}
	}
	LCD_CS_DIS();
}

void LCD_ShowString(uint16_t x, uint16_t y, const uint8_t *p, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
	while (*p != '\0')
	{
		LCD_ShowChar(x, y, *p, fc, bc, sizey, mode);
		x += sizey / 2;
		p++;
	}
}

uint32_t mypow(uint8_t m, uint8_t n)
{
	uint32_t result = 1;
	while (n--)
		result *= m;
	return result;
}

void LCD_ShowIntNum(uint16_t x, uint16_t y, uint16_t num, uint8_t len, uint16_t fc, uint16_t bc, uint8_t sizey)
{
	uint8_t t, temp;
	uint8_t enshow = 0;
	uint8_t sizex = sizey / 2;
	for (t = 0; t < len; t++)
	{
		temp = (num / mypow(10, len - t - 1)) % 10;
		if (enshow == 0 && t < (len - 1))
		{
			if (temp == 0)
			{
				LCD_ShowChar(x + t * sizex, y, ' ', fc, bc, sizey, 0);
				continue;
			}
			else
				enshow = 1;
		}
		LCD_ShowChar(x + t * sizex, y, temp + 48, fc, bc, sizey, 0);
	}
}

void LCD_ShowFloatNum1(uint16_t x, uint16_t y, float num, uint8_t len, uint16_t fc, uint16_t bc, uint8_t sizey)
{
	uint8_t t, temp, sizex;
	uint16_t num1;
	sizex = sizey / 2;
	num1 = num * 100;
	for (t = 0; t < len; t++)
	{
		temp = (num1 / mypow(10, len - t - 1)) % 10;
		if (t == (len - 2))
		{
			LCD_ShowChar(x + (len - 2) * sizex, y, '.', fc, bc, sizey, 0);
			t++;
			len += 1;
		}
		LCD_ShowChar(x + t * sizex, y, temp + 48, fc, bc, sizey, 0);
	}
}

void LCD_ShowPicture(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t pic[])
{
	uint16_t i, j;
	uint32_t k = 0;
	LCD_Address_Set(x, y, x + length - 1, y + width - 1);
	LCD_CS_EN();
	for (i = 0; i < length; i++)
	{
		for (j = 0; j < width; j++)
		{
			LCD_WR_DATA8(pic[k * 2]);
			LCD_WR_DATA8(pic[k * 2 + 1]);
			k++;
		}
	}
	LCD_CS_DIS();
}