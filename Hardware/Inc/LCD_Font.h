#ifndef __LCD_FONT_H__
#define __LCD_FONT_H__

#include "stm32f4xx_hal.h"

typedef enum
{
  LCDFont6x12,
} LCD_Font;

extern const uint8_t LCDFont6x12Bitmap[][12];

#endif
