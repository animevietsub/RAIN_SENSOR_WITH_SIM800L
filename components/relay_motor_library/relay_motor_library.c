/**
 ******************************************************************************
 * @file           : relay_motor_libary.c
 * @brief          : Relay & Motor control
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
#include "driver/gpio.h"

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

void RelayMotor_MotorOn(rm_control_t *rm_control)
{
    gpio_set_level(MOTOR_CONTROL_PIN, true);
    (*rm_control).MOTOR_EN = true;
}

void RelayMotor_MotorOff(rm_control_t *rm_control)
{
    gpio_set_level(MOTOR_CONTROL_PIN, false);
    (*rm_control).MOTOR_EN = false;
}

void RelayMotor_RelayOn(rm_control_t *rm_control)
{
    gpio_set_level(RELAY_CONTROL_PIN, true);
    (*rm_control).RELAY_EN = true;
}

void RelayMotor_RelayOff(rm_control_t *rm_control)
{
    gpio_set_level(RELAY_CONTROL_PIN, false);
    (*rm_control).RELAY_EN = false;
}

void RelayMotor_Update(rm_control_t *rm_control)
{
    gpio_set_level(RELAY_CONTROL_PIN, (*rm_control).RELAY_EN);
    gpio_set_level(MOTOR_CONTROL_PIN, (*rm_control).MOTOR_EN);
}