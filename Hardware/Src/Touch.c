#include <string.h>

#include "cmsis_os2.h"

#include "LCD.h"
#include "Touch.h"

uint8_t GT1151_Init(Touch_t *Self);
void GT1151_ReadBytes(Touch_t *Self, uint16_t reg, uint8_t *buffer, uint8_t length);
uint8_t GT1151_WriteBytes(Touch_t *Self, uint16_t reg, uint8_t *buffer, uint8_t length);

void GT1151_IIC_Init(Touch_t *Self);
void GT1151_IIC_Start(Touch_t *Self);
void GT1151_IIC_Stop(Touch_t *Self);
void GT1151_IIC_SendByte(Touch_t *Self, uint8_t txd);
uint8_t GT1151_IIC_ReadByte(Touch_t *Self, unsigned char ack);
void GT1151_IIC_Ack(Touch_t *Self);
void GT1151_IIC_NAck(Touch_t *Self);
uint8_t GT1151_IIC_WaitAck(Touch_t *Self);

void Touch_Init(Touch_t *Self)
{
  GT1151_Init(Self);
}

uint8_t Touch_Scan(Touch_t *Self, LCD_t *LCD)
{
  static uint8_t t = 0;
  uint8_t mode = 0, resault = 0;

  t++;
  if ((t % 10) == 0 || t < 10)
  {
    GT1151_ReadBytes(Self, GT1151_GSTID_REG, &mode, 1);

    uint8_t temp;
    if (mode & 0X80 && ((mode & 0XF) < 6))
    {
      temp = 0;
      GT1151_WriteBytes(Self, GT1151_GSTID_REG, &temp, 1);
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
          GT1151_ReadBytes(Self, GT1151_TPX_TBL[i], Buffer, 4);
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

static void GT1151_Delay(void) { Time_Delayus(2); }

uint8_t GT1151_Init(Touch_t *Self)
{
  Self->SCL_ODR = GPIO_ODR(Self->SCL);
  Self->SDA_IDR = GPIO_IDR(Self->SDA);
  Self->SDA_ODR = GPIO_ODR(Self->SDA);
  Self->RST_ODR = GPIO_ODR(Self->RST);

  uint8_t Temp[5];

  GPIO_Write(Self->RST_ODR, 0);
  osDelay(10);
  GPIO_Write(Self->RST_ODR, 1);
  osDelay(10);

  osDelay(100);
  GT1151_ReadBytes(Self, GT1151_PID_REG, Temp, 4);
  Temp[4] = 0;

  if (strcmp((char *) Temp, "1158") == 0)
  {
    Temp[0] = 0X02;
    GT1151_WriteBytes(Self, GT1151_CTRL_REG, Temp, 1);
    GT1151_ReadBytes(Self, GT1151_CFGS_REG, Temp, 1);

    osDelay(10);
    Temp[0] = 0X00;
    GT1151_WriteBytes(Self, GT1151_CTRL_REG, Temp, 1);
    return 0;
  }
  return 1;
}

void GT1151_ReadBytes(Touch_t *Self, uint16_t reg, uint8_t *buffer, uint8_t length)
{
  GT1151_IIC_Start(Self);
  GT1151_IIC_SendByte(Self, GT1151_CMD_WR);
  GT1151_IIC_WaitAck(Self);
  GT1151_IIC_SendByte(Self, reg >> 8);
  GT1151_IIC_WaitAck(Self);
  GT1151_IIC_SendByte(Self, reg & 0XFF);
  GT1151_IIC_WaitAck(Self);
  GT1151_IIC_Start(Self);
  GT1151_IIC_SendByte(Self, GT1151_CMD_RD);
  GT1151_IIC_WaitAck(Self);

  for (uint8_t i = 0; i < length; i++)
  {
    buffer[i] = GT1151_IIC_ReadByte(Self, i == (length - 1) ? 0 : 1);
  }

  GT1151_IIC_Stop(Self);
}

uint8_t GT1151_WriteBytes(Touch_t *Self, uint16_t reg, uint8_t *buffer, uint8_t length)
{
  GT1151_IIC_Start(Self);
  GT1151_IIC_SendByte(Self, GT1151_CMD_WR);
  GT1151_IIC_WaitAck(Self);
  GT1151_IIC_SendByte(Self, reg >> 8);
  GT1151_IIC_WaitAck(Self);
  GT1151_IIC_SendByte(Self, reg & 0XFF);
  GT1151_IIC_WaitAck(Self);

  uint8_t ret = 0;
  for (uint8_t i = 0; i < length; i++)
  {
    GT1151_IIC_SendByte(Self, buffer[i]);
    ret = GT1151_IIC_WaitAck(Self);
    if (ret)
    {
      break;
    }
  }

  GT1151_IIC_Stop(Self);

  return ret;
}

void GT1151_IIC_Start(Touch_t *Self)
{
  GPIO_OutputMode(Self->SDA);
  GPIO_Write(Self->SDA_ODR, 1);
  GPIO_Write(Self->SCL_ODR, 1);
  Time_Delayus(30);
  GPIO_Write(Self->SDA_ODR, 0);
  GT1151_Delay();
  GPIO_Write(Self->SCL_ODR, 0);
}

void GT1151_IIC_Stop(Touch_t *Self)
{
  GPIO_OutputMode(Self->SDA);
  GPIO_Write(Self->SCL_ODR, 1);
  Time_Delayus(30);
  GPIO_Write(Self->SDA_ODR, 0);
  GT1151_Delay();
  GPIO_Write(Self->SDA_ODR, 1);
}

void GT1151_IIC_SendByte(Touch_t *Self, uint8_t txd)
{
  GPIO_OutputMode(Self->SDA);
  GPIO_Write(Self->SCL_ODR, 0);
  GT1151_Delay();

  for (uint8_t t = 0; t < 8; t++)
  {
    GT1151_IIC_SDA = (txd & 0x80) >> 7;
    txd <<= 1;
    GT1151_Delay();
    GPIO_Write(Self->SCL_ODR, 1);
    GT1151_Delay();
    GPIO_Write(Self->SCL_ODR, 0);
    GT1151_Delay();
  }
}

uint8_t GT1151_IIC_ReadByte(Touch_t *Self, unsigned char ack)
{
  GPIO_InputMode(Self->SDA);
  Time_Delayus(30);

  uint8_t receive = 0;
  for (uint8_t i = 0; i < 8; i++)
  {
    GPIO_Write(Self->SCL_ODR, 0);
    GT1151_Delay();
    GPIO_Write(Self->SCL_ODR, 1);
    receive <<= 1;
    GT1151_Delay();
    if (GPIO_ReadInput(Self->SDA_IDR))
      receive++;
    GT1151_Delay();
  }

  if (!ack)
  {
    GT1151_IIC_NAck(Self);
  } else
  {
    GT1151_IIC_Ack(Self);
  }

  return receive;
}

void GT1151_IIC_Ack(Touch_t *Self)
{
  GPIO_Write(Self->SCL_ODR, 0);
  GPIO_OutputMode(Self->SDA);
  GT1151_Delay();
  GPIO_Write(Self->SDA_ODR, 0);
  GT1151_Delay();
  GPIO_Write(Self->SCL_ODR, 1);
  GT1151_Delay();
  GPIO_Write(Self->SCL_ODR, 0);
}

void GT1151_IIC_NAck(Touch_t *Self)
{
  GPIO_Write(Self->SCL_ODR, 0);
  GPIO_OutputMode(Self->SDA);
  GT1151_Delay();
  GPIO_Write(Self->SDA_ODR, 1);
  GT1151_Delay();
  GPIO_Write(Self->SCL_ODR, 1);
  GT1151_Delay();
  GPIO_Write(Self->SCL_ODR, 0);
}

uint8_t GT1151_IIC_WaitAck(Touch_t *Self)
{
  GPIO_InputMode(Self->SDA);
  GPIO_Write(Self->SDA_ODR, 1);
  GPIO_Write(Self->SCL_ODR, 1);
  GT1151_Delay();

  uint8_t ucErrTime = 0;
  while (GPIO_ReadInput(Self->SDA_IDR))
  {
    ucErrTime++;
    if (ucErrTime > 250)
    {
      GT1151_IIC_Stop(Self);
      return 1;
    }
    GT1151_Delay();
  }
  GPIO_Write(Self->SCL_ODR, 0);

  return 0;
}
