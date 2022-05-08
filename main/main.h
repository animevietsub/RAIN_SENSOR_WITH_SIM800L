/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : RAIN_SENSOR_WITH_SIM800L
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
#ifndef __MAIN_H__
#define __MAIN_H__

typedef enum
{
    WEATHER_SUNNY,
    WEATHER_RAINY,
    WEATHER_SUNSHOWER,
} weather_status_t;

typedef enum
{
    DUMMY = -1,
    MENU_IDLE = 0,
    MENU_MANUAL_AUTO,
    MENU_LST,
    MENU_RST,
} menu_list_t;

typedef enum
{
    ACTION_DRYING = 0,
    ACTION_STORING,
} my_action_t;

#endif
