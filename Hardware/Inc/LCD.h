#ifndef __LCD_H__
#define __LCD_H__

#include "stm32f4xx_hal.h"

#include "gpio.h"

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

#define LCD_LED PDout(12)
#define LCD_RST PDout(11)

#define LCD_RST_SET() HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET)
#define LCD_RST_RESET() HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET)
#define LCD_BLK_SET() HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_SET)
#define LCD_BLK_RESET() HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_RESET)

typedef enum
{
  L2R_U2D = 0,
  L2R_D2U = 1,
  R2L_U2D = 2,
  R2L_D2U = 3,
  U2D_L2R = 4,
  U2D_R2L = 5,
  D2U_L2R = 6,
  D2U_R2L = 7,
} LCDScanDirection_t;

#define Vertical_ScanDirection L2R_U2D
#define Horizontal_ScanDirection U2D_R2L

typedef enum
{
  LCD_Vertical = 0,
  LCD_Horizontal = 1,
} LCDDirection_t;

#define LCD_BASE ((uint32_t) (0x60000000 + (0x4000000 * (1 - 1))) | (((1 << 18) * 2) - 2))
#define LCD1 ((LCD_TypeDef *) LCD_BASE)

typedef struct
{
  volatile uint16_t LCD_REG;
  volatile uint16_t LCD_RAM;
} LCD_TypeDef;

typedef struct
{
  LCDDirection_t Direction;
  LCDScanDirection_t ScanDirection;

  DMA_HandleTypeDef *hDMAx;

  uint16_t PointColor;
  uint16_t BackColor;

  uint16_t ID;
  uint16_t Width;
  uint16_t Height;

  uint16_t SetXCMD;
  uint16_t SetYCMD;
  uint16_t GRAMCMD;

  uint8_t PrintfBuffer[128];
} LCD_t;

void LCD_Init(LCD_t *self);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);

void LCD_SetCursor(LCD_t *self, uint16_t x, uint16_t y);
void LCD_SetWindow(LCD_t *self, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void LCD_SetScanDirection(LCD_t *self);
void LCD_SetDisplayDirection(LCD_t *self);
void LCD_SetPointColor(LCD_t *self, uint16_t color);
void LCD_SetBackColor(LCD_t *self, uint16_t color);

void LCD_Clear(LCD_t *self, uint16_t Color);
void LCD_Fill(LCD_t *self, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color);

uint16_t LCD_ReadPoint(LCD_t *self, uint16_t x, uint16_t y);
void LCD_DrawPoint(LCD_t *self, uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawLine(LCD_t *self, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_DrawRectangle(LCD_t *self, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_DrawCircle(LCD_t *self, uint16_t x0, uint16_t y0, uint8_t r);

void LCD_ShowImage(LCD_t *self, uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *image);

void LCD_ShowChar(LCD_t *self, uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode);
void LCD_ShowString(LCD_t *self, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p);
void LCD_Printf(LCD_t *self, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *format, ...);

#endif
