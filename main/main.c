/**
 ******************************************************************************
 * @file           : main.c
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
#include "l298n_library.h"
#include "adc_sensor_library.h"

#include "port.h"
#include "main.h"

#define DRIVER "ST7775"
#define INTERVAL 500
#define WAIT vTaskDelay(INTERVAL)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static const char *TAG_I2S = "[HC595_I2S]";
static const char *TAG_SPIFFS = "[SPIFFS]";
static char AUTO_MANUAL[10];
static uint16_t ldr_value = 0;
static uint16_t rain_value = 0;
static uint16_t ldr_threshold = 0;
static uint16_t rain_threshold = 0;
static uint8_t start_count = false;

weather_status_t weather_status = WEATHER_SUNNY;
menu_list_t menu_list = MENU_IDLE;
l298n_direction_t cur_dir = L298N_DIRECTION_CW;
nvs_handle_t nvs_handle_data;
my_action_t my_action = ACTION_STORING;
l298n_control_t l298n_control;

SemaphoreHandle_t xSemaphore1;

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
        nvs_get_u16(nvs_handle_data, "ldr_threshold", &ldr_threshold);
        nvs_get_u16(nvs_handle_data, "rain_threshold", &rain_threshold);
        WAIT;

        strcpy(file, "/spiffs/background.jpg");
        JPEGLOGO(&dev, file, CONFIG_WIDTH, CONFIG_HEIGHT);
        vTaskDelay(10);
        char COMMON_TEXT[32];
        strcpy(AUTO_MANUAL, "AUTO");
        while (1)
        {
            if (menu_list == MENU_MANUAL_AUTO)
                lcdSetFontUnderLine(&dev, RED);
            lcdSetFontFill(&dev, GREEN);
            setAutoManualText(&dev, fx16M, AUTO_MANUAL);
            lcdUnsetFontUnderLine(&dev);
            lcdUnsetFontFill(&dev);
            setLSV(&dev, fx16G, ldr_value);
            setRSV(&dev, fx16G, rain_value);
            if (menu_list == MENU_LST)
                lcdSetFontUnderLine(&dev, RED);
            setLST(&dev, fx16G, ldr_threshold);
            lcdUnsetFontUnderLine(&dev);
            if (menu_list == MENU_RST)
                lcdSetFontUnderLine(&dev, RED);
            setRST(&dev, fx16G, rain_threshold);
            lcdUnsetFontUnderLine(&dev);
            switch (weather_status)
            {
            case WEATHER_SUNNY:
                drawSunny(&dev, 74, 15);
                strcpy(COMMON_TEXT, "Sunny");
                setWeatherText(&dev, fx16G, COMMON_TEXT);
                break;
            case WEATHER_SUNSHOWER:
                drawSunshower(&dev, 74, 15);
                strcpy(COMMON_TEXT, "Sunshower");
                setWeatherText(&dev, fx16G, COMMON_TEXT);
                break;
            case WEATHER_RAINY:
                drawRainy(&dev, 74, 15);
                strcpy(COMMON_TEXT, "Rainy");
                setWeatherText(&dev, fx16G, COMMON_TEXT);
                break;
            }
            switch (my_action)
            {
            case ACTION_DRYING:
                strcpy(COMMON_TEXT, "Drying");
                setStatusText(&dev, fx16G, COMMON_TEXT);
                break;
            case ACTION_STORING:
                strcpy(COMMON_TEXT, "Storing");
                setStatusText(&dev, fx16G, COMMON_TEXT);
                break;
            }
            strcpy(COMMON_TEXT, "0706825803");
            setPhoneText(&dev, fx16G, COMMON_TEXT);
            xSemaphoreTake(xSemaphore1, pdMS_TO_TICKS(500));
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

static void taskMotorController()
{
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = false,
        .pull_up_en = true,
    };
    io_conf.pin_bit_mask = (1ULL << SWITCH_H_PIN) | (1ULL << SWITCH_L_PIN);
    gpio_config(&io_conf);
    L298N_Init(&l298n_control);
    if (gpio_get_level(SWITCH_L_PIN) == false)
    {

        start_count = true;
        L298N_SetDirection(&l298n_control, L298N_CHANNEL_B, L298N_DIRECTION_CW);
    }
    while (1)
    {
        if (gpio_get_level(SWITCH_H_PIN) == true && gpio_get_level(SWITCH_L_PIN) == false && (weather_status == WEATHER_SUNNY))
        {
            L298N_Brake(&l298n_control, L298N_CHANNEL_B);
            strcpy(AUTO_MANUAL, "AUTO");
        }
        else if (gpio_get_level(SWITCH_H_PIN) == true && gpio_get_level(SWITCH_L_PIN) == false && ((weather_status != WEATHER_SUNNY) || my_action == ACTION_STORING))
        {
            start_count = true;
            L298N_SetDirection(&l298n_control, L298N_CHANNEL_B, L298N_DIRECTION_CW);
            my_action = ACTION_STORING;
        }
        else if (gpio_get_level(SWITCH_L_PIN) == true && gpio_get_level(SWITCH_H_PIN) == false && weather_status != WEATHER_SUNNY)
        {
            L298N_Brake(&l298n_control, L298N_CHANNEL_B);
            strcpy(AUTO_MANUAL, "AUTO");
        }
        else if ((gpio_get_level(SWITCH_L_PIN) == true && gpio_get_level(SWITCH_H_PIN) == false && my_action == ACTION_DRYING))
        {
            start_count = true;
            L298N_SetDirection(&l298n_control, L298N_CHANNEL_B, L298N_DIRECTION_CCW);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}

static void taskADCSensor()
{
    ADCSensor_Init();
    while (1)
    {
        ADCSensor_GetLDRValue(&ldr_value);
        ADCSensor_GetRainValue(&rain_value);
        if (ldr_value < ldr_threshold)
        {
            if (rain_value < rain_threshold)
            {
                weather_status = WEATHER_SUNSHOWER;
            }
            else
            {
                weather_status = WEATHER_SUNNY;
            }
        }
        else
        {
            weather_status = WEATHER_RAINY;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
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
        if (gpio_get_level(SWITCH_UP_PIN) == 0)
        {
            menu_list--;
            if (menu_list < MENU_IDLE)
                menu_list = MENU_RST;
            xSemaphoreGive(xSemaphore1);
        }
        else if (gpio_get_level(SWITCH_DOWN_PIN) == 0)
        {
            menu_list++;
            if (menu_list > MENU_RST)
                menu_list = MENU_IDLE;
            xSemaphoreGive(xSemaphore1);
        }
        else if (gpio_get_level(SWITCH_LEFT_PIN) == 0)
        {
            if (menu_list == MENU_LST)
            {
                if (ldr_threshold > 0)
                {
                    ldr_threshold--;
                }
                else
                {
                    ldr_threshold = 4600;
                }
                nvs_set_u16(nvs_handle_data, "ldr_threshold", ldr_threshold);
                xSemaphoreGive(xSemaphore1);
            }
            else if (menu_list == MENU_RST)
            {
                if (rain_threshold > 0)
                {
                    rain_threshold--;
                }
                nvs_set_u16(nvs_handle_data, "rain_threshold", rain_threshold);
                xSemaphoreGive(xSemaphore1);
            }
            else if (menu_list == MENU_MANUAL_AUTO)
            {
                strcpy(AUTO_MANUAL, "MANUAL");
                my_action = ACTION_DRYING;
                xSemaphoreGive(xSemaphore1);
            }
        }
        else if (gpio_get_level(SWITCH_RIGHT_PIN) == 0)
        {
            if (menu_list == MENU_LST)
            {
                if (ldr_threshold < 4600)
                {
                    ldr_threshold++;
                }
                else
                {
                    ldr_threshold = 0;
                }
                nvs_set_u16(nvs_handle_data, "ldr_threshold", ldr_threshold);
                xSemaphoreGive(xSemaphore1);
            }
            else if (menu_list == MENU_RST)
            {
                if (rain_threshold < 4600)
                {
                    rain_threshold++;
                }
                else
                {
                    rain_threshold = 0;
                }
                nvs_set_u16(nvs_handle_data, "rain_threshold", rain_threshold);
                xSemaphoreGive(xSemaphore1);
            }
            else if (menu_list == MENU_MANUAL_AUTO)
            {
                strcpy(AUTO_MANUAL, "MANUAL");
                my_action = ACTION_STORING;
                xSemaphoreGive(xSemaphore1);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    vTaskDelete(NULL);
}

static void taskSpeedControl()
{
    uint32_t start_time = 0;
    while (1)
    {
        if (start_count)
        {
            start_time = xTaskGetTickCount();
            L298N_SetPWMDuty(&l298n_control, L298N_CHANNEL_B, 100);
            start_count = false;
        }
        if (pdTICKS_TO_MS(xTaskGetTickCount() - start_time) > 500)
        {
            L298N_SetPWMDuty(&l298n_control, L298N_CHANNEL_B, 75);
        }
        vTaskDelay(10);
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
    xTaskCreate(taskMotorController, "[taskMotorController]", 1024 * 3, NULL, 2, NULL);
    xTaskCreate(taskADCSensor, "[taskADCSensor]", 1024 * 3, NULL, 2, NULL);
    xTaskCreate(taskButton, "[taskButton]", 1024 * 3, NULL, 2, NULL);
    xTaskCreate(taskSpeedControl, "[taskSpeedControl]", 1024 * 3, NULL, 2, NULL);
}