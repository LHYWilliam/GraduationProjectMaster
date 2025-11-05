#include <string.h>

#include "Delay.h"
#include "LCD.h"
#include "Touch.h"

void Touch_Init(Touch_t *self) { GT1151_Init(); }

uint8_t Touch_Scan(Touch_t *self, LCD_t *lcd)
{
  static uint8_t t = 0;
  uint8_t mode = 0, resault = 0;

  t++;
  if ((t % 10) == 0 || t < 10)
  {
    GT1151_ReadBytes(GT1151_GSTID_REG, &mode, 1);

    uint8_t temp;
    if (mode & 0X80 && ((mode & 0XF) < 6))
    {
      temp = 0;
      GT1151_WriteBytes(GT1151_GSTID_REG, &temp, 1);
    }

    if ((mode & 0XF) && ((mode & 0XF) < 6))
    {
      temp = 0XFF << (mode & 0XF);

      uint8_t tempSTA = self->TouchFlag;
      self->TouchFlag = (~temp) | PRES_DOWN | CATH_PRES;
      self->X[4] = self->X[0];
      self->Y[4] = self->Y[0];

      for (uint8_t i = 0; i < 5; i++)
      {
        if (self->TouchFlag & (1 << i))
        {
          uint8_t buffer[4];
          GT1151_ReadBytes(GT1151_TPX_TBL[i], buffer, 4);

          if (self->Direction == LCD_Vertical)
          {
            self->X[i] = ((uint16_t) buffer[1] << 8) + buffer[0];
            self->Y[i] = ((uint16_t) buffer[3] << 8) + buffer[2];
          } else if (self->Direction == LCD_Horizontal)
          {
            self->X[i] = (((uint16_t) buffer[3] << 8) + buffer[2]);
            self->Y[i] = lcd->Height -
                         (((uint16_t) buffer[1] << 8) + buffer[0]);
          }
        }
      }

      resault = 1;

      if (self->X[0] > lcd->Width || self->Y[0] > lcd->Height)
      {
        if ((mode & 0XF) > 1)
        {
          self->X[0] = self->X[1];
          self->Y[0] = self->Y[1];
          t = 0;
        } else
        {
          self->X[0] = self->X[4];
          self->Y[0] = self->Y[4];
          mode = 0X80;
          self->TouchFlag = tempSTA;
        }
      } else
      {
        t = 0;
      }
    }
  }

  if ((mode & 0X8F) == 0X80)
  {
    if (self->TouchFlag & PRES_DOWN)
    {
      self->TouchFlag &= ~(1 << 7);
    } else
    {
      self->X[0] = 0xffff;
      self->Y[0] = 0xffff;
      self->TouchFlag &= 0XE0;
    }
  }

  if (t > 240)
  {
    t = 10;
  }

  return resault;
}

uint8_t Touch_ScanChannel(Touch_t *self, LCD_t *lcd, uint8_t channel)
{
  Touch_Scan(self, lcd);
  return self->TouchFlag & (1 << channel);
}

static void GT1151_Delay(void) { Delay_us(2); }

uint8_t GT1151_Init(void)
{
  uint8_t temp[5];
  GPIO_InitTypeDef GPIO_Initure;
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_Initure.Pin = GPIO_PIN_13;
  GPIO_Initure.Mode = GPIO_MODE_INPUT;
  GPIO_Initure.Pull = GPIO_PULLUP;
  GPIO_Initure.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_Initure);

  GPIO_Initure.Pin = GPIO_PIN_14;
  GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOB, &GPIO_Initure);

  GT1151_IIC_Init();
  GT1151_RST = 0;
  Delay_ms(10);
  GT1151_RST = 1;
  Delay_ms(10);

  GPIO_Initure.Pin = GPIO_PIN_1;
  GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_Initure.Pull = GPIO_NOPULL;
  GPIO_Initure.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_Initure);

  Delay_ms(100);
  GT1151_ReadBytes(GT1151_PID_REG, temp, 4);
  temp[4] = 0;

  if (strcmp((char *) temp, "1158") == 0)
  {
    temp[0] = 0X02;
    GT1151_WriteBytes(GT1151_CTRL_REG, temp, 1);
    GT1151_ReadBytes(GT1151_CFGS_REG, temp, 1);

    Delay_ms(10);
    temp[0] = 0X00;
    GT1151_WriteBytes(GT1151_CTRL_REG, temp, 1);
    return 0;
  }
  return 1;
}

void GT1151_ReadBytes(uint16_t reg, uint8_t *buffer, uint8_t length)
{
  GT1151_IIC_Start();
  GT1151_IIC_SendByte(GT1151_CMD_WR);
  GT1151_IIC_WaitAck();
  GT1151_IIC_SendByte(reg >> 8);
  GT1151_IIC_WaitAck();
  GT1151_IIC_SendByte(reg & 0XFF);
  GT1151_IIC_WaitAck();
  GT1151_IIC_Start();
  GT1151_IIC_SendByte(GT1151_CMD_RD);
  GT1151_IIC_WaitAck();

  for (uint8_t i = 0; i < length; i++)
  {
    buffer[i] = GT1151_IIC_ReadByte(i == (length - 1) ? 0 : 1);
  }

  GT1151_IIC_Stop();
}

uint8_t GT1151_WriteBytes(uint16_t reg, uint8_t *buffer, uint8_t length)
{
  GT1151_IIC_Start();
  GT1151_IIC_SendByte(GT1151_CMD_WR);
  GT1151_IIC_WaitAck();
  GT1151_IIC_SendByte(reg >> 8);
  GT1151_IIC_WaitAck();
  GT1151_IIC_SendByte(reg & 0XFF);
  GT1151_IIC_WaitAck();

  uint8_t ret = 0;
  for (uint8_t i = 0; i < length; i++)
  {
    GT1151_IIC_SendByte(buffer[i]);
    ret = GT1151_IIC_WaitAck();
    if (ret)
    {
      break;
    }
  }

  GT1151_IIC_Stop();

  return ret;
}

void GT1151_IIC_Init(void)
{
  GPIO_InitTypeDef GPIO_Initure;

  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_Initure.Pin = GPIO_PIN_15;
  GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_Initure.Pull = GPIO_PULLUP;
  GPIO_Initure.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_Initure);

  GPIO_Initure.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOB, &GPIO_Initure);
}

void GT1151_IIC_Start(void)
{
  GT1151_SDA_OUT();
  GT1151_IIC_SDA = 1;
  GT1151_IIC_SCL = 1;
  Delay_us(30);
  GT1151_IIC_SDA = 0;
  GT1151_Delay();
  GT1151_IIC_SCL = 0;
}

void GT1151_IIC_Stop(void)
{
  GT1151_SDA_OUT();
  GT1151_IIC_SCL = 1;
  Delay_us(30);
  GT1151_IIC_SDA = 0;
  GT1151_Delay();
  GT1151_IIC_SDA = 1;
}

void GT1151_IIC_SendByte(uint8_t txd)
{
  GT1151_SDA_OUT();
  GT1151_IIC_SCL = 0;
  GT1151_Delay();

  for (uint8_t t = 0; t < 8; t++)
  {
    GT1151_IIC_SDA = (txd & 0x80) >> 7;
    txd <<= 1;
    GT1151_Delay();
    GT1151_IIC_SCL = 1;
    GT1151_Delay();
    GT1151_IIC_SCL = 0;
    GT1151_Delay();
  }
}

uint8_t GT1151_IIC_ReadByte(unsigned char ack)
{
  GT1151_SDA_IN();
  Delay_us(30);

  uint8_t receive = 0;
  for (uint8_t i = 0; i < 8; i++)
  {
    GT1151_IIC_SCL = 0;
    GT1151_Delay();
    GT1151_IIC_SCL = 1;
    receive <<= 1;
    GT1151_Delay();
    if (GT1151_READ_SDA)
      receive++;
    GT1151_Delay();
  }

  if (!ack)
  {
    GT1151_IIC_NAck();
  } else
  {
    GT1151_IIC_Ack();
  }

  return receive;
}

void GT1151_IIC_Ack(void)
{
  GT1151_IIC_SCL = 0;
  GT1151_SDA_OUT();
  GT1151_Delay();
  GT1151_IIC_SDA = 0;
  GT1151_Delay();
  GT1151_IIC_SCL = 1;
  GT1151_Delay();
  GT1151_IIC_SCL = 0;
}

void GT1151_IIC_NAck(void)
{
  GT1151_IIC_SCL = 0;
  GT1151_SDA_OUT();
  GT1151_Delay();
  GT1151_IIC_SDA = 1;
  GT1151_Delay();
  GT1151_IIC_SCL = 1;
  GT1151_Delay();
  GT1151_IIC_SCL = 0;
}

uint8_t GT1151_IIC_WaitAck(void)
{
  GT1151_SDA_IN();
  GT1151_IIC_SDA = 1;
  GT1151_IIC_SCL = 1;
  GT1151_Delay();

  uint8_t ucErrTime = 0;
  while (GT1151_READ_SDA)
  {
    ucErrTime++;
    if (ucErrTime > 250)
    {
      GT1151_IIC_Stop();
      return 1;
    }
    GT1151_Delay();
  }
  GT1151_IIC_SCL = 0;

  return 0;
}
