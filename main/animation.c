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

void setLSV(TFT_t *dev, FontxFile *fx, uint16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[6];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 119, 124, 119 + fontHeight, 124 + fontWidth * 5, BLACK);
    lcdDrawString(dev, fx, 119, 124, text, YELLOW);
}

void setRSV(TFT_t *dev, FontxFile *fx, uint16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[6];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 101, 124, 101 + fontHeight, 124 + fontWidth * 5, BLACK);
    lcdDrawString(dev, fx, 101, 124, text, CYAN);
}

void setLST(TFT_t *dev, FontxFile *fx, uint16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[6];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 83, 124, 83 + fontHeight, 124 + fontWidth * 5, BLACK);
    lcdDrawString(dev, fx, 83, 124, text, YELLOW);
}

void setRST(TFT_t *dev, FontxFile *fx, uint16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[6];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 65, 124, 65 + fontHeight, 124 + fontWidth * 5, BLACK);
    lcdDrawString(dev, fx, 65, 124, text, CYAN);
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

void drawRainy(TFT_t *dev, uint16_t offsetX, uint16_t offsetY)
{
    char file[32];
    strcpy(file, "/spiffs/rainy.jpg");
    drawImage(dev, file, offsetX, offsetY, 65, 65);
}

void drawSunny(TFT_t *dev, uint16_t offsetX, uint16_t offsetY)
{
    char file[32];
    strcpy(file, "/spiffs/sunny.jpg");
    drawImage(dev, file, offsetX, offsetY, 65, 65);
}

void drawSunshower(TFT_t *dev, uint16_t offsetX, uint16_t offsetY)
{
    char file[32];
    strcpy(file, "/spiffs/sunshower.jpg");
    drawImage(dev, file, offsetX, offsetY, 65, 65);
}

void setWeatherText(TFT_t *dev, FontxFile *fx, char *text)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    lcdDrawFillRect(dev, 44, 124, 44 + fontHeight, 124 + fontWidth * 12, BLACK);
    lcdDrawString(dev, fx, 44, 124, text, WHITE);
}

void setStatusText(TFT_t *dev, FontxFile *fx, char *text)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    lcdDrawFillRect(dev, 28, 112, 28 + fontHeight, 112 + fontWidth * 12, BLACK);
    lcdDrawString(dev, fx, 28, 112, text, WHITE);
}

void setPhoneText(TFT_t *dev, FontxFile *fx, char *text)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    lcdDrawFillRect(dev, 11, 124, 11 + fontHeight, 124 + fontWidth * 12, BLACK);
    lcdDrawString(dev, fx, 11, 124, text, WHITE);
}

void setAutoManualText(TFT_t *dev, FontxFile *fx, char *text)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    lcdDrawFillRect(dev, 143, 90, 143 + fontHeight, 90 + fontWidth * 6, BLACK);
    lcdDrawString(dev, fx, 143, 90, text, BLACK);
}
