#include "image_provider.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <stdio.h>
#include <dirent.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <nvs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "esp_camera.h"
#include <esp_timer.h>
#include "esp_log.h"
#include <iostream>
#include <string.h>
#include "esp_camera.h"
#include "esp_log.h"
#include <stdlib.h>

#define CAMERA_TAG "CAMERA"
#define IMAGE_TAG "IMAGE_PROVIDER"

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 // software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

#define CONFIG_XCLK_FREQ 20000000

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    .xclk_freq_hz = CONFIG_XCLK_FREQ,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_GRAYSCALE, // YUV422,PIXFORMAT_GRAYSCALE,RGB565,PIXFORMAT_JPEG
    .frame_size = FRAMESIZE_VGA,         // QQVGA-QXGA Do not use sizes above QVGA when not JPEG
    .jpeg_quality = 16,                  // 0-63 lower number means higher quality
    .fb_count = 1,                       // if more than one, i2s runs in continuous mode. Use only with JPEG

    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};


esp_err_t init_camera() {
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(CAMERA_TAG, "Error inicializando la cámara: %d", err);
        return err;
    }
    ESP_LOGI(CAMERA_TAG, "Cámara inicializada correctamente");
    return ESP_OK;
}

// Inicialización de la SD
void init_sd() {
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 3,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    esp_err_t err = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (err != ESP_OK) {
        ESP_LOGE(IMAGE_TAG, "Failed to mount SD card, error code: %d", err);
        return;
    }
    ESP_LOGI(IMAGE_TAG, "SD card mounted successfully.");
}

void load_image_from_sd(const char* directory_path, uint8_t** output_data, size_t* output_size) {
    const char *input_image_path = directory_path;
    init_sd();

    FILE *file = fopen(input_image_path, "rb");
    if (file == NULL) {
        ESP_LOGE("SD", "Failed to open file for reading: %s", input_image_path);
        return;
    }

    fseek(file, 0, SEEK_END);
    *output_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *output_data = (uint8_t *)malloc(*output_size);
    if (*output_data == NULL) {
        ESP_LOGE("IMAGE_PROVIDER", "Failed to allocate memory for image data");
        fclose(file);
        return;
    }

    fread(*output_data, 1, *output_size, file);
    fclose(file);

    ESP_LOGI("IMAGE_PROVIDER", "Image loaded from SD card, size: %d bytes", *output_size);
}

void capture_image_from_camera(uint8_t** output_data, size_t* output_size) {
    init_camera();
    init_sd();  // Asegurar que la SD esté inicializada

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(CAMERA_TAG, "Error al capturar imagen");
        *output_data = NULL;
        *output_size = 0;
        return;
    }

    // Copiar los datos de la imagen
    *output_size = fb->len;
    *output_data = (uint8_t*)malloc(*output_size);
    if (*output_data == NULL) {
        ESP_LOGE(CAMERA_TAG, "Fallo en la asignación de memoria");
        esp_camera_fb_return(fb);
        *output_size = 0;
        return;
    }
    memcpy(*output_data, fb->buf, *output_size);

    ESP_LOGI(CAMERA_TAG, "Imagen capturada, tamaño: %d bytes", *output_size);

    // Guardar la imagen en la SD
    const char* filename = "/sdcard/ca1.bmp";
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        ESP_LOGE(CAMERA_TAG, "Error al abrir archivo para escritura en SD");
    } else {
        fwrite(fb->buf, 1, fb->len, file);
        fclose(file);
        ESP_LOGI(CAMERA_TAG, "Imagen guardada en: %s", filename);
    }

    // Liberar el frame buffer
    esp_camera_fb_return(fb);
}