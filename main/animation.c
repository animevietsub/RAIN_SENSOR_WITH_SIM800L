/**
 ******************************************************************************
 * @file           : animation.c
 * @brief          : WLC_LCD176X220_PID
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
#include "animation.h"

void setT_REF(TFT_t *dev, FontxFile *fx, uint16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[6];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 124, 3, 124 + fontHeight, 3 + fontWidth * 2, BLACK);
    lcdDrawString(dev, fx, 124, 3 + (value / 10 == 0) * fontWidth, text, WHITE);
}

void setT_ACTIVE(TFT_t *dev, FontxFile *fx, uint16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[6];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 124, 37, 124 + fontHeight, 37 + fontWidth * 2, BLACK);
    lcdDrawString(dev, fx, 124, 37 + (value / 10 == 0) * fontWidth, text, WHITE);
}

void setT_STORE(TFT_t *dev, FontxFile *fx, uint16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[6];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 124, 72, 124 + fontHeight, 72 + fontWidth * 2, BLACK);
    lcdDrawString(dev, fx, 124, 72 + (value / 10 == 0) * fontWidth, text, WHITE);
}

void drawImage(TFT_t *dev, char *file, uint16_t offsetX, uint16_t offsetY, uint16_t width, uint16_t height)
{
    pixel_jpeg **pixels;
    uint16_t imageWidth;
    uint16_t imageHeight;
    esp_err_t err = decode_jpeg(&pixels, file, width, height, &imageWidth, &imageHeight);
    if (err == ESP_OK)
    {
        uint16_t *colors = (uint16_t *)malloc(sizeof(uint16_t) * width);
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                colors[x] = pixels[y][x];
            }
            lcdDrawMultiPixels(dev, offsetX, y + offsetY, width, colors);
        }
        free(colors);
        release_image(&pixels, width, height);
    }
    else
    {
        ESP_LOGE(__FUNCTION__, "decode_image err=%d imageWidth=%d imageHeight=%d", err, imageWidth, imageHeight);
    }
}

void drawMotorOff(TFT_t *dev, uint16_t offsetX, uint16_t offsetY)
{
    char file[32];
    strcpy(file, "/spiffs/motor_off.jpg");
    drawImage(dev, file, offsetX, offsetY, 32, 46);
}

void drawMotorOn(TFT_t *dev, uint16_t offsetX, uint16_t offsetY)
{
    char file[32];
    strcpy(file, "/spiffs/motor_on.jpg");
    drawImage(dev, file, offsetX, offsetY, 32, 46);
}

void drawValveOn(TFT_t *dev, uint16_t offsetX, uint16_t offsetY)
{
    char file[32];
    strcpy(file, "/spiffs/valve_off.jpg");
    drawImage(dev, file, offsetX, offsetY, 32, 42);
}

void drawValveOff(TFT_t *dev, uint16_t offsetX, uint16_t offsetY)
{
    char file[32];
    strcpy(file, "/spiffs/valve_on.jpg");
    drawImage(dev, file, offsetX, offsetY, 32, 42);
}