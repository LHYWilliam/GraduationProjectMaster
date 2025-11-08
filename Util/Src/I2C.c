#include "I2C.h"

void I2C_Init(I2C_t *Self)
{
  Self->SCL_ODR = GPIO_ODR(Self->SCL);
  Self->SDA_IDR = GPIO_IDR(Self->SDA);
  Self->SDA_ODR = GPIO_ODR(Self->SDA);
}

void I2C_Delay(I2C_t *Self)
{
  for (uint8_t i = 0; i < Self->Delay; i++)
  {
    __NOP();
  }
}

void I2C_Start(I2C_t *Self)
{
  GPIO_OutputMode(Self->SDA);

  GPIO_Write(Self->SDA_ODR, 1);
  GPIO_Write(Self->SCL_ODR, 1);
  I2C_Delay(Self);

  GPIO_Write(Self->SDA_ODR, 0);
  I2C_Delay(Self);
  GPIO_Write(Self->SCL_ODR, 0);
  I2C_Delay(Self);
}

void I2C_Stop(I2C_t *Self)
{
  GPIO_OutputMode(Self->SDA);

  GPIO_Write(Self->SCL_ODR, 0);
  GPIO_Write(Self->SDA_ODR, 0);
  I2C_Delay(Self);

  GPIO_Write(Self->SCL_ODR, 1);
  I2C_Delay(Self);
  GPIO_Write(Self->SDA_ODR, 1);
  I2C_Delay(Self);
}

void I2C_Tick(I2C_t *Self)
{
  GPIO_Write(Self->SCL_ODR, 1);
  I2C_Delay(Self);
  GPIO_Write(Self->SCL_ODR, 0);
}

void I2C_Ack(I2C_t *Self, I2CAck_t Ack)
{
  GPIO_Write(Self->SCL_ODR, 0);
  I2C_Delay(Self);

  GPIO_OutputMode(Self->SDA);

  GPIO_Write(Self->SDA_ODR, Ack);

  I2C_Tick(Self);
}

I2CAck_t I2C_WaitAck(I2C_t *Self)
{
  GPIO_InputMode(Self->SDA);

  uint16_t Timeout = Self->Timeout;
  while (GPIO_ReadInput(Self->SDA_IDR) != I2CAck && Timeout)
  {
    Timeout--;
  }

  if (Timeout == 0)
  {
    I2C_Stop(Self);
    return I2CNAck;
  } else
  {
    I2C_Tick(Self);
    return I2CAck;
  }
}

uint8_t I2C_ReadByte(I2C_t *Self, I2CAck_t Ack)
{
  uint8_t Byte = 0;
  GPIO_InputMode(Self->SDA);

  for (uint8_t i = 0; i < 8; i++)
  {
    GPIO_Write(Self->SCL_ODR, 0);
    I2C_Delay(Self);
    GPIO_Write(Self->SCL_ODR, 1);

    Byte = Byte << 1 | GPIO_ReadInput(Self->SDA_IDR);
  }

  I2C_Ack(Self, Ack);

  return Byte;
}

void I2C_WriteByte(I2C_t *Self, uint8_t Byte)
{
  GPIO_OutputMode(Self->SDA);

  for (uint8_t i = 0; i < 8; i++)
  {
    GPIO_Write(Self->SDA_ODR, Byte & (0x80 >> i));
    I2C_Tick(Self);
  }
}

#define I2C_WriteByteWithAck(Self, Byte) \
  do                                     \
  {                                      \
    I2C_WriteByte(Self, Byte);           \
    I2CAck_t Ack = I2C_WaitAck(Self);    \
    if (Ack == I2CNAck)                  \
    {                                    \
      I2C_Stop(Self);                    \
      GPIO_InputMode(Self->SDA);         \
      return Ack;                        \
    }                                    \
  } while (0)

I2CAck_t I2C_NowAddrRead(I2C_t *Self, uint8_t DevAddr, uint8_t *Bytes, uint8_t Length)
{
  I2C_Start(Self);

  I2C_WriteByteWithAck(Self, DevAddr | I2CRead);

  for (uint8_t i = 0; i < Length; i++)
  {
    Bytes[i] = I2C_ReadByte(Self, i == Length - 1 ? I2CNAck : I2CAck);
  }

  I2C_Stop(Self);

  return I2CAck;
}

I2CAck_t I2C_NowAddrWrite(I2C_t *Self, uint8_t DevAddr, const uint8_t *Bytes, uint8_t Length)
{
  I2C_Start(Self);

  I2C_WriteByteWithAck(Self, DevAddr | I2CWrite);

  for (uint8_t i = 0; i < Length; ++i)
  {
    I2C_WriteByteWithAck(Self, Bytes[i]);
  }

  I2C_Stop(Self);

  return I2CAck;
}

I2CAck_t I2C_SignedAddrRead(I2C_t *Self, uint8_t DevAddr, uint8_t MemAddr, uint8_t *Bytes, uint8_t Length)
{
  I2C_Start(Self);

  I2C_WriteByteWithAck(Self, DevAddr | I2CWrite);
  I2C_WriteByteWithAck(Self, MemAddr);

  I2C_Start(Self);

  I2C_WriteByteWithAck(Self, DevAddr | I2CRead);

  for (uint8_t i = 0; i < Length; i++)
  {
    Bytes[i] = I2C_ReadByte(Self, (i == Length - 1) ? I2CNAck : I2CAck);
  }

  I2C_Stop(Self);

  return I2CAck;
}

I2CAck_t I2C_SignedAddrWrite(I2C_t *Self, uint8_t DevAddr, uint8_t MemAddr, const uint8_t *Bytes, uint8_t Length)
{
  I2C_Start(Self);

  I2C_WriteByteWithAck(Self, DevAddr | I2CWrite);
  I2C_WriteByteWithAck(Self, MemAddr);

  for (uint8_t i = 0; i < Length; i++)
  {
    I2C_WriteByteWithAck(Self, Bytes[i]);
  }

  I2C_Stop(Self);

  return I2CAck;
}
