#ifndef _MYSD_H_
#define _MYSD_H_

#include "gd32f30x.h"
#include "sdcard.h"

sd_error_enum mysd_InitCard(sd_card_info_struct *sd_cardinfo);
//void mysd_GetCardInfo(sd_card_info_struct sd_cardinfo);

#endif
