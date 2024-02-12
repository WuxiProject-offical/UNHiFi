/*------------------------------------------------------------------------*/
/* Unicode Handling Functions for FatFs R0.13 and Later                   */
/*------------------------------------------------------------------------*/
/* This module will occupy a huge memory in the .rodata section when the  */
/* FatFs is configured for LFN with DBCS. If the system has a Unicode     */
/* library for the code conversion, this module should be modified to use */
/* it to avoid silly memory consumption.                                  */
/*------------------------------------------------------------------------*/
/*
/ Copyright (C) 2022, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
*/

#include "ff.h"
#include <stdint.h>
#include "spiflash.h"

#if FF_USE_LFN != 0 /* This module will be blanked if in non-LFN configuration */

#define MERGE2(a, b) a##b
#define CVTBL(tbl, cp) MERGE2(tbl, cp)

/*------------------------------------------------------------------------*/
/* Code Conversion Tables                                                 */
/*------------------------------------------------------------------------*/
#define FFU_U2O_932_BASEADDR 0x0BD000
#define FFU_U2O_932_LEN (2 * 14780UL)
#define FFU_O2U_932_BASEADDR 0x0C5000
#define FFU_O2U_932_LEN (2 * 14780UL)
#define FFU_U2O_936_BASEADDR 0x0CD000
#define FFU_U2O_936_LEN (2 * 43586UL)
#define FFU_O2U_936_BASEADDR 0x0E3000
#define FFU_O2U_936_LEN (2 * 43586UL)
#define FFU_U2O_949_BASEADDR 0x0F9000
#define FFU_U2O_949_LEN (2 * 34098UL)
#define FFU_O2U_949_BASEADDR 0x10A000
#define FFU_O2U_949_LEN (2 * 34098UL)
#define FFU_U2O_950_BASEADDR 0x11B000
#define FFU_U2O_950_LEN (2 * 27008UL)
#define FFU_O2U_950_BASEADDR 0x128800
#define FFU_O2U_950_LEN (2 * 27008UL)
#define FFU_UC437_BASEADDR 0x136000
#define FFU_UC720_BASEADDR 0x136100
#define FFU_UC737_BASEADDR 0x136200
#define FFU_UC771_BASEADDR 0x136300
#define FFU_UC775_BASEADDR 0x136400
#define FFU_UC850_BASEADDR 0x136500
#define FFU_UC852_BASEADDR 0x136600
#define FFU_UC855_BASEADDR 0x136700
#define FFU_UC857_BASEADDR 0x136800
#define FFU_UC860_BASEADDR 0x136900
#define FFU_UC861_BASEADDR 0x136A00
#define FFU_UC862_BASEADDR 0x136B00
#define FFU_UC863_BASEADDR 0x136C00
#define FFU_UC864_BASEADDR 0x136D00
#define FFU_UC865_BASEADDR 0x136E00
#define FFU_UC866_BASEADDR 0x136F00
#define FFU_UC869_BASEADDR 0x137000

/*------------------------------------------------------------------------*/
/* OEM <==> Unicode Conversions for Dynamic Code Page Configuration       */
/*------------------------------------------------------------------------*/

#if FF_CODE_PAGE == 0

static const WORD cp_code[] = {437, 720, 737, 771, 775, 850, 852, 855, 857, 860, 861, 862, 863, 864, 865, 866, 869, 0};
// static const WCHAR *const cp_table[] = {uc437, uc720, uc737, uc771, uc775, uc850, uc852, uc855, uc857, uc860, uc861, uc862, uc863, uc864, uc865, uc866, uc869, 0};
static const uint32_t cp_addr_table[] = {
	FFU_UC437_BASEADDR,
	FFU_UC720_BASEADDR,
	FFU_UC737_BASEADDR,
	FFU_UC771_BASEADDR,
	FFU_UC775_BASEADDR,
	FFU_UC850_BASEADDR,
	FFU_UC852_BASEADDR,
	FFU_UC855_BASEADDR,
	FFU_UC857_BASEADDR,
	FFU_UC860_BASEADDR,
	FFU_UC861_BASEADDR,
	FFU_UC862_BASEADDR,
	FFU_UC863_BASEADDR,
	FFU_UC864_BASEADDR,
	FFU_UC865_BASEADDR,
	FFU_UC866_BASEADDR,
	FFU_UC869_BASEADDR,
	0};

WCHAR ff_uni2oem(			/* Returns OEM code character, zero on error */
				 DWORD uni, /* UTF-16 encoded character to be converted */
				 WORD cp	/* Code page for the conversion */
)
{
	uint32_t tableaddr;
	WCHAR c = 0, uc;
	UINT i, n, li, hi;

	if (uni < 0x80)
	{ /* ASCII? */
		c = (WCHAR)uni;
	}
	else
	{ /* Non-ASCII */
		if (uni < 0x10000)
		{ /* Is it in BMP? */
			uc = (WCHAR)uni;
			tableaddr = 0;
			if (cp < 900)
			{ /* SBCS */
				for (i = 0; cp_code[i] != 0 && cp_code[i] != cp; i++)
					; /* Get conversion table */
				tableaddr = cp_addr_table[i];
				uint8_t tbuf[256];
				if (tableaddr)
				{ /* Is it valid code page ? */
					spiflash_Read(tableaddr + c, tbuf, 256);
					for (c = 0; c < 0x80 && uc != ((((uint16_t)tbuf[c]) << 8) + tbuf[c + 1]); c++)
						; /* Find OEM code in the table */
					c = (c + 0x80) & 0xFF;
				}
			}
			else
			{ /* DBCS */
				switch (cp)
				{ /* Get conversion table */
				case 932:
					tableaddr = FFU_U2O_932_BASEADDR;
					hi = FFU_U2O_932_LEN / 4 - 1;
					break;
				case 936:
					tableaddr = FFU_U2O_936_BASEADDR;
					hi = FFU_U2O_936_LEN / 4 - 1;
					break;
				case 949:
					tableaddr = FFU_U2O_949_BASEADDR;
					hi = FFU_U2O_949_LEN / 4 - 1;
					break;
				case 950:
					tableaddr = FFU_U2O_950_BASEADDR;
					hi = FFU_U2O_950_LEN / 4 - 1;
					break;
				}
				if (tableaddr)
				{ /* Is it valid code page? */
					li = 0;
					uint16_t utcode;
					uint8_t tbuf[2];
					for (n = 16; n; n--)
					{ /* Find OEM code */
						i = li + (hi - li) / 2;
						spiflash_Read(tableaddr + i * 4, tbuf, 2);
						utcode = (((uint16_t)tbuf[0]) << 8) + tbuf[1];
						if (uc == utcode)
							break;
						if (uc > utcode)
						{
							li = i;
						}
						else
						{
							hi = i;
						}
					}
					if (n != 0)
					{
						spiflash_Read(tableaddr + i * 4 + 2, tbuf, 2);
						c = (((uint16_t)tbuf[0]) << 8) + tbuf[1];
					}
				}
			}
		}
	}

	return c;
}

WCHAR ff_oem2uni(			/* Returns Unicode character in UTF-16, zero on error */
				 WCHAR oem, /* OEM code to be converted (DBC if >=0x100) */
				 WORD cp	/* Code page for the conversion */
)
{
	uint32_t tableaddr;
	WCHAR c = 0;
	UINT i, n, li, hi;

	if (oem < 0x80)
	{ /* ASCII? */
		c = oem;
	}
	else
	{ /* Extended char */
		tableaddr = 0;
		if (cp < 900)
		{ /* SBCS */
			for (i = 0; cp_code[i] != 0 && cp_code[i] != cp; i++)
				; /* Get table */
			tableaddr = cp_addr_table[i];
			if (tableaddr)
			{ /* Is it a valid CP ? */
				uint8_t tbuf[2];
				if (oem < 0x100)
				{
					spiflash_Read(tableaddr + oem - 0x80, tbuf, 2);
					c = (((uint16_t)tbuf[0]) << 8) + tbuf[1];
				}
			}
		}
		else
		{ /* DBCS */
			switch (cp)
			{
			case 932:
				tableaddr = FFU_O2U_932_BASEADDR;
				hi = FFU_O2U_932_LEN / 4 - 1;
				break;
			case 936:
				tableaddr = FFU_O2U_936_BASEADDR;
				hi = FFU_O2U_936_LEN / 4 - 1;
				break;
			case 949:
				tableaddr = FFU_O2U_949_BASEADDR;
				hi = FFU_O2U_949_LEN / 4 - 1;
				break;
			case 950:
				tableaddr = FFU_O2U_950_BASEADDR;
				hi = FFU_O2U_950_LEN / 4 - 1;
				break;
			}
			if (tableaddr)
			{
				li = 0;
				uint16_t otcode;
				uint8_t tbuf[2];
				for (n = 16; n; n--)
				{
					i = li + (hi - li) / 2;
					spiflash_Read(tableaddr + i * 4, tbuf, 2);
					otcode = (((uint16_t)tbuf[0]) << 8) + tbuf[1];
					if (oem == otcode)
						break;
					if (oem > otcode)
					{
						li = i;
					}
					else
					{
						hi = i;
					}
				}
				if (n != 0)
				{
					spiflash_Read(tableaddr + i * 4 + 2, tbuf, 2);
					c = (((uint16_t)tbuf[0]) << 8) + tbuf[1];
				}
			}
		}
	}

	return c;
}
#endif

/*------------------------------------------------------------------------*/
/* Unicode Up-case Conversion                                             */
/*------------------------------------------------------------------------*/

DWORD ff_wtoupper(			/* Returns up-converted code point */
				  DWORD uni /* Unicode code point to be up-converted */
)
{
	const WORD *p;
	WORD uc, bc, nc, cmd;
	static const WORD cvt1[] = {
		/* Compressed up conversion table for U+0000 - U+0FFF */
		/* Basic Latin */
		0x0061, 0x031A,
		/* Latin-1 Supplement */
		0x00E0, 0x0317,
		0x00F8, 0x0307,
		0x00FF, 0x0001, 0x0178,
		/* Latin Extended-A */
		0x0100, 0x0130,
		0x0132, 0x0106,
		0x0139, 0x0110,
		0x014A, 0x012E,
		0x0179, 0x0106,
		/* Latin Extended-B */
		0x0180, 0x004D, 0x0243, 0x0181, 0x0182, 0x0182, 0x0184, 0x0184, 0x0186, 0x0187, 0x0187, 0x0189, 0x018A, 0x018B, 0x018B, 0x018D, 0x018E, 0x018F, 0x0190, 0x0191, 0x0191, 0x0193, 0x0194, 0x01F6, 0x0196, 0x0197, 0x0198, 0x0198, 0x023D, 0x019B, 0x019C, 0x019D, 0x0220, 0x019F, 0x01A0, 0x01A0, 0x01A2, 0x01A2, 0x01A4, 0x01A4, 0x01A6, 0x01A7, 0x01A7, 0x01A9, 0x01AA, 0x01AB, 0x01AC, 0x01AC, 0x01AE, 0x01AF, 0x01AF, 0x01B1, 0x01B2, 0x01B3, 0x01B3, 0x01B5, 0x01B5, 0x01B7, 0x01B8, 0x01B8, 0x01BA, 0x01BB, 0x01BC, 0x01BC, 0x01BE, 0x01F7, 0x01C0, 0x01C1, 0x01C2, 0x01C3, 0x01C4, 0x01C5, 0x01C4, 0x01C7, 0x01C8, 0x01C7, 0x01CA, 0x01CB, 0x01CA,
		0x01CD, 0x0110,
		0x01DD, 0x0001, 0x018E,
		0x01DE, 0x0112,
		0x01F3, 0x0003, 0x01F1, 0x01F4, 0x01F4,
		0x01F8, 0x0128,
		0x0222, 0x0112,
		0x023A, 0x0009, 0x2C65, 0x023B, 0x023B, 0x023D, 0x2C66, 0x023F, 0x0240, 0x0241, 0x0241,
		0x0246, 0x010A,
		/* IPA Extensions */
		0x0253, 0x0040, 0x0181, 0x0186, 0x0255, 0x0189, 0x018A, 0x0258, 0x018F, 0x025A, 0x0190, 0x025C, 0x025D, 0x025E, 0x025F, 0x0193, 0x0261, 0x0262, 0x0194, 0x0264, 0x0265, 0x0266, 0x0267, 0x0197, 0x0196, 0x026A, 0x2C62, 0x026C, 0x026D, 0x026E, 0x019C, 0x0270, 0x0271, 0x019D, 0x0273, 0x0274, 0x019F, 0x0276, 0x0277, 0x0278, 0x0279, 0x027A, 0x027B, 0x027C, 0x2C64, 0x027E, 0x027F, 0x01A6, 0x0281, 0x0282, 0x01A9, 0x0284, 0x0285, 0x0286, 0x0287, 0x01AE, 0x0244, 0x01B1, 0x01B2, 0x0245, 0x028D, 0x028E, 0x028F, 0x0290, 0x0291, 0x01B7,
		/* Greek, Coptic */
		0x037B, 0x0003, 0x03FD, 0x03FE, 0x03FF,
		0x03AC, 0x0004, 0x0386, 0x0388, 0x0389, 0x038A,
		0x03B1, 0x0311,
		0x03C2, 0x0002, 0x03A3, 0x03A3,
		0x03C4, 0x0308,
		0x03CC, 0x0003, 0x038C, 0x038E, 0x038F,
		0x03D8, 0x0118,
		0x03F2, 0x000A, 0x03F9, 0x03F3, 0x03F4, 0x03F5, 0x03F6, 0x03F7, 0x03F7, 0x03F9, 0x03FA, 0x03FA,
		/* Cyrillic */
		0x0430, 0x0320,
		0x0450, 0x0710,
		0x0460, 0x0122,
		0x048A, 0x0136,
		0x04C1, 0x010E,
		0x04CF, 0x0001, 0x04C0,
		0x04D0, 0x0144,
		/* Armenian */
		0x0561, 0x0426,

		0x0000 /* EOT */
	};
	static const WORD cvt2[] = {
		/* Compressed up conversion table for U+1000 - U+FFFF */
		/* Phonetic Extensions */
		0x1D7D, 0x0001, 0x2C63,
		/* Latin Extended Additional */
		0x1E00, 0x0196,
		0x1EA0, 0x015A,
		/* Greek Extended */
		0x1F00, 0x0608,
		0x1F10, 0x0606,
		0x1F20, 0x0608,
		0x1F30, 0x0608,
		0x1F40, 0x0606,
		0x1F51, 0x0007, 0x1F59, 0x1F52, 0x1F5B, 0x1F54, 0x1F5D, 0x1F56, 0x1F5F,
		0x1F60, 0x0608,
		0x1F70, 0x000E, 0x1FBA, 0x1FBB, 0x1FC8, 0x1FC9, 0x1FCA, 0x1FCB, 0x1FDA, 0x1FDB, 0x1FF8, 0x1FF9, 0x1FEA, 0x1FEB, 0x1FFA, 0x1FFB,
		0x1F80, 0x0608,
		0x1F90, 0x0608,
		0x1FA0, 0x0608,
		0x1FB0, 0x0004, 0x1FB8, 0x1FB9, 0x1FB2, 0x1FBC,
		0x1FCC, 0x0001, 0x1FC3,
		0x1FD0, 0x0602,
		0x1FE0, 0x0602,
		0x1FE5, 0x0001, 0x1FEC,
		0x1FF3, 0x0001, 0x1FFC,
		/* Letterlike Symbols */
		0x214E, 0x0001, 0x2132,
		/* Number forms */
		0x2170, 0x0210,
		0x2184, 0x0001, 0x2183,
		/* Enclosed Alphanumerics */
		0x24D0, 0x051A,
		0x2C30, 0x042F,
		/* Latin Extended-C */
		0x2C60, 0x0102,
		0x2C67, 0x0106, 0x2C75, 0x0102,
		/* Coptic */
		0x2C80, 0x0164,
		/* Georgian Supplement */
		0x2D00, 0x0826,
		/* Full-width */
		0xFF41, 0x031A,

		0x0000 /* EOT */
	};

	if (uni < 0x10000)
	{ /* Is it in BMP? */
		uc = (WORD)uni;
		p = uc < 0x1000 ? cvt1 : cvt2;
		for (;;)
		{
			bc = *p++; /* Get the block base */
			if (bc == 0 || uc < bc)
				break; /* Not matched? */
			nc = *p++;
			cmd = nc >> 8;
			nc &= 0xFF; /* Get processing command and block size */
			if (uc < bc + nc)
			{ /* In the block? */
				switch (cmd)
				{
				case 0:
					uc = p[uc - bc];
					break; /* Table conversion */
				case 1:
					uc -= (uc - bc) & 1;
					break; /* Case pairs */
				case 2:
					uc -= 16;
					break; /* Shift -16 */
				case 3:
					uc -= 32;
					break; /* Shift -32 */
				case 4:
					uc -= 48;
					break; /* Shift -48 */
				case 5:
					uc -= 26;
					break; /* Shift -26 */
				case 6:
					uc += 8;
					break; /* Shift +8 */
				case 7:
					uc -= 80;
					break; /* Shift -80 */
				case 8:
					uc -= 0x1C60;
					break; /* Shift -0x1C60 */
				}
				break;
			}
			if (cmd == 0)
				p += nc; /* Skip table if needed */
		}
		uni = uc;
	}

	return uni;
}

#endif /* #if FF_USE_LFN != 0 */
