#include "Touch.h"
#include "Touch_I2C.h"

uint8_t Touch_I2C_Init(Touch_t *Self)
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
  Touch_I2C_ReadBytes(Self, GT1151_PID_REG, Temp, 4);
  Temp[4] = 0;

  if (strcmp((char *) Temp, "1158") == 0)
  {
    Temp[0] = 0X02;
    Touch_I2C_WriteBytes(Self, GT1151_CTRL_REG, Temp, 1);
    Touch_I2C_ReadBytes(Self, GT1151_CFGS_REG, Temp, 1);

    osDelay(10);
    Temp[0] = 0X00;
    Touch_I2C_WriteBytes(Self, GT1151_CTRL_REG, Temp, 1);
    return 0;
  }
  return 1;
}

void Touch_I2C_Delay(void)
{
  Time_Delayus(2);
}

void Touch_I2C_Start(Touch_t *Self)
{
  GPIO_OutputMode(Self->SDA);

  GPIO_Write(Self->SDA_ODR, 1);
  GPIO_Write(Self->SCL_ODR, 1);

  GPIO_Write(Self->SDA_ODR, 0);
  GPIO_Write(Self->SCL_ODR, 0);
}

void Touch_I2C_Stop(Touch_t *Self)
{
  GPIO_OutputMode(Self->SDA);

  GPIO_Write(Self->SDA_ODR, 0);

  GPIO_Write(Self->SCL_ODR, 1);
  GPIO_Write(Self->SDA_ODR, 1);
}

void Touch_I2C_SendByte(Touch_t *Self, uint8_t Byte)
{
  GPIO_OutputMode(Self->SDA);

  GPIO_Write(Self->SCL_ODR, 0);
  for (uint8_t t = 0; t < 8; t++)
  {
    GPIO_Write(Self->SDA_ODR, (Byte & 0x80) >> 7);
    Byte <<= 1;

    GPIO_Write(Self->SCL_ODR, 1);
    Touch_I2C_Delay(); // TODO: Delay
    GPIO_Write(Self->SCL_ODR, 0);
  }
}

void Touch_I2C_Ack(Touch_t *Self)
{
  GPIO_OutputMode(Self->SDA);
  GPIO_Write(Self->SDA_ODR, 0);

  GPIO_Write(Self->SCL_ODR, 1);
  Touch_I2C_Delay(); // TODO: Delay
  GPIO_Write(Self->SCL_ODR, 0);
}

void Touch_I2C_NAck(Touch_t *Self)
{
  GPIO_OutputMode(Self->SDA);
  GPIO_Write(Self->SDA_ODR, 1);

  GPIO_Write(Self->SCL_ODR, 1);
  Touch_I2C_Delay();
  GPIO_Write(Self->SCL_ODR, 0);
}

uint8_t Touch_I2C_ReadByte(Touch_t *Self, unsigned char Ack)
{
  uint8_t Byte = 0;

  GPIO_InputMode(Self->SDA);
  for (uint8_t i = 0; i < 8; i++)
  {
    GPIO_Write(Self->SCL_ODR, 0);
    Touch_I2C_Delay();
    GPIO_Write(Self->SCL_ODR, 1);

    Byte <<= 1;
    if (GPIO_ReadInput(Self->SDA_IDR))
    {
      Byte++;
    }
  }
  GPIO_Write(Self->SCL_ODR, 0);

  if (!Ack)
  {
    Touch_I2C_NAck(Self);
  } else
  {
    Touch_I2C_Ack(Self);
  }

  return Byte;
}

uint8_t Touch_I2C_WaitAck(Touch_t *Self)
{
  GPIO_InputMode(Self->SDA);

  GPIO_Write(Self->SDA_ODR, 1);

  uint8_t ucErrTime = 0;
  while (GPIO_ReadInput(Self->SDA_IDR))
  {
    ucErrTime++;
    if (ucErrTime > 250)
    {
      Touch_I2C_Stop(Self);
      return 1;
    }
  }

  GPIO_Write(Self->SCL_ODR, 1);
  Touch_I2C_Delay();
  GPIO_Write(Self->SCL_ODR, 0);

  return 0;
}

void Touch_I2C_ReadBytes(Touch_t *Self, uint16_t RegAddress, uint8_t *Buffer, uint8_t Length)
{
  Touch_I2C_Start(Self);

  Touch_I2C_SendByte(Self, GT1151_CMD_WR);
  Touch_I2C_WaitAck(Self);

  Touch_I2C_SendByte(Self, RegAddress >> 8);
  Touch_I2C_WaitAck(Self);

  Touch_I2C_SendByte(Self, RegAddress & 0XFF);
  Touch_I2C_WaitAck(Self);

  Touch_I2C_Start(Self);

  Touch_I2C_SendByte(Self, GT1151_CMD_RD);
  Touch_I2C_WaitAck(Self);

  for (uint8_t i = 0; i < Length; i++)
  {
    Buffer[i] = Touch_I2C_ReadByte(Self, i == (Length - 1) ? 0 : 1);
  }

  Touch_I2C_Stop(Self);
}

void Touch_I2C_WriteBytes(Touch_t *Self, uint16_t RegAddress, uint8_t *Buffer, uint8_t Length)
{
  Touch_I2C_Start(Self);

  Touch_I2C_SendByte(Self, GT1151_CMD_WR);
  Touch_I2C_WaitAck(Self);

  Touch_I2C_SendByte(Self, RegAddress >> 8);
  Touch_I2C_WaitAck(Self);

  Touch_I2C_SendByte(Self, RegAddress & 0XFF);
  Touch_I2C_WaitAck(Self);

  for (uint8_t i = 0; i < Length; i++)
  {
    Touch_I2C_SendByte(Self, Buffer[i]);
    uint8_t Ack = Touch_I2C_WaitAck(Self);
    if (Ack)
    {
      break;
    }
  }

  Touch_I2C_Stop(Self);
}
