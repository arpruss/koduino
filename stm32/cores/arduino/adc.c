/**
 * @authors Avik De <avikde@gmail.com>

  This file is part of koduino <https://github.com/avikde/koduino>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include "adc.h"
#include "pins.h"

////////////////////////////////////////////////////////////////////////////////////


#if defined(SERIES_STM32F37x) || defined(SERIES_STM32F10x)
uint8_t ADC_SAMPLE_TIME = ADC_SampleTime_13Cycles5;
#elif defined(SERIES_STM32F4xx)
uint8_t ADC_SAMPLE_TIME = ADC_SampleTime_15Cycles;
#elif defined(SERIES_STM32F30x)
uint8_t ADC_SAMPLE_TIME = ADC_SampleTime_19Cycles5;
#endif
#define ADC_TIMEOUT 50

void analogReadSampleTime(uint8_t sampleTime) {
  ADC_SAMPLE_TIME = sampleTime;
}

void adcCommonInit() {

#if defined(SERIES_STM32F4xx)
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_DeInit();

  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);
#elif defined(SERIES_STM32F30x)
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_CommonStructInit(&ADC_CommonInitStructure);
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0x0;
  ADC_CommonInit(ADC1, &ADC_CommonInitStructure);
#if defined(STM32F303xC)
  ADC_CommonInit(ADC2, &ADC_CommonInitStructure);
#endif
#endif
}

void adcInit(ADC_TypeDef *ADCx) {
#if defined(SERIES_STM32F37x) || defined(SERIES_STM32F30x)
  // was causing problems if initing multiple ADCs
  // ADC_DeInit(ADCx);
#endif

  ADC_InitTypeDef  ADC_InitStructure;
  ADC_StructInit(&ADC_InitStructure);

  // Common
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;

  // Data align right
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;

  // Continuous conv
#if defined(SERIES_STM32F37x) || defined(SERIES_STM32F10x) || defined(SERIES_STM32F4xx)
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
#endif

  // Resolution
#if defined(SERIES_STM32F30x)
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
#endif

  // Configure for 1 channel
#if defined(SERIES_STM32F37x) || defined(SERIES_STM32F10x)
  ADC_InitStructure.ADC_NbrOfChannel = 1;
#elif defined(SERIES_STM32F30x)
  ADC_InitStructure.ADC_NbrOfRegChannel = 1;
#else
  ADC_InitStructure.ADC_NbrOfConversion = 1;
#endif

  // No external trigger
#if defined(SERIES_STM32F37x) || defined(SERIES_STM32F10x)
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
#elif defined(SERIES_STM32F30x)
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
  // Immaterial
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
#endif

  // Independent
// #if defined(SERIES_STM32F37x)
//   ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
// #endif

  // other options
#if defined(SERIES_STM32F30x)
  ADC_InitStructure.ADC_OverrunMode = DISABLE;
  ADC_InitStructure.ADC_AutoInjMode = DISABLE;
#endif

  ADC_Init(ADCx, &ADC_InitStructure);
  ADC_Cmd(ADCx, ENABLE);

#if defined(SERIES_STM32F37x)
  ADC_ResetCalibration(ADCx);
  while(ADC_GetResetCalibrationStatus(ADCx));
  ADC_StartCalibration(ADCx);
  while(ADC_GetCalibrationStatus(ADCx));
#endif
}


uint16_t analogRead(uint8_t pin) {
  // Check ioConfig and set?
  // pinMode(pin, INPUT_ANALOG);

  // which ADC? mostly use ADC1
  ADC_TypeDef *ADCx = ADC1;
  // special cases:
#if defined(STM32F303xC)
  // PC4 and PC5 on ADC2
  if ((PIN_MAP[pin].port==GPIOC && PIN_MAP[pin].pin==4) || (PIN_MAP[pin].port==GPIOC && PIN_MAP[pin].pin==5))
    ADCx = ADC2;
#endif

  ADC_RegularChannelConfig(ADCx, PIN_MAP[pin].adcChannel, 1, ADC_SAMPLE_TIME);
  // Start the conversion
#if defined(SERIES_STM32F10x)
  ADC_SoftwareStartConvCmd(ADCx, ENABLE);
#elif defined(SERIES_STM32F30x)
  ADC_StartConversion(ADCx);
#else
  ADC_SoftwareStartConv(ADCx);
#endif

  // Wait until conversion completion
  while(ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC) == RESET);
  // Get the conversion value
  return ADC_GetConversionValue(ADCx);
}



// #if defined(SERIES_STM32F4xx)
// #define MAX_ADCS 3
// uint16_t syncReadBuffer[MAX_ADCS];

// const uint16_t *analogSyncRead(uint8_t pin1, uint8_t pin2, uint8_t pin3) {
//   ADC_RegularChannelConfig(ADC1, PIN_MAP[pin1].adcChannel, 1, ADC_SAMPLE_TIME);
//   ADC_RegularChannelConfig(ADC2, PIN_MAP[pin2].adcChannel, 1, ADC_SAMPLE_TIME);
//   ADC_RegularChannelConfig(ADC3, PIN_MAP[pin3].adcChannel, 1, ADC_SAMPLE_TIME);

//   ADC_SoftwareStartConv(ADC1);
//   ADC_SoftwareStartConv(ADC2);
//   ADC_SoftwareStartConv(ADC3);

//   while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
//   while(ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET);
//   while(ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) == RESET);
//   // Get the conversion value
//   syncReadBuffer[0] = ADC_GetConversionValue(ADC1);
//   syncReadBuffer[1] = ADC_GetConversionValue(ADC2);
//   syncReadBuffer[2] = ADC_GetConversionValue(ADC3);

//   return (const uint16_t *)&syncReadBuffer;
// }

// const uint16_t *analogSyncRead2(uint8_t pin1, uint8_t pin2) {
//   static int timeout;

//   ADC_RegularChannelConfig(ADC1, PIN_MAP[pin1].adcChannel, 1, ADC_SAMPLE_TIME);
//   ADC_RegularChannelConfig(ADC2, PIN_MAP[pin2].adcChannel, 1, ADC_SAMPLE_TIME);

//   ADC_SoftwareStartConv(ADC1);
//   ADC_SoftwareStartConv(ADC2);

//   // Need a timeout
//   timeout = ADC_TIMEOUT;
//   while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET && ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET)
//     // return previous values
//     if ((timeout--) == 0) return (const uint16_t *)&syncReadBuffer;

//   // Didn't time out; get the conversion value
//   syncReadBuffer[0] = ADC_GetConversionValue(ADC1);
//   syncReadBuffer[1] = ADC_GetConversionValue(ADC2);

//   return (const uint16_t *)&syncReadBuffer;
// }
// #endif
