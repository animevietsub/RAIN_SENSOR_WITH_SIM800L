/**
 ******************************************************************************
 * @file           : animation.h
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
#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"

#include "lcd_com.h"
#include "lcd_lib.h"
#include "fontx.h"
#include "decode_jpeg.h"

#include "ili9225.h"

void setLSV(TFT_t *dev, FontxFile *fx, uint16_t value);
void setRSV(TFT_t *dev, FontxFile *fx, uint16_t value);
void setLST(TFT_t *dev, FontxFile *fx, uint16_t value);
void setRST(TFT_t *dev, FontxFile *fx, uint16_t value);
void drawImage(TFT_t *dev, char *file, uint16_t offsetX, uint16_t offsetY, uint16_t width, uint16_t height);
void drawRainy(TFT_t *dev, uint16_t offsetX, uint16_t offsetY);
void drawSunny(TFT_t *dev, uint16_t offsetX, uint16_t offsetY);
void drawSunshower(TFT_t *dev, uint16_t offsetX, uint16_t offsetY);
void setWeatherText(TFT_t *dev, FontxFile *fx, char * text);
void setStatusText(TFT_t *dev, FontxFile *fx, char * text);
void setPhoneText(TFT_t *dev, FontxFile *fx, char * text);
void setAutoManualText(TFT_t *dev, FontxFile *fx, char *text);

#endif