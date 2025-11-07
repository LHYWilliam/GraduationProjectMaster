#ifndef __TOUCH_I2C_H__
#define __TOUCH_I2C_H__

#include <string.h>

#include "gpio.h"
#include "cmsis_os2.h"

#include "Touch.h"

uint8_t Touch_I2C_Init(Touch_t *Self);
void Touch_I2C_ReadBytes(Touch_t *Self, uint16_t RegAddress, uint8_t *Buffer, uint8_t Length);
void Touch_I2C_WriteBytes(Touch_t *Self, uint16_t RegAddress, uint8_t *Buffer, uint8_t Length);

#endif