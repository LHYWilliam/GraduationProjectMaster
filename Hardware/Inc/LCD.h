#ifndef __LCD_H__
#define __LCD_H__

#include "stm32f4xx_hal.h"

#include "LCD_Font.h"

#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40
#define BRRED 0XFC07
#define GRAY 0X8430
#define DARKBLUE 0X01CF
#define LIGHTBLUE 0X7D7C
#define GRAYBLUE 0X5458
#define LIGHTGREEN 0X841F
#define LGRAY 0XC618
#define LGRAYBLUE 0XA651
#define LBBLUE 0X2B12

#define LCD_BASE ((uint32_t) (0x60000000 + (0x4000000 * (1 - 1))) | (((1 << 18) * 2) - 2))
#define LCD1 ((LCD_TypeDef *) LCD_BASE)

typedef struct
{
  volatile uint16_t LCD_REG;
  volatile uint16_t LCD_RAM;
} LCD_TypeDef;

typedef enum
{
  Display_Vertical,
  Display_Horizontal,
} LCD_DisplayDirection;

typedef enum
{
  Scan_L2R_U2D,
  Scan_L2R_D2U,
  Scan_R2L_U2D,
  Scan_R2L_D2U,
  Scan_U2D_L2R,
  Scan_U2D_R2L,
  Scan_D2U_L2R,
  Scan_D2U_R2L,
} LCD_ScanDirection;

typedef enum
{
  Rotation0,
  Rotation90,
  Rotation180,
  Rotation270,
} LCD_Rotation;

typedef struct
{
  GPIO_TypeDef *RST_Port;
  uint32_t RST_Pin;
  GPIO_TypeDef *BLK_Port;
  uint32_t BLK_Pin;

  DMA_HandleTypeDef *hDMAx;

  LCD_Rotation Rotation;
  LCD_DisplayDirection DisplayDirection;
  LCD_ScanDirection ScanDirection;

  uint16_t ID;
  uint16_t Width;
  uint16_t Height;

  LCD_Font Font;
  uint8_t FontWidth;
  uint8_t FontHight;

  uint16_t SetXCMD;
  uint16_t SetYCMD;
  uint16_t GRAMCMD;

  uint16_t PenColor;
  uint16_t BackgroundColor;

  uint8_t PrintfBuffer[128];
} LCD_t;

void LCD_Init(LCD_t *Self);
void LCD_DisplayOn(LCD_t *Self);
void LCD_DisplayOff(LCD_t *Self);

void LCD_SetFont(LCD_t *Self, LCD_Font Font);
void LCD_SetRotation(LCD_t *Self, LCD_Rotation Rotation);
void LCD_SetDisplayDirection(LCD_t *Self, LCD_DisplayDirection DisplayDirection);
void LCD_SetScanDirection(LCD_t *Self, LCD_ScanDirection ScanDirection);
void LCD_SetPenColor(LCD_t *Self, uint16_t Color);
void LCD_SetBackgroundColor(LCD_t *Self, uint16_t Color);

void LCD_SetCursor(LCD_t *Self, uint16_t X, uint16_t Y);
void LCD_SetWindow(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height);

void LCD_Clear(LCD_t *Self);
void LCD_Fill(LCD_t *Self);
void LCD_ClearArea(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height);
void LCD_FillArea(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height);

uint16_t LCD_ReadPoint(LCD_t *Self, uint16_t X, uint16_t Y);
void LCD_DrawPoint(LCD_t *Self, uint16_t X, uint16_t Y);
void LCD_DrawPointWithColor(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Color);
void LCD_DrawHLine(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Width);
void LCD_DrawVLine(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Hight);
void LCD_DrawLine(LCD_t *Self, uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2);
void LCD_DrawRectangle(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Hight);
void LCD_DrawCircle(LCD_t *Self, uint16_t X, uint16_t Y, uint8_t Radius);

void LCD_ShowImage(LCD_t *Self, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, const uint8_t *Image);

void LCD_ShowChar(LCD_t *Self, uint16_t X, uint16_t Y, uint8_t Char);
void LCD_ShowString(LCD_t *Self, uint16_t X, uint16_t Y, char *String);
void LCD_Printf(LCD_t *Self, uint16_t X, uint16_t Y, char *Format, ...);

#endif
