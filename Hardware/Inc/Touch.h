#ifndef TOUCH_H
#define TOUCH_H

#include "RTE_Components.h"
#include CMSIS_device_header

#include "LCD.h"
#include "sys.h"

#define GT1151_CMD_WR 0X28
#define GT1151_CMD_RD 0X29
#define GT1151_CTRL_REG 0X8040
#define GT1151_CFGS_REG 0X8050
#define GT1151_PID_REG 0X8140
#define GT1151_GSTID_REG 0X814E
#define GT1151_TP1_REG 0X8150
#define GT1151_TP2_REG 0X8158
#define GT1151_TP3_REG 0X8160
#define GT1151_TP4_REG 0X8168
#define GT1151_TP5_REG 0X8170

#define PRES_DOWN 0x80
#define CATH_PRES 0x40
#define MAX_TOUCH 5

static const uint16_t GT1151_TPX_TBL[5] = {
    GT1151_TP1_REG,
    GT1151_TP2_REG,
    GT1151_TP3_REG,
    GT1151_TP4_REG,
    GT1151_TP5_REG,
};

#define GT1151_SDA_IN()               \
  {                                   \
    GPIOB->MODER &= ~(3 << (2 * 12)); \
    GPIOB->MODER |= 0 << 2 * 12;      \
  }
#define GT1151_SDA_OUT()              \
  {                                   \
    GPIOB->MODER &= ~(3 << (2 * 12)); \
    GPIOB->MODER |= 1 << 2 * 12;      \
  }

#define PEN PBin(13)
#define DOUT PBin(11)
#define TDIN PBout(12)
#define TCLK PBout(15)
#define TCS PBout(14)

#define GT1151_IIC_SCL PBout(15)
#define GT1151_IIC_SDA PBout(12)
#define GT1151_READ_SDA PBin(12)
#define GT1151_RST PBout(14)
#define GT1151_INT PBin(13)

typedef struct Touch
{
  uint8_t Direction;

  uint8_t TouchFlag;
  uint16_t X[MAX_TOUCH];
  uint16_t Y[MAX_TOUCH];
} Touch_t;

void Touch_Init(Touch_t *self);
uint8_t Touch_Scan(Touch_t *self, LCD_t *lcd);
uint8_t Touch_ScanChannel(Touch_t *self, LCD_t *lcd, uint8_t channel);

uint8_t GT1151_Init(void);
void GT1151_ReadBytes(uint16_t reg, uint8_t *buffer, uint8_t length);
uint8_t GT1151_WriteBytes(uint16_t reg, uint8_t *buffer, uint8_t length);

void GT1151_IIC_Init(void);
void GT1151_IIC_Start(void);
void GT1151_IIC_Stop(void);
void GT1151_IIC_SendByte(uint8_t txd);
uint8_t GT1151_IIC_ReadByte(unsigned char ack);
void GT1151_IIC_Ack(void);
void GT1151_IIC_NAck(void);
uint8_t GT1151_IIC_WaitAck(void);

#endif
