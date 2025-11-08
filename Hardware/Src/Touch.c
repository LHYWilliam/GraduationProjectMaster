#include "Touch.h"
#include "Touch_I2C.h"

void Touch_Init(Touch_t *Self)
{
  Touch_I2C_Init(Self);
}

uint8_t Touch_Scan(Touch_t *Self, LCD_t *LCD)
{
  uint8_t Mode = 0, Temp = 0;

  Touch_I2C_ReadBytes(Self, GT1151_GSTID_REG, &Mode, 1);

  if (Mode & 0X80 && ((Mode & 0XF) < 6))
  {
    Temp = 0;
    Touch_I2C_WriteBytes(Self, GT1151_GSTID_REG, &Temp, 1);
  }

  if ((Mode & 0XF) && ((Mode & 0XF) < 6))
  {
    Temp = 0XFF << (Mode & 0XF);
    Self->TouchFlag = (~Temp) | PRES_DOWN | CATH_PRES;

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
  }

  return Self->TouchFlag;
}

uint8_t Touch_ScanChannel(Touch_t *Self, LCD_t *LCD, uint8_t Channel)
{
  Touch_Scan(Self, LCD);
  return Self->TouchFlag & (1 << Channel);
}
