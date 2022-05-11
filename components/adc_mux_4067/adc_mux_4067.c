/**
 ******************************************************************************
 * @file           : adc_mux_4067.c
 * @brief          : HEF4067 ADC MUX
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 Espressif.
 * All rights reserved.
 *
 * Vo Duc Toan / B1907202
 * Can Tho University.
 * March - 2022
 * Built with ESP-IDF Version: 4.4.
 * Target device: ESP32-WROOM.
 *
 ******************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "adc_mux_4067.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

static esp_adc_cal_characteristics_t *adc_chars;

void ADCMUX4067_Init(void)
{
#if (HC4067_ADC_UNIT == ADC_UINT_1)
    adc1_config_width(HC4067_ADC_WIDTH);
    adc1_config_channel_atten(HC4067_ADC_CHANNEL, HC4067_ADC_ATTEN);
#endif
#if (HC4067_ADC_UNIT == ADC_UINT_2)
    adc2_config_channel_atten(HC4067_ADC_CHANNEL, HC4067_ADC_ATTEN);
#endif
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(HC4067_ADC_UNIT, HC4067_ADC_ATTEN, HC4067_ADC_WIDTH, DEFAULT_VREF, adc_chars);
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    io_conf.pin_bit_mask = (1ULL << HC4067_A0_PIN) | (1ULL << HC4067_A1_PIN) | (1ULL << HC4067_A2_PIN) | (1ULL << HC4067_A3_PIN);
    gpio_config(&io_conf);
}

void ADCMUX4067_GetValue(uint16_t *val, uint16_t channel)
{
    uint32_t temp = 0;
    gpio_set_level(HC4067_A0_PIN, ((channel & BIT0) != 0));
    gpio_set_level(HC4067_A1_PIN, ((channel & BIT1) != 0));
    gpio_set_level(HC4067_A2_PIN, ((channel & BIT2) != 0));
    gpio_set_level(HC4067_A3_PIN, ((channel & BIT3) != 0));
    vTaskDelay(pdMS_TO_TICKS(50));
    for (int i = 0; i < HC4067_ADC_SAMPLES; i++)
    {
#if (HC4067_ADC_UNIT == ADC_UINT_1)
        temp += adc1_get_raw(HC4067_ADC_CHANNEL);
#endif
#if (HC4067_ADC_UNIT == ADC_UINT_2)
        int raw;
        adc2_get_raw(HC4067_ADC_CHANNEL, HC4067_ADC_WIDTH, &raw);
        temp += raw;
#endif
    }
    temp /= HC4067_ADC_SAMPLES;
    *val = temp;
}

void ADCMUX4067_GetVoltage(uint16_t *val, uint16_t channel)
{
    uint32_t temp = 0;
    gpio_set_level(HC4067_A0_PIN, ((channel & BIT0) != 0));
    gpio_set_level(HC4067_A1_PIN, ((channel & BIT1) != 0));
    gpio_set_level(HC4067_A2_PIN, ((channel & BIT2) != 0));
    gpio_set_level(HC4067_A3_PIN, ((channel & BIT3) != 0));
    vTaskDelay(pdMS_TO_TICKS(50));
    for (int i = 0; i < HC4067_ADC_SAMPLES; i++)
    {
#if (HC4067_ADC_UNIT == ADC_UINT_1)
        temp += adc1_get_raw(HC4067_ADC_CHANNEL);
#endif
#if (HC4067_ADC_UNIT == ADC_UINT_2)
        int raw;
        adc2_get_raw(HC4067_ADC_CHANNEL, HC4067_ADC_WIDTH, &raw);
        temp += raw;
#endif
    }
    temp /= HC4067_ADC_SAMPLES;
    *val = esp_adc_cal_raw_to_voltage(temp, adc_chars) + HC4067_OFFSET_ADC;
}
