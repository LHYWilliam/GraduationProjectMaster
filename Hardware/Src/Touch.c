#include "Touch.h"
#include "Touch_I2C.h"

void Touch_Init(Touch_t *Self)
{
  Touch_I2C_Init(Self);
}

uint8_t Touch_Scan(Touch_t *Self, LCD_t *LCD)
{
  static uint8_t t = 0;
  uint8_t mode = 0, resault = 0;

  t++;
  if ((t % 10) == 0 || t < 10)
  {
    Touch_I2C_ReadBytes(Self, GT1151_GSTID_REG, &mode, 1);

    uint8_t temp;
    if (mode & 0X80 && ((mode & 0XF) < 6))
    {
      temp = 0;
      Touch_I2C_WriteBytes(Self, GT1151_GSTID_REG, &temp, 1);
    }

    if ((mode & 0XF) && ((mode & 0XF) < 6))
    {
      temp = 0XFF << (mode & 0XF);

      uint8_t tempSTA = Self->TouchFlag;
      Self->TouchFlag = (~temp) | PRES_DOWN | CATH_PRES;
      Self->X[4] = Self->X[0];
      Self->Y[4] = Self->Y[0];

      for (uint8_t i = 0; i < 5; i++)
      {
        if (Self->TouchFlag & (1 << i))
        {
          uint8_t Buffer[4];
          Touch_I2C_ReadBytes(Self, GT1151_TPX_TBL[i], Buffer, 4);
          switch (Self->Rotation)
          {
          case Rotation0:
            Self->X[i] = ((uint16_t) Buffer[1] << 8) + Buffer[0];
            Self->Y[i] = ((uint16_t) Buffer[3] << 8) + Buffer[2];
            break;

          case Rotation90:
            Self->X[i] = ((uint16_t) Buffer[3] << 8) + Buffer[2];
            Self->Y[i] = LCD->Height - (((uint16_t) Buffer[1] << 8) + Buffer[0]);
            break;

          case Rotation180:
            Self->X[i] = LCD->Width - (((uint16_t) Buffer[1] << 8) + Buffer[0]);
            Self->Y[i] = LCD->Height - (((uint16_t) Buffer[3] << 8) + Buffer[2]);
            break;

          case Rotation270:
            Self->X[i] = LCD->Width - (((uint16_t) Buffer[3] << 8) + Buffer[2]);
            Self->Y[i] = ((uint16_t) Buffer[1] << 8) + Buffer[0];
            break;
          }
        }
      }

      resault = 1;

      if (Self->X[0] > LCD->Width || Self->Y[0] > LCD->Height)
      {
        if ((mode & 0XF) > 1)
        {
          Self->X[0] = Self->X[1];
          Self->Y[0] = Self->Y[1];
          t = 0;
        } else
        {
          Self->X[0] = Self->X[4];
          Self->Y[0] = Self->Y[4];
          mode = 0X80;
          Self->TouchFlag = tempSTA;
        }
      } else
      {
        t = 0;
      }
    }
  }

  if ((mode & 0X8F) == 0X80)
  {
    if (Self->TouchFlag & PRES_DOWN)
    {
      Self->TouchFlag &= ~(1 << 7);
    } else
    {
      Self->X[0] = 0xffff;
      Self->Y[0] = 0xffff;
      Self->TouchFlag &= 0XE0;
    }
  }

  if (t > 240)
  {
    t = 10;
  }

  return resault;
}

uint8_t Touch_ScanChannel(Touch_t *Self, LCD_t *LCD, uint8_t channel)
{
  Touch_Scan(Self, LCD);
  return Self->TouchFlag & (1 << channel);
}
