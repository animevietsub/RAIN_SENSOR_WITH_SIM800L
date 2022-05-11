#ifndef __ADC_MUX_4067_H__
#define __ADC_MUX_4067_H__

#include "soc/adc_channel.h"

#define HC4067_A0_PIN 5
#define HC4067_A1_PIN 18
#define HC4067_A2_PIN 19
#define HC4067_A3_PIN 21

#define HC4067_LEVEL1_BIT 15
#define HC4067_LEVEL2_BIT 14
#define HC4067_LEVEL3_BIT 13
#define HC4067_LEVEL4_BIT 12
#define HC4067_LEVEL5_BIT 11

#define HC4067_TEMP1_BIT 0
#define HC4067_TEMP2_BIT 1
#define HC4067_TEMP3_BIT 2
#define HC4067_TEMP4_BIT 3
#define HC4067_TEMP5_BIT 4

#define HC4067_ADC_WIDTH ADC_WIDTH_BIT_12
#define HC4067_ADC_ATTEN ADC_ATTEN_DB_2_5
#define HC4067_ADC_UNIT ADC_UNIT_2
#define HC4067_ADC_SAMPLES 256
#define HC4067_ADC_CHANNEL ADC2_GPIO15_CHANNEL
#define HC4067_OFFSET_ADC -80

#define DEFAULT_VREF 1086 // Use adc2_vref_to_gpio() to obtain a better estimate

void ADCMUX4067_Init(void);
void ADCMUX4067_GetValue(uint16_t *val, uint16_t channel);
void ADCMUX4067_GetVoltage(uint16_t *val, uint16_t channel);

#endif
