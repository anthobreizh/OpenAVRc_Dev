/*
**************************************************************************
*                                                                        *
*              This file is part of the OpenAVRc project.                *
*                                                                        *
*                         Based on code named                            *
*             OpenTx - https://github.com/opentx/opentx                  *
*                                                                        *
*                Only AVR code here for lisibility ;-)                   *
*                                                                        *
*   OpenAVRc is free software: you can redistribute it and/or modify     *
*   it under the terms of the GNU General Public License as published by *
*   the Free Software Foundation, either version 2 of the License, or    *
*   (at your option) any later version.                                  *
*                                                                        *
*   OpenAVRc is distributed in the hope that it will be useful,          *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of       *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
*   GNU General Public License for more details.                         *
*                                                                        *
*       License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html          *
*                                                                        *
**************************************************************************
*/


#include "OpenAVRc.h"


#define ADC_VREF_TYPE (1 << REFS0) // AVCC with external capacitor at AREF pin

void adcInit()
{
  ADMUX = ADC_VREF_TYPE;
  ADCSRA = 0x85; // ADC enabled, pre-scaler division=32 (no interrupt, no auto-triggering)
#if defined(CPUM2560)
  ADCSRB = (1 << MUX5);
#endif
}

void adcPrepareBandgap()
{
  // #if structure identical to the one in getADC_bandgap()
#if   defined(PCBMEGA2560)
#else
  ADMUX = 0x1E|ADC_VREF_TYPE; // Switch MUX to internal reference
#endif
}

void getADC()
{
  for (uint8_t adc_input=0; adc_input<8; adc_input++) {
    uint16_t temp_ana;
    ADMUX = adc_input|ADC_VREF_TYPE;
    ADCSRA |= 1 << ADSC; // Start the AD conversion
    while SIMU_UNLOCK_MACRO(ADCSRA & (1 << ADSC)); // Wait for the AD conversion to complete
    temp_ana = ADC;
    ADCSRA |= 1 << ADSC; // Start the second AD conversion
    while SIMU_UNLOCK_MACRO(ADCSRA & (1 << ADSC)); // Wait for the AD conversion to complete
    temp_ana += ADC;
#if !defined(SIMU)
    s_anaFilt[adc_input] = temp_ana;
#endif
  }

#if defined(TELEMETRY_MOD_14051) || defined(TELEMETRY_MOD_14051_SWAPPED)
  processMultiplexAna();
#endif
}

void getADC_bandgap()
{
#if   defined(PCBMEGA2560)
  BandGap = 2000;
#else
  /*
    MCUCR|=0x28;  // enable Sleep (bit5) enable ADC Noise Reduction (bit2)
    asm volatile(" sleep        \n\t");  // if _SLEEP() is not defined use this
    // ADCSRA|=0x40;
    while ((ADCSRA & 0x10)==0);
    ADCSRA|=0x10; // take sample  clear flag?
    BandGap=ADC;
    MCUCR&=0x08;  // disable sleep
    */

  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  BandGap = ADC;
#endif
}
