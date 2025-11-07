#include "I2C.h"
#include "Touch.h"
#include "Touch_I2C.h"

uint8_t Touch_I2C_Init(Touch_t *Self)
{
  I2C_Init(&Self->I2C);

  Self->RST_ODR = GPIO_ODR(Self->RST);

  GPIO_Write(Self->RST_ODR, 0);
  osDelay(10);
  GPIO_Write(Self->RST_ODR, 1);
  osDelay(10);

  uint8_t Temp[5];
  Touch_I2C_ReadBytes(Self, GT1151_PID_REG, Temp, 4);
  Temp[4] = 0;

  if (strcmp((char *) Temp, "1158") == 0)
  {
    Temp[0] = 0X02;
    Touch_I2C_WriteBytes(Self, GT1151_CTRL_REG, Temp, 1);
    Touch_I2C_ReadBytes(Self, GT1151_CFGS_REG, Temp, 1);

    Temp[0] = 0X00;
    Touch_I2C_WriteBytes(Self, GT1151_CTRL_REG, Temp, 1);
    return 0;
  }
  return 1;
}

void Touch_I2C_ReadBytes(Touch_t *Self, uint16_t RegAddr, uint8_t *Buffer, uint8_t Length)
{
  I2C_Start(&Self->I2C);

  I2C_WriteByte(&Self->I2C, GT1151_CMD_WR);
  I2C_WaitAck(&Self->I2C);

  I2C_WriteByte(&Self->I2C, RegAddr >> 8);
  I2C_WaitAck(&Self->I2C);

  I2C_WriteByte(&Self->I2C, RegAddr & 0XFF);
  I2C_WaitAck(&Self->I2C);

  I2C_Start(&Self->I2C);

  I2C_WriteByte(&Self->I2C, GT1151_CMD_RD);
  I2C_WaitAck(&Self->I2C);

  for (uint8_t i = 0; i < Length; i++)
  {
    Buffer[i] = I2C_ReadByte(&Self->I2C, i == (Length - 1) ? 1 : 0);
  }

  I2C_Stop(&Self->I2C);
}

void Touch_I2C_WriteBytes(Touch_t *Self, uint16_t RegAddr, uint8_t *Buffer, uint8_t Length)
{
  I2C_Start(&Self->I2C);

  I2C_WriteByte(&Self->I2C, GT1151_CMD_WR);
  I2C_WaitAck(&Self->I2C);

  I2C_WriteByte(&Self->I2C, RegAddr >> 8);
  I2C_WaitAck(&Self->I2C);

  I2C_WriteByte(&Self->I2C, RegAddr & 0XFF);
  I2C_WaitAck(&Self->I2C);

  for (uint8_t i = 0; i < Length; i++)
  {
    I2C_WriteByte(&Self->I2C, Buffer[i]);
    uint8_t Ack = I2C_WaitAck(&Self->I2C);
    if (Ack)
    {
      break;
    }
  }

  I2C_Stop(&Self->I2C);
}
