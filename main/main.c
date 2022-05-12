/**
 ******************************************************************************
 * @file           : main.c
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
#include "nvs_flash.h"
#include "nvs.h"

#include "lcd_com.h"
#include "lcd_lib.h"
#include "fontx.h"
#include "decode_jpeg.h"
#include "animation.h"

#include "ili9225.h"
#include "adc_mux_4067.h"
#include "relay_motor_library.h"
#include "main.h"
#include "port.h"
#include "math.h"

#define DRIVER "ST7775"
#define INTERVAL 500
#define WAIT vTaskDelay(INTERVAL)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static const char *TAG_I2S = "[HC595_I2S]";
static const char *TAG_SPIFFS = "[SPIFFS]";

static uint16_t temp1_value = 0; // Active
static uint16_t temp2_value = 0; // End
static uint16_t temp3_value = 0; // REF
static uint16_t temp4_value = 0; // Store

static uint16_t level1_value = 0;
static uint16_t level2_value = 0;
static uint16_t level3_value = 0;
static uint16_t level4_value = 0;
static uint16_t level5_value = 0;
static uint16_t water_level = 0;

static uint16_t end_diff = 0;
static uint16_t active_diff = 0;
static uint16_t store_diff = 0;

rm_control_t rm_control = {
    .MOTOR_EN = false,
    .RELAY_EN = false,
};
nvs_handle_t nvs_handle_data;
SemaphoreHandle_t xSemaphore1;
menu_list_t menu_list = MENU_IDLE;

uint16_t calcTemp(uint16_t temp_value)
{
    return (uint16_t)((float)TEMP_P1_COEFF * temp_value * temp_value + TEMP_P2_COEFF * temp_value + TEMP_P3_COEFF);
}

static void checkSPIFFS(char *path)
{
    DIR *dir = opendir(path);
    assert(dir != NULL);
    while (1)
    {
        struct dirent *data = readdir(dir);
        if (!data)
            break;
        ESP_LOGI(TAG_SPIFFS, "d_name=%s", data->d_name);
    }
    closedir(dir);
}

TickType_t JPEGLOGO(TFT_t *dev, char *file, int width, int height)
{
    TickType_t startTick, endTick, diffTick;
    lcdSetFontDirection(dev, 0);
    lcdFillScreen(dev, BLACK);
    startTick = xTaskGetTickCount();
    pixel_jpeg **pixels;
    uint16_t imageWidth;
    uint16_t imageHeight;
    // uint freeRAM = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    // ESP_LOGI("[DRAM]", "free RAM is %d.", freeRAM);
    esp_err_t err = decode_jpeg(&pixels, file, width, height, &imageWidth, &imageHeight);
    if (err == ESP_OK)
    {
        ESP_LOGI(__FUNCTION__, "imageWidth=%d imageHeight=%d", imageWidth, imageHeight);

        uint16_t jpegWidth = width;
        uint16_t offsetX = 0;
        if (width > imageWidth)
        {
            jpegWidth = imageWidth;
            offsetX = (width - imageWidth) / 2;
        }
        ESP_LOGD(__FUNCTION__, "jpegWidth=%d offsetX=%d", jpegWidth, offsetX);

        uint16_t jpegHeight = height;
        uint16_t offsetY = 0;
        if (height > imageHeight)
        {
            jpegHeight = imageHeight;
            offsetY = (height - imageHeight) / 2;
        }
        ESP_LOGD(__FUNCTION__, "jpegHeight=%d offsetY=%d", jpegHeight, offsetY);
        uint16_t *colors = (uint16_t *)malloc(sizeof(uint16_t) * jpegWidth);
        for (int y = 0; y < jpegHeight; y++)
        {
            for (int x = 0; x < jpegWidth; x++)
            {
                colors[x] = pixels[y][x];
            }
            lcdDrawMultiPixels(dev, offsetX, y + offsetY, jpegWidth, colors);
            // vTaskDelay(1);
        }
        free(colors);
        release_image(&pixels, width, height);
        ESP_LOGD(__FUNCTION__, "Finish");
    }
    else
    {
        ESP_LOGE(__FUNCTION__, "decode_image err=%d imageWidth=%d imageHeight=%d", err, imageWidth, imageHeight);
    }

    endTick = xTaskGetTickCount();
    diffTick = endTick - startTick;
    ESP_LOGI(__FUNCTION__, "elapsed time[ms]:%d", diffTick * portTICK_RATE_MS);
    return diffTick;
}

static void taskLCDContoller()
{
    FontxFile fx16G[2];
    FontxFile fx24G[2];
    FontxFile fx32G[2];
    InitFontx(fx16G, "/spiffs/ILGH16XB.FNT", ""); // 8x16Dot Gothic
    InitFontx(fx24G, "/spiffs/ILGH24XB.FNT", ""); // 12x24Dot Gothic
    InitFontx(fx32G, "/spiffs/ILGH32XB.FNT", ""); // 16x32Dot Gothic
    FontxFile fx16M[2];
    FontxFile fx24M[2];
    FontxFile fx32M[2];
    InitFontx(fx16M, "/spiffs/ILMH16XB.FNT", ""); // 8x16Dot Mincyo
    InitFontx(fx24M, "/spiffs/ILMH24XB.FNT", ""); // 12x24Dot Mincyo
    InitFontx(fx32M, "/spiffs/ILMH32XB.FNT", ""); // 16x32Dot Mincyo
    TFT_t dev;
    lcd_interface_cfg(&dev, 1);
    ili9225_lcdInit(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);
    // char stats_buffer[1024];
    // vTaskList(stats_buffer);
    // ESP_LOGI("[stats_buffer]", "%s", stats_buffer);
#if CONFIG_INVERSION
    ESP_LOGI(TAG, "Enable Display Inversion");
    lcdInversionOn(&dev);
#endif
    while (1)
    {
        char file[32];
        strcpy(file, "/spiffs/logo_gamo.jpg");
        JPEGLOGO(&dev, file, CONFIG_WIDTH, CONFIG_HEIGHT);
        nvs_get_u16(nvs_handle_data, "end_diff", &end_diff);
        nvs_get_u16(nvs_handle_data, "active_diff", &active_diff);
        nvs_get_u16(nvs_handle_data, "store_diff", &store_diff);
        WAIT;
        strcpy(file, "/spiffs/background.jpg");
        JPEGLOGO(&dev, file, CONFIG_WIDTH, CONFIG_HEIGHT);
        vTaskDelay(10);
        while (1)
        {
            // setT_REF(&dev, fx16G, level1_value / 100);
            // setT_ACTIVE(&dev, fx16G, level1_value % 100);
            // setT_REF(&dev, fx16G, (int)((float)TEMP_P1_COEFF * temp1_value * temp1_value + TEMP_P2_COEFF * temp1_value + TEMP_P3_COEFF));
            // setT_ACTIVE(&dev, fx16G, (int)((float)TEMP_P1_COEFF * temp2_value * temp2_value + TEMP_P2_COEFF * temp2_value + TEMP_P3_COEFF));
            if (menu_list == MENU_END)
            {
                lcdSetFontUnderLine(&dev, RED);
                setT_REF(&dev, fx16G, end_diff);
                lcdUnsetFontUnderLine(&dev);
            }
            else
            {
                setT_REF(&dev, fx16G, calcTemp(temp3_value));
            }
            if (menu_list == MENU_ACTIVE)
            {
                lcdSetFontUnderLine(&dev, RED);
                setT_ACTIVE(&dev, fx16G, active_diff);
                lcdUnsetFontUnderLine(&dev);
            }
            else
            {
                setT_ACTIVE(&dev, fx16G, calcTemp(temp1_value));
            }
            if (menu_list == MENU_STORE)
            {
                lcdSetFontUnderLine(&dev, RED);
                setT_STORE(&dev, fx16G, store_diff);
                lcdUnsetFontUnderLine(&dev);
            }
            else
            {
                setT_STORE(&dev, fx16G, calcTemp(temp4_value));
            }
            setWaterLevel(&dev, water_level * 20);
            if (rm_control.MOTOR_EN == 1)
            {
                drawMotorOn(&dev, 83, 94);
            }
            else if (rm_control.MOTOR_EN == 0)
            {
                drawMotorOff(&dev, 83, 94);
            }
            if (rm_control.RELAY_EN == 1)
            {
                drawValveOn(&dev, 45, 99);
            }
            else if (rm_control.RELAY_EN == 0)
            {
                drawValveOff(&dev, 45, 99);
            }
            xSemaphoreTake(xSemaphore1, pdMS_TO_TICKS(500));
            // SM04M_getDistance(&distance);
            // setSV(&dev, fx16G, (uint8_t)random() % 100);
            // setCV(&dev, fx16G, distance);
            // setP(&dev, fx16G, (uint8_t)random() % 100);
            // setI(&dev, fx16G, (uint8_t)random() % 100);
            // setD(&dev, fx16G, (uint8_t)random() % 100);
            // drawLightRED(&dev, 94, 78);
            // setDisplaySpeed(&dev, (uint8_t)random() % 100);
            // setDisplayLevel(&dev, MIN(500, MAX(250, distance)) * (-4) / 10 + 200);
            // vTaskDelay(pdMS_TO_TICKS(200));
            // drawLightGREEN(&dev, 94, 78);
            // setDisplaySpeed(&dev, (uint8_t)random() % 100);
            // setDisplayLevel(&dev, (uint8_t)random() % 100);
            // lcdDrawFillRect(&dev, 15, 188, 144, 197, BLACK);
            // vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
    vTaskDelete(NULL);
}

static void taskADCMUX4067()
{
    ADCMUX4067_Init();
    while (1)
    {
        ADCMUX4067_GetVoltage(&temp1_value, HC4067_TEMP3_BIT);
        ADCMUX4067_GetVoltage(&temp2_value, HC4067_TEMP4_BIT);
        ADCMUX4067_GetVoltage(&temp3_value, HC4067_TEMP5_BIT);
        ADCMUX4067_GetVoltage(&temp4_value, HC4067_TEMP2_BIT);

        ADCMUX4067_GetValue(&level5_value, HC4067_LEVEL5_BIT);
        ADCMUX4067_GetValue(&level4_value, HC4067_LEVEL4_BIT);
        ADCMUX4067_GetValue(&level3_value, HC4067_LEVEL3_BIT);
        ADCMUX4067_GetValue(&level2_value, HC4067_LEVEL2_BIT);
        ADCMUX4067_GetValue(&level1_value, HC4067_LEVEL1_BIT);
        if (level5_value > 2000)
            water_level = 5;
        else if (level4_value < 2000)
            water_level = 4;
        else if (level3_value < 2000)
            water_level = 3;
        else if (level2_value < 2000)
            water_level = 2;
        else if (level1_value < 2000)
            water_level = 1;
        else
            water_level = 0;

        vTaskDelay(pdMS_TO_TICKS(200));
    }
    vTaskDelete(NULL);
}

static void taskRelayMotor()
{
    RelayMotor_Init();
    RelayMotor_RelayOff(&rm_control);
    RelayMotor_MotorOff(&rm_control);
    while (1)
    {
        if (water_level == 5)
        {
            RelayMotor_RelayOff(&rm_control);
            if (calcTemp(temp3_value) - calcTemp(temp4_value) >= store_diff)
            {
                if (calcTemp(temp3_value) - calcTemp(temp1_value) <= active_diff)
                {
                    RelayMotor_MotorOn(&rm_control);
                }
            }
            if (calcTemp(temp3_value) - calcTemp(temp2_value) >= end_diff)
            {
                RelayMotor_MotorOff(&rm_control);
            }
        }
        else
        {
            RelayMotor_MotorOff(&rm_control);
            if (calcTemp(temp3_value) - calcTemp(temp4_value) >= store_diff)
            {
                if (calcTemp(temp3_value) - calcTemp(temp1_value) <= active_diff)
                {
                    RelayMotor_RelayOn(&rm_control);
                }
            }
            if (calcTemp(temp3_value) - calcTemp(temp2_value) >= end_diff)
            {
                RelayMotor_RelayOff(&rm_control);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelete(NULL);
}

static void taskButton()
{
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = false,
        .pull_up_en = false,
    };
    io_conf.pin_bit_mask = (1ULL << SWITCH_UP_PIN) | (1ULL << SWITCH_DOWN_PIN) | (1ULL << SWITCH_LEFT_PIN) | (1ULL << SWITCH_RIGHT_PIN);
    gpio_config(&io_conf);
    while (1)
    {
        if (gpio_get_level(SWITCH_LEFT_PIN) == 0)
        {
            menu_list--;
            if (menu_list < MENU_IDLE)
                menu_list = MENU_STORE;
            xSemaphoreGive(xSemaphore1);
        }
        else if (gpio_get_level(SWITCH_RIGHT_PIN) == 0)
        {
            menu_list++;
            if (menu_list > MENU_STORE)
                menu_list = MENU_IDLE;
            xSemaphoreGive(xSemaphore1);
        }
        else if (gpio_get_level(SWITCH_DOWN_PIN) == 0)
        {
            if (menu_list == MENU_END)
            {
                if (end_diff > 0)
                {
                    end_diff--;
                }
                nvs_set_u16(nvs_handle_data, "end_diff", end_diff);
                xSemaphoreGive(xSemaphore1);
            }
            else if (menu_list == MENU_ACTIVE)
            {
                if (active_diff > 0)
                {
                    active_diff--;
                }
                nvs_set_u16(nvs_handle_data, "active_diff", active_diff);
                xSemaphoreGive(xSemaphore1);
            }
            else if (menu_list == MENU_STORE)
            {
                if (store_diff > 0)
                {
                    store_diff--;
                }
                nvs_set_u16(nvs_handle_data, "store_diff", store_diff);
                xSemaphoreGive(xSemaphore1);
            }
        }
        else if (gpio_get_level(SWITCH_UP_PIN) == 0)
        {
            if (menu_list == MENU_END)
            {
                if (end_diff < 50)
                {
                    end_diff++;
                }
                nvs_set_u16(nvs_handle_data, "end_diff", end_diff);
                xSemaphoreGive(xSemaphore1);
            }
            else if (menu_list == MENU_ACTIVE)
            {
                if (active_diff < 50)
                {
                    active_diff++;
                }
                nvs_set_u16(nvs_handle_data, "active_diff", active_diff);
                xSemaphoreGive(xSemaphore1);
            }
            else if (menu_list == MENU_STORE)
            {
                if (store_diff < 50)
                {
                    store_diff++;
                }
                nvs_set_u16(nvs_handle_data, "store_diff", store_diff);
                xSemaphoreGive(xSemaphore1);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(300));
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    xSemaphore1 = xSemaphoreCreateBinary();
    ESP_LOGI(TAG_SPIFFS, "Initializing SPIFFS");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 10,
        .format_if_mount_failed = true,
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG_SPIFFS, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG_SPIFFS, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG_SPIFFS, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_SPIFFS, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG_SPIFFS, "Partition size: total: %d, used: %d", total, used);
    }
    // esp_spiffs_format(conf.partition_label);
    checkSPIFFS("/spiffs/"); // Check files
    ESP_LOGI(TAG_I2S, "Starting init LCD_I2S");
    HC595_I2SInit();
    nvs_flash_init();
    nvs_open("storage", NVS_READWRITE, &nvs_handle_data);
    xTaskCreate(taskLCDContoller, "[taskLCDContoller]", 1024 * 6, NULL, 2, NULL);
    xTaskCreate(taskADCMUX4067, "[taskADCMUX4067]", 1024 * 3, NULL, 3, NULL);
    xTaskCreate(taskRelayMotor, "[taskRelayMotor]", 1024 * 3, NULL, 3, NULL);
    xTaskCreate(taskButton, "[taskButton]", 1024 * 3, NULL, 2, NULL);
}
