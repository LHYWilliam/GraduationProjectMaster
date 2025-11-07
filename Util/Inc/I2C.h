#ifndef __I2C_H__
#define __I2C_H__

#include "stm32f4xx_hal.h"

#include "gpio.h"

typedef enum
{
  I2CAck,
  I2CNAck
} I2CAck_t;

typedef enum
{
  I2CWrite = 0x00,
  I2CRead = 0x01,
} I2CMode_t;

typedef struct
{
  GPIOxPiny_t SCL;
  GPIOxPiny_t SDA;
  uint32_t SCL_ODR;
  uint32_t SDA_IDR;
  uint32_t SDA_ODR;

  uint8_t Delay;
  uint16_t Timeout;
} I2C_t;

void I2C_Init(I2C_t *Self);
void I2C_Start(I2C_t *Self);
void I2C_Stop(I2C_t *Self);
I2CAck_t I2C_WaitAck(I2C_t *Self);
uint8_t I2C_ReadByte(I2C_t *Self, I2CAck_t Ack);
void I2C_WriteByte(I2C_t *Self, uint8_t Byte);

#endif
