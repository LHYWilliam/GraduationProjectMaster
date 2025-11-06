#include <stdarg.h>
#include <stdio.h>

#include "cmsis_os2.h"

#include "LCD.h"
#include "LCD_Font.h"

#define LCD_ReadDATA() LCD1->LCD_RAM
#define LCD_WriteREG(REG) LCD1->LCD_REG = (REG);
#define LCD_WriteDATA(RAM) LCD1->LCD_RAM = (RAM);
#define LCD_WriteGRAM(Self) LCD1->LCD_REG = Self->GRAMCMD;

void LCD_ReadID(LCD_t *Self);
void LCD_ConfigReg(LCD_t *Self);

void LCD_WriteRST(LCD_t *Self, GPIO_PinState State)
{
  HAL_GPIO_WritePin(Self->RST_Port, Self->RST_Pin, State);
}

void LCD_WriteBLK(LCD_t *Self, GPIO_PinState State)
{
  HAL_GPIO_WritePin(Self->BLK_Port, Self->BLK_Pin, State);
}

void LCD_Init(LCD_t *Self)
{
  osDelay(50);

  LCD_WriteRST(Self, GPIO_PIN_SET);
  osDelay(10);
  LCD_WriteRST(Self, GPIO_PIN_RESET);
  osDelay(50);
  LCD_WriteRST(Self, GPIO_PIN_SET);
  osDelay(200);

  LCD_ReadID(Self);
  LCD_ConfigReg(Self);

  FSMC_Bank1E->BWTR[0] &= ~(0XF << 0);
  FSMC_Bank1E->BWTR[0] &= ~(0XF << 8);
  FSMC_Bank1E->BWTR[0] |= 3 << 0;

  FSMC_Bank1E->BWTR[0] |= 2 << 8;

  LCD_DisplayOff(Self);

  LCD_SetFont(Self, Self->Font);
  LCD_SetPenColor(Self, BLACK);
  LCD_SetBackgroundColor(Self, WHITE);
  LCD_SetRotation(Self, Self->Rotation);
  LCD_Clear(Self);

  LCD_DisplayOn(Self);
}


void LCD_DisplayOn(LCD_t *Self)
{
  LCD_WriteREG(0X29);
  LCD_WriteBLK(Self, GPIO_PIN_SET);
}

void LCD_DisplayOff(LCD_t *Self)
{
  LCD_WriteREG(0X28);
  LCD_WriteBLK(Self, GPIO_PIN_RESET);
}

void LCD_SetCursor(LCD_t *Self, uint16_t X, uint16_t Y)
{
  LCD_WriteREG(Self->SetXCMD);
  LCD_WriteDATA(X >> 8);
  LCD_WriteDATA(X & 0XFF);

  LCD_WriteREG(Self->SetYCMD);
  LCD_WriteDATA(Y >> 8);
  LCD_WriteDATA(Y & 0XFF);
}

void LCD_SetWindow(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height)
{
  uint16_t X2 = X + Width - 1, Y2 = Y + Height - 1;

  LCD_WriteREG(Self->SetXCMD);
  LCD_WriteDATA(X >> 8);
  LCD_WriteDATA(X & 0XFF);
  LCD_WriteDATA(X2 >> 8);
  LCD_WriteDATA(X2 & 0XFF);

  LCD_WriteREG(Self->SetYCMD);
  LCD_WriteDATA(Y >> 8);
  LCD_WriteDATA(Y & 0XFF);
  LCD_WriteDATA(Y2 >> 8);
  LCD_WriteDATA(Y2 & 0XFF);
}

void LCD_SetFont(LCD_t *Self, LCD_Font Font)
{
  Self->Font = Font;

  switch (Self->Font)
  {
  case LCDFont6x12:
    Self->FontWidth = 6;
    Self->FontHight = 12;
    break;
  }
}

void LCD_SetRotation(LCD_t *Self, LCD_Rotation Rotation)
{
  Self->Rotation = Rotation;

  switch (Self->Rotation)
  {
  case Rotation0:
    LCD_SetDisplayDirection(Self, Display_Vertical);
    LCD_SetScanDirection(Self, Scan_L2R_U2D);
    break;

  case Rotation90:
    LCD_SetDisplayDirection(Self, Display_Horizontal);
    LCD_SetScanDirection(Self, Scan_U2D_R2L);
    break;

  case Rotation180:
    LCD_SetDisplayDirection(Self, Display_Vertical);
    LCD_SetScanDirection(Self, Scan_R2L_D2U);
    break;

  case Rotation270:
    LCD_SetDisplayDirection(Self, Display_Horizontal);
    LCD_SetScanDirection(Self, Scan_D2U_L2R);
    break;
  }
}

void LCD_SetDisplayDirection(LCD_t *Self, LCD_DisplayDirection DisplayDirection)
{
  Self->DisplayDirection = DisplayDirection;

  if (Self->DisplayDirection == Display_Vertical)
  {
    Self->Width = 320;
    Self->Height = 480;
  } else if (Self->DisplayDirection == Display_Horizontal)
  {
    Self->Width = 480;
    Self->Height = 320;
  }

  Self->GRAMCMD = 0X2C;
  Self->SetXCMD = 0X2A;
  Self->SetYCMD = 0X2B;
}

uint16_t ScanRegValue[] = {
    (0 << 7) | (0 << 6) | (0 << 5),
    (1 << 7) | (0 << 6) | (0 << 5),
    (0 << 7) | (1 << 6) | (0 << 5),
    (1 << 7) | (1 << 6) | (0 << 5),
    (0 << 7) | (0 << 6) | (1 << 5),
    (0 << 7) | (1 << 6) | (1 << 5),
    (1 << 7) | (0 << 6) | (1 << 5),
    (1 << 7) | (1 << 6) | (1 << 5),
};

void LCD_SetScanDirection(LCD_t *Self, LCD_ScanDirection ScanDirection)
{
  Self->ScanDirection = ScanDirection;

  uint16_t Value = 0;
  Value |= ScanRegValue[Self->ScanDirection];

  LCD_WriteREG(0X36);
  LCD_WriteDATA(Value);
}

void LCD_SetPenColor(LCD_t *Self, uint16_t Color)
{
  Self->PenColor = Color;
}

void LCD_SetBackgroundColor(LCD_t *Self, uint16_t Color)
{
  Self->BackgroundColor = Color;
}

void LCD_Clear(LCD_t *Self)
{
  LCD_ClearArea(Self, 0, 0, Self->Width, Self->Height);
}

void LCD_ClearArea(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height)
{
  uint16_t Color = Self->BackgroundColor;
  uint32_t TotalPoint = Width * Height;

  LCD_SetWindow(Self, X, Y, Width, Height);
  LCD_WriteGRAM(Self);
  for (uint32_t i = 0; i < TotalPoint; i++)
  {
    LCD_WriteDATA(Color);
  }
}

void LCD_Fill(LCD_t *Self)
{
  LCD_FillArea(Self, 0, 0, Self->Width, Self->Height);
}

void LCD_FillArea(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height)
{
  uint16_t Color = Self->PenColor;
  uint32_t TotalPoint = Width * Height;

  LCD_SetWindow(Self, X, Y, Width, Height);
  LCD_WriteGRAM(Self);
  for (uint32_t i = 0; i < TotalPoint; i++)
  {
    LCD_WriteDATA(Color);
  }
}

void LCD_Delay(uint8_t i)
{
  while (i--)
  {
  }
}

uint16_t LCD_ReadPoint(LCD_t *Self, uint16_t X, uint16_t Y)
{
  if (X < 0 || Y < 0 || X >= Self->Width || Y >= Self->Height)
  {
    return 0;
  }

  uint16_t R = 0, G = 0, B = 0;

  LCD_SetCursor(Self, X, Y);
  LCD_WriteREG(0X2E);

  R = LCD_ReadDATA();
  LCD_Delay(2);

  R = LCD_ReadDATA();
  LCD_Delay(2);

  B = LCD_ReadDATA();

  G = R & 0XFF;
  G <<= 8;

  return (((R >> 11) << 11) | ((G >> 10) << 5) | (B >> 11));
}

void LCD_DrawPoint(LCD_t *Self, uint16_t X, uint16_t Y)
{
  LCD_SetCursor(Self, X, Y);
  LCD_WriteGRAM(Self);
  LCD_WriteDATA(Self->PenColor);
}

void LCD_DrawPointWithColor(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Color)
{
  LCD_SetCursor(Self, X, Y);
  LCD_WriteGRAM(Self);
  LCD_WriteDATA(Color);
}

void LCD_DrawHLine(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Width)
{
  LCD_FillArea(Self, X, Y, Width, 1);
}

void LCD_DrawVLine(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Hight)
{
  LCD_FillArea(Self, X, Y, 1, Hight);
}

void LCD_DrawLine(LCD_t *Self, uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2)
{
  int32_t xerr = 0, yerr = 0, delta_x = X2 - X1, delta_y = Y2 - Y1, distance;
  int32_t incx, incy, uRow = X1, uCol = Y1;

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
    LCD_DrawPoint(Self, uRow, uCol);

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

void LCD_DrawRectangle(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Hight)
{
  LCD_DrawHLine(Self, X, Y, Width);
  LCD_DrawHLine(Self, X, Y + Hight - 1, Width);
  LCD_DrawVLine(Self, X, Y, Hight);
  LCD_DrawVLine(Self, X + Width - 1, Y, Hight);
}

void LCD_DrawCircle(LCD_t *Self, uint16_t X, uint16_t Y, uint8_t Radius)
{
  int32_t a = 0, b = Radius, di = 3 - (Radius << 1);

  while (a <= b)
  {
    LCD_DrawPoint(Self, X + a, Y - b);
    LCD_DrawPoint(Self, X + b, Y - a);
    LCD_DrawPoint(Self, X + b, Y + a);
    LCD_DrawPoint(Self, X + a, Y + b);
    LCD_DrawPoint(Self, X - a, Y + b);
    LCD_DrawPoint(Self, X - b, Y + a);
    LCD_DrawPoint(Self, X - a, Y - b);
    LCD_DrawPoint(Self, X - b, Y - a);
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

void LCD_ShowImage(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, const uint8_t *Image)
{
  LCD_SetWindow(Self, X, Y, Width, Height);
  LCD_WriteGRAM(Self);

  if (Self->hDMAx)
  {
    uint32_t RestPoint = Width * Height, AlreadyPoint = 0;
    do
    {
      uint32_t PointToShow = RestPoint >= 65535 ? 65535 : RestPoint;

      HAL_DMA_Start(Self->hDMAx, (uint32_t) (Image + AlreadyPoint * 2), (uint32_t) &LCD1->LCD_RAM, PointToShow);
      while (HAL_DMA_PollForTransfer(Self->hDMAx, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY) != HAL_OK)
        ;

      RestPoint -= PointToShow;
      AlreadyPoint += PointToShow;
    } while (RestPoint);

  } else
  {
    uint32_t TotalPoint = Width * Height;
    for (uint32_t i = 0; i < TotalPoint; i++)
    {
      LCD_WriteDATA((Image[2 * i + 1] << 8) | Image[2 * i]);
    }
  }
}

#define LCD_ShowCharWithFont(Self, X, Y, Char, Font)                  \
  do                                                                  \
  {                                                                   \
    uint8_t Index = Char - ' ';                                       \
    uint8_t *Bitmap = (uint8_t *) Font[Index];                        \
    uint8_t BytesPerColumn = (Self->FontHight + 7) / 8;               \
    for (uint16_t j = Y; j < Y + Self->FontHight; j++)                \
    {                                                                 \
      uint8_t BytesNowColumn = (j - Y) / 8;                           \
      uint8_t Mask = 1 << (7 - ((j - Y) % 8));                        \
      for (uint16_t i = X; i < X + Self->FontWidth; i++)              \
      {                                                               \
        if (Bitmap[(i - X) * BytesPerColumn + BytesNowColumn] & Mask) \
        {                                                             \
          LCD_DrawPoint(Self, i, j);                                  \
        }                                                             \
      }                                                               \
    }                                                                 \
  } while (0)

void LCD_ShowChar(LCD_t *Self, uint16_t X, uint16_t Y, uint8_t Char)
{
  switch (Self->Font)
  {
  case LCDFont6x12:
    LCD_ShowCharWithFont(Self, X, Y, Char, LCDFont6x12Bitmap);
    break;
  }
}


void LCD_ShowString(LCD_t *Self, uint16_t X, uint16_t Y, char *String)
{
  for (uint16_t i = 0; String[i]; i++)
  {
    LCD_ShowChar(Self, X, Y, String[i]);

    X += Self->FontWidth;
  }
}

void LCD_Printf(LCD_t *Self, uint16_t X, uint16_t Y, char *Format, ...)
{
  va_list Args;
  va_start(Args, Format);
  int32_t Length = vsnprintf((char *) Self->PrintfBuffer, sizeof(Self->PrintfBuffer), Format, Args);
  va_end(Args);

  if (Length > 0)
  {
    LCD_ShowString(Self, X, Y, (char *) Self->PrintfBuffer);
  }
}

void LCD_ReadID(LCD_t *Self)
{
  LCD_WriteREG(0XD4);
  Self->ID = LCD_ReadDATA();
  Self->ID = LCD_ReadDATA();
  Self->ID = LCD_ReadDATA();
  Self->ID <<= 8;
  Self->ID |= LCD_ReadDATA();
}

void LCD_ConfigReg(LCD_t *Self)
{
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
}
