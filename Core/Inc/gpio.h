/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

#define BITBAND(addr, bitnum) \
  ((addr & 0xF0000000) + 0x2000000 + ((addr & 0xFFFFF) << 5) + (bitnum << 2))
#define MEM_ADDR(addr) *((volatile unsigned long *) (addr))
#define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))

#define GPIOA_ODR (GPIOA_BASE + 20)
#define GPIOB_ODR (GPIOB_BASE + 20)
#define GPIOC_ODR (GPIOC_BASE + 20)
#define GPIOD_ODR (GPIOD_BASE + 20)
#define GPIOE_ODR (GPIOE_BASE + 20)

#define GPIOA_IDR (GPIOA_BASE + 16)
#define GPIOB_IDR (GPIOB_BASE + 16)
#define GPIOC_IDR (GPIOC_BASE + 16)
#define GPIOD_IDR (GPIOD_BASE + 16)
#define GPIOE_IDR (GPIOE_BASE + 16)

#define PAout(n) BIT_ADDR(GPIOA_ODR, n)
#define PBout(n) BIT_ADDR(GPIOB_ODR, n)
#define PCout(n) BIT_ADDR(GPIOC_ODR, n)
#define PDout(n) BIT_ADDR(GPIOD_ODR, n)
#define PEout(n) BIT_ADDR(GPIOE_ODR, n)

#define PAin(n) BIT_ADDR(GPIOA_IDR, n)
#define PBin(n) BIT_ADDR(GPIOB_IDR, n)
#define PCin(n) BIT_ADDR(GPIOC_IDR, n)
#define PDin(n) BIT_ADDR(GPIOD_IDR, n)
#define PEin(n) BIT_ADDR(GPIOE_IDR, n)

#define PA0 "A0"
#define PA1 "A1"
#define PA2 "A2"
#define PA3 "A3"
#define PA4 "A4"
#define PA5 "A5"
#define PA6 "A6"
#define PA7 "A7"
#define PA8 "A8"
#define PA9 "A9"
#define PA10 "A10"
#define PA11 "A11"
#define PA12 "A12"
#define PA13 "A13"
#define PA14 "A14"
#define PA15 "A15"

#define PB0 "B0"
#define PB1 "B1"
#define PB2 "B2"
#define PB3 "B3"
#define PB4 "B4"
#define PB5 "B5"
#define PB6 "B6"
#define PB7 "B7"
#define PB8 "B8"
#define PB9 "B9"
#define PB10 "B10"
#define PB11 "B11"
#define PB12 "B12"
#define PB13 "B13"
#define PB14 "B14"
#define PB15 "B15"

#define PC0 "C0"
#define PC1 "C1"
#define PC2 "C2"
#define PC3 "C3"
#define PC4 "C4"
#define PC5 "C5"
#define PC6 "C6"
#define PC7 "C7"
#define PC8 "C8"
#define PC9 "C9"
#define PC10 "C10"
#define PC11 "C11"
#define PC12 "C12"
#define PC13 "C13"
#define PC14 "C14"
#define PC15 "C15"

#define PD0 "D0"
#define PD1 "D1"
#define PD2 "D2"
#define PD3 "D3"
#define PD4 "D4"
#define PD5 "D5"
#define PD6 "D6"
#define PD7 "D7"
#define PD8 "D8"
#define PD9 "D9"
#define PD10 "D10"
#define PD11 "D11"
#define PD12 "D12"
#define PD13 "D13"
#define PD14 "D14"
#define PD15 "D15"

#define PE0 "E0"
#define PE1 "E1"
#define PE2 "E2"
#define PE3 "E3"
#define PE4 "E4"
#define PE5 "E5"
#define PE6 "E6"
#define PE7 "E7"
#define PE8 "E8"
#define PE9 "E9"
#define PE10 "E10"
#define PE11 "E11"
#define PE12 "E12"
#define PE13 "E13"
#define PE14 "E14"
#define PE15 "E15"

#define GPIOx(x) ((GPIO_TypeDef *) (AHB1PERIPH_BASE + 0x0400UL * ((x)[0] - 'A')))

#define GPIO_Pin_Num(x) ((x)[2] ? (10 + (x)[2] - '0') : ((x)[1] - '0'))
#define GPIO_Pin_x(x) GPIO_PIN_0 << GPIO_Pin(x)

#define GPIO_Write(DR, Value) MEM_ADDR((DR)) = ((Value) ? 1 : 0)
#define GPIO_Toggle(DR) MEM_ADDR((DR)) = !MEM_ADDR((DR))
#define GPIO_ReadInput(DR) MEM_ADDR((DR))
#define GPIO_ReadOutput(DR) MEM_ADDR((DR))

#define GPIO_InputMode(GPIOxPiny)          \
  do                                       \
  {                                        \
    GPIO_TypeDef *Port = GPIOx(GPIOxPiny); \
    uint8_t Pin = GPIO_Pin_Num(GPIOxPiny); \
    Port->MODER &= ~(3UL << (2 * Pin));    \
    Port->MODER |= (0UL << (2 * Pin));     \
  } while (0)

#define GPIO_OutputMode(GPIOxPiny)         \
  do                                       \
  {                                        \
    GPIO_TypeDef *Port = GPIOx(GPIOxPiny); \
    uint8_t Pin = GPIO_Pin_Num(GPIOxPiny); \
    Port->MODER &= ~(3UL << (2 * Pin));    \
    Port->MODER |= (1UL << (2 * Pin));     \
  } while (0)

typedef char GPIOxPiny_t[4];

/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */

uint32_t GPIO_IDR(const GPIOxPiny_t Pin);
uint32_t GPIO_ODR(const GPIOxPiny_t Pin);

void Time_Delayus(uint32_t us);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

