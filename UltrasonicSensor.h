#ifndef __STM32L476G_ULTRASONICSENSOR_H
#define __STM32L476G_ULTRASONICSENSOR_H

#include "stm32l476xx.h"

void TIM4_CH1_Init(void);
void TIM3_CH2_Init(void);
void TIM4_IRQHandler(void);
void Display_Centimeters(void);
void Display_Inches(void);

#endif