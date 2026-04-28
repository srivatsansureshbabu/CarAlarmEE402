#ifndef __STM32L476G_DAC_H
#define __STM32L476G_DAC_H

#include "stm32l476xx.h"

void InitDAC(void);
void playNote(uint16_t counts, uint16_t ms);
void TIM6_Init(uint16_t arr);

#endif
