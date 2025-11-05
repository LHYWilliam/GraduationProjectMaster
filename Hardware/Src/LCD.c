#include <stdarg.h>
#include <stdio.h>

#include "cmsis_os2.h"

#include "LCD.h"
#include "LCD_Font.h"

#define LCD_WriteREG(Reg) LCD1->LCD_REG = (Reg);

#define LCD_WriteDATA(Ram) LCD1->LCD_RAM = (Ram);

#define LCD_WriteGRAM(lcd) LCD1->LCD_REG = lcd->GRAMCMD;

#define LCD_ReadDATA() LCD1->LCD_RAM

void LCD_DisplayOn(void) { LCD_WriteREG(0X29); }

void LCD_DisplayOff(void) { LCD_WriteREG(0X28); }

void LCD_SetCursor(LCD_t *Self, uint16_t x, uint16_t y)
{
  LCD_WriteREG(Self->SetXCMD);
  LCD_WriteDATA(x >> 8);
  LCD_WriteDATA(x & 0XFF);

  LCD_WriteREG(Self->SetYCMD);
  LCD_WriteDATA(y >> 8);
  LCD_WriteDATA(y & 0XFF);
}

void LCD_SetWindow(LCD_t *Self, uint16_t x1, uint16_t y1, uint16_t Width, uint16_t Height)
{
  uint16_t x2 = x1 + Width - 1, y2 = y1 + Height - 1;

  LCD_WriteREG(Self->SetXCMD);
  LCD_WriteDATA(x1 >> 8);
  LCD_WriteDATA(x1 & 0XFF);
  LCD_WriteDATA(x2 >> 8);
  LCD_WriteDATA(x2 & 0XFF);

  LCD_WriteREG(Self->SetYCMD);
  LCD_WriteDATA(y1 >> 8);
  LCD_WriteDATA(y1 & 0XFF);
  LCD_WriteDATA(y2 >> 8);
  LCD_WriteDATA(y2 & 0XFF);
}

void LCD_SetDisplayDirection(LCD_t *Self)
{
  if (Self->Direction == LCD_Vertical)
  {
    Self->Width = 320;
    Self->Height = 480;
    Self->GRAMCMD = 0X2C;
    Self->SetXCMD = 0X2A;
    Self->SetYCMD = 0X2B;
    Self->ScanDirection = Vertical_ScanDirection;
  } else if (Self->Direction == LCD_Horizontal)
  {
    Self->Width = 480;
    Self->Height = 320;
    Self->GRAMCMD = 0X2C;
    Self->SetXCMD = 0X2A;
    Self->SetYCMD = 0X2B;
    Self->ScanDirection = Horizontal_ScanDirection;
  }
}

void LCD_SetScanDirection(LCD_t *Self)
{
  uint16_t regval = 0;
  switch (Self->ScanDirection)
  {
  case L2R_U2D:
    regval |= (0 << 7) | (0 << 6) | (0 << 5);
    break;

  case L2R_D2U:
    regval |= (1 << 7) | (0 << 6) | (0 << 5);
    break;

  case R2L_U2D:
    regval |= (0 << 7) | (1 << 6) | (0 << 5);
    break;

  case R2L_D2U:
    regval |= (1 << 7) | (1 << 6) | (0 << 5);
    break;

  case U2D_L2R:
    regval |= (0 << 7) | (0 << 6) | (1 << 5);
    break;

  case U2D_R2L:
    regval |= (0 << 7) | (1 << 6) | (1 << 5);
    break;

  case D2U_L2R:
    regval |= (1 << 7) | (0 << 6) | (1 << 5);
    break;

  case D2U_R2L:
    regval |= (1 << 7) | (1 << 6) | (1 << 5);
    break;
  }

  LCD_WriteREG(0X36);
  LCD_WriteDATA(regval);

  uint16_t temp;
  if (regval & 0X20)
  {
    if (Self->Width < Self->Height)
    {
      temp = Self->Width;
      Self->Width = Self->Height;
      Self->Height = temp;
    }
  } else
  {
    if (Self->Width > Self->Height)
    {
      temp = Self->Width;
      Self->Width = Self->Height;
      Self->Height = temp;
    }
  }
}

void LCD_SetPointColor(LCD_t *Self, uint16_t Color)
{
  Self->PointColor = Color;
}

void LCD_SetBackColor(LCD_t *Self, uint16_t Color) { Self->BackColor = Color; }

void LCD_Clear(LCD_t *Self, uint16_t Color)
{
  LCD_SetWindow(Self, 0, 0, Self->Width, Self->Height);

  LCD_WriteGRAM(Self);
  uint32_t totalPoint = Self->Width * Self->Height;
  for (uint32_t index = 0; index < totalPoint; index++)
  {
    LCD_WriteDATA(Color);
  }
}

void LCD_Fill(LCD_t *Self, uint16_t x, uint16_t y, uint16_t Width, uint16_t Height, uint16_t Color)
{
  LCD_SetWindow(Self, x, y, Width, Height);

  LCD_WriteGRAM(Self);
  uint32_t totalPoint = Width * Height;
  for (uint32_t i = 0; i < totalPoint; i++)
  {
    LCD_WriteDATA(Color);
  }
}

uint16_t LCD_ReadPoint(LCD_t *Self, uint16_t x, uint16_t y)
{
  uint16_t r = 0, g = 0, b = 0;
  if (x >= Self->Width || y >= Self->Height)
    return 0;
  LCD_SetCursor(Self, x, y);

  LCD_WriteREG(0X2E);

  r = LCD_ReadDATA();

  uint8_t i = 2;
  while (i--)
    ;
  r = LCD_ReadDATA();

  i = 2;
  while (i--)
    ;
  b = LCD_ReadDATA();
  g = r & 0XFF;

  g <<= 8;

  return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));
}

void LCD_DrawPoint(LCD_t *Self, uint16_t x, uint16_t y, uint16_t Color)
{
  LCD_SetCursor(Self, x, y);
  LCD_WriteGRAM(Self);
  LCD_WriteDATA(Color);
}

void LCD_DrawLine(LCD_t *Self, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  int32_t xerr = 0, yerr = 0, delta_x = x2 - x1, delta_y = y2 - y1, distance;
  int32_t incx, incy, uRow = x1, uCol = y1;

  if (delta_x > 0)
    incx = 1;
  else if (delta_x == 0)
    incx = 0;
  else
  {
    incx = -1;
    delta_x = -delta_x;
  }

  if (delta_y > 0)
    incy = 1;
  else if (delta_y == 0)
    incy = 0;
  else
  {
    incy = -1;
    delta_y = -delta_y;
  }

  if (delta_x > delta_y)
  {
    distance = delta_x;
  } else
  {
    distance = delta_y;
  }

  for (uint16_t t = 0; t <= distance + 1; t++)
  {
    LCD_DrawPoint(Self, uRow, uCol, Self->PointColor);

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

void LCD_DrawRectangle(LCD_t *Self, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  LCD_DrawLine(Self, x1, y1, x2, y1);
  LCD_DrawLine(Self, x1, y1, x1, y2);
  LCD_DrawLine(Self, x1, y2, x2, y2);
  LCD_DrawLine(Self, x2, y1, x2, y2);
}

void LCD_DrawCircle(LCD_t *Self, uint16_t x0, uint16_t y0, uint8_t r)
{
  int32_t a = 0, b = r, di = 3 - (r << 1);

  while (a <= b)
  {
    LCD_DrawPoint(Self, x0 + a, y0 - b, Self->PointColor);
    LCD_DrawPoint(Self, x0 + b, y0 - a, Self->PointColor);
    LCD_DrawPoint(Self, x0 + b, y0 + a, Self->PointColor);
    LCD_DrawPoint(Self, x0 + a, y0 + b, Self->PointColor);
    LCD_DrawPoint(Self, x0 - a, y0 + b, Self->PointColor);
    LCD_DrawPoint(Self, x0 - b, y0 + a, Self->PointColor);
    LCD_DrawPoint(Self, x0 - a, y0 - b, Self->PointColor);
    LCD_DrawPoint(Self, x0 - b, y0 - a, Self->PointColor);
    a++;

    if (di < 0)
    {
      di += 4 * a + 6;
    } else
    {
      di += 10 + 4 * (a - b);
      b--;
    }
  }
}

void LCD_ShowImage(LCD_t *Self, uint16_t x, uint16_t y, uint16_t Width, uint16_t Height, const uint8_t *image)
{
  LCD_SetWindow(Self, x, y, Width, Height);
  LCD_WriteGRAM(Self);

  if (Self->hDMAx)
  {
    uint32_t restPoint = Width * Height, alreadyPoint = 0;
    do
    {
      uint32_t pointToShow = restPoint >= 65535 ? 65535 : restPoint;

      HAL_DMA_Start(Self->hDMAx, (uint32_t) (image + alreadyPoint * 2), (uint32_t) &LCD1->LCD_RAM, pointToShow);
      while (HAL_DMA_PollForTransfer(Self->hDMAx, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY) != HAL_OK)
        ;

      restPoint -= pointToShow;
      alreadyPoint += pointToShow;
    } while (restPoint);

  } else
  {
    uint32_t totalPoint = Width * Height;
    for (uint32_t i = 0; i < totalPoint; i++)
    {
      LCD_WriteDATA((image[2 * i + 1] << 8) | image[2 * i]);
    }
  }
}

void LCD_ShowChar(LCD_t *Self, uint16_t x, uint16_t y, uint8_t num, uint8_t Size, uint8_t mode)
{
  uint16_t y0 = y;
  num = num - ' ';

  uint8_t csize = (Size / 8 + ((Size % 8) ? 1 : 0)) * (Size / 2);
  for (uint8_t t = 0; t < csize; t++)
  {
    uint8_t temp;
    if (Size == 12)
    {
      temp = asc2_1206[num][t];
    } else
    {
      return;
    }

    for (uint8_t t1 = 0; t1 < 8; t1++)
    {
      if (temp & 0x80)
      {
        LCD_DrawPoint(Self, x, y, Self->PointColor);
      } else if (mode == 0)
      {
        LCD_DrawPoint(Self, x, y, Self->BackColor);
      }

      temp <<= 1;
      y++;

      if (y >= Self->Height)
      {
        return;
      }

      if ((y - y0) == Size)
      {
        y = y0;
        x++;

        if (x >= Self->Width)
        {
          return;
        }

        break;
      }
    }
  }
}


void LCD_ShowString(LCD_t *Self, uint16_t x, uint16_t y, uint16_t Width, uint16_t Height, uint8_t Size, char *string)
{
  uint8_t x0 = x;
  Width += x;
  Height += y;

  while ((*string <= '~') && (*string >= ' '))
  {
    if (x >= Width)
    {
      x = x0;
      y += Size;
    }

    if (y >= Height)
    {
      break;
    }

    LCD_ShowChar(Self, x, y, *string, Size, 0);
    x += Size / 2;
    string++;
  }
}

void LCD_Printf(LCD_t *Self, uint16_t x, uint16_t y, uint16_t Width, uint16_t Height, uint8_t Size, char *Format, ...)
{
  va_list Args;
  va_start(Args, Format);
  vsprintf((char *) Self->PrintfBuffer, Format, Args);
  va_end(Args);

  LCD_ShowString(Self, x, y, Width, Height, Size, (char *) Self->PrintfBuffer);
}

void LCD_Init(LCD_t *Self)
{
  osDelay(50);

  LCD_RST_SET();
  osDelay(10);
  LCD_RST_RESET();
  osDelay(50);
  LCD_RST_SET();
  osDelay(200);

  LCD_WriteREG(0XD3);
  Self->ID = LCD_ReadDATA();
  Self->ID = LCD_ReadDATA();
  Self->ID = LCD_ReadDATA();
  Self->ID <<= 8;
  Self->ID |= LCD_ReadDATA();

  LCD_WriteREG(0X04);
  Self->ID = LCD_ReadDATA();
  Self->ID = LCD_ReadDATA();
  Self->ID = LCD_ReadDATA();
  Self->ID <<= 8;
  Self->ID |= LCD_ReadDATA() & 0XFF;

  LCD_WriteREG(0XD4);
  Self->ID = LCD_ReadDATA();
  Self->ID = LCD_ReadDATA();
  Self->ID = LCD_ReadDATA();
  Self->ID <<= 8;
  Self->ID |= LCD_ReadDATA();

  LCD_WriteREG(0xED);
  LCD_WriteDATA(0x01);
  LCD_WriteDATA(0xFE);

  LCD_WriteREG(0xEE);
  LCD_WriteDATA(0xDE);
  LCD_WriteDATA(0x21);

  LCD_WriteREG(0xF1);
  LCD_WriteDATA(0x01);
  LCD_WriteREG(0xDF);
  LCD_WriteDATA(0x10);

  LCD_WriteREG(0xC4);
  LCD_WriteDATA(0x8F);

  LCD_WriteREG(0xC6);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xE2);
  LCD_WriteDATA(0xE2);
  LCD_WriteDATA(0xE2);
  LCD_WriteREG(0xBF);
  LCD_WriteDATA(0xAA);

  LCD_WriteREG(0xB0);
  LCD_WriteDATA(0x0D);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x0D);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x11);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x19);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x21);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x2D);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x3D);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x5D);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x5D);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xB1);
  LCD_WriteDATA(0x80);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x8B);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x96);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xB2);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x02);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x03);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xB3);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xB4);
  LCD_WriteDATA(0x8B);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x96);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xA1);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xB5);
  LCD_WriteDATA(0x02);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x03);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x04);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xB6);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xB7);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x3F);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x5E);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x64);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x8C);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xAC);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xDC);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x70);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x90);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xEB);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xDC);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xB8);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xBA);
  LCD_WriteDATA(0x24);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xC1);
  LCD_WriteDATA(0x20);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x54);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xFF);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xC2);
  LCD_WriteDATA(0x0A);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x04);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xC3);
  LCD_WriteDATA(0x3C);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x3A);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x39);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x37);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x3C);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x36);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x32);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x2F);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x2C);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x29);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x26);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x24);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x24);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x23);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x3C);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x36);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x32);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x2F);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x2C);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x29);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x26);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x24);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x24);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x23);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xC4);
  LCD_WriteDATA(0x62);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x05);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x84);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xF0);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x18);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xA4);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x18);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x50);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x0C);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x17);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x95);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xF3);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xE6);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xC5);
  LCD_WriteDATA(0x32);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x44);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x65);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x76);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x88);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xC6);
  LCD_WriteDATA(0x20);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x17);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x01);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xC7);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xC8);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xC9);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xE0);
  LCD_WriteDATA(0x16);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x1C);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x21);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x36);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x46);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x52);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x64);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x7A);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x8B);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x99);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xA8);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xB9);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xC4);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xCA);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xD2);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xD9);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xE0);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xF3);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xE1);
  LCD_WriteDATA(0x16);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x1C);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x22);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x36);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x45);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x52);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x64);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x7A);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x8B);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x99);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xA8);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xB9);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xC4);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xCA);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xD2);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xD8);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xE0);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xF3);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xE2);
  LCD_WriteDATA(0x05);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x0B);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x1B);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x34);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x44);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x4F);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x61);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x79);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x88);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x97);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xA6);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xB7);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xC2);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xC7);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xD1);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xD6);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xDD);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xF3);
  LCD_WriteDATA(0x00);
  LCD_WriteREG(0xE3);
  LCD_WriteDATA(0x05);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xA);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x1C);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x33);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x44);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x50);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x62);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x78);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x88);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x97);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xA6);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xB7);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xC2);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xC7);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xD1);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xD5);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xDD);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xF3);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xE4);
  LCD_WriteDATA(0x01);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x01);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x02);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x2A);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x3C);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x4B);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x5D);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x74);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x84);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x93);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xA2);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xB3);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xBE);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xC4);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xCD);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xD3);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xDD);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xF3);
  LCD_WriteDATA(0x00);
  LCD_WriteREG(0xE5);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x02);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x29);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x3C);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x4B);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x5D);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x74);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x84);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x93);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xA2);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xB3);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xBE);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xC4);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xCD);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xD3);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xDC);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xF3);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xE6);
  LCD_WriteDATA(0x11);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x34);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x56);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x76);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x77);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x66);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x88);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x99);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xBB);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x99);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x66);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x55);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x55);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x45);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x43);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x44);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xE7);
  LCD_WriteDATA(0x32);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x55);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x76);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x66);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x67);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x67);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x87);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x99);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xBB);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x99);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x77);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x44);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x56);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x23);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x33);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x45);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xE8);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x99);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x87);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x88);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x77);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x66);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x88);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xAA);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0xBB);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x99);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x66);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x55);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x55);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x44);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x44);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x55);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xE9);
  LCD_WriteDATA(0xAA);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0x00);
  LCD_WriteDATA(0xAA);

  LCD_WriteREG(0xCF);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xF0);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x50);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xF3);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0xF9);
  LCD_WriteDATA(0x06);
  LCD_WriteDATA(0x10);
  LCD_WriteDATA(0x29);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0x3A);
  LCD_WriteDATA(0x55);

  LCD_WriteREG(0x11);
  osDelay(100);
  LCD_WriteREG(0x29);
  LCD_WriteREG(0x35);
  LCD_WriteDATA(0x00);

  LCD_WriteREG(0x51);
  LCD_WriteDATA(0xFF);
  LCD_WriteREG(0x53);
  LCD_WriteDATA(0x2C);
  LCD_WriteREG(0x55);
  LCD_WriteDATA(0x82);
  LCD_WriteREG(0x2c);

  FSMC_Bank1E->BWTR[0] &= ~(0XF << 0);
  FSMC_Bank1E->BWTR[0] &= ~(0XF << 8);
  FSMC_Bank1E->BWTR[0] |= 3 << 0;

  FSMC_Bank1E->BWTR[0] |= 2 << 8;

  LCD_SetDisplayDirection(Self);
  LCD_SetScanDirection(Self);
  LCD_Clear(Self, WHITE);
  LCD_SetBackColor(Self, WHITE);
  LCD_SetPointColor(Self, BLACK);

  LCD_BLK_SET();
}
