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
#include "relay_motor_library.h"

void RelayMotor_Init(void)
{
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    io_conf.pin_bit_mask = (1ULL << RELAY_CONTROL_PIN) | (1ULL << MOTOR_CONTROL_PIN);
    gpio_config(&io_conf);
}

void RelayMotor_MotorOn()
{
   gpio_set_level(MOTOR_CONTROL_PIN, true);
}

void RelayMotor_MotorOff()
{
   gpio_set_level(MOTOR_CONTROL_PIN, false);
}

void RelayMotor_RelayOn()
{
   gpio_set_level(RELAY_CONTROL_PIN, true);
}

void RelayMotor_RelayOff()
{
   gpio_set_level(RELAY_CONTROL_PIN, false);
}