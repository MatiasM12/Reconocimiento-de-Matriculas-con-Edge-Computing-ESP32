#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image_provider.h"
#include "freertos/FreeRTOS.h"
#include "esp_camera.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <esp_system.h>
#include "pipeline_runner.h"
#include <esp_timer.h>
#include "esp_heap_trace.h"

// #define NUM_RECORDS 100
// static heap_trace_record_t trace_record[NUM_RECORDS]; 

bool USE_SD_IMAGE = false;
const char* DIRECTORY_PATH = "/sdcard/P0.JPG";
const char* TAG = "MAIN";

uint8_t* input_data = nullptr;
size_t file_size = 0;

void start_pipeline(void *pvParameters)
{    
    const char* format = "BMP";
    if (USE_SD_IMAGE) {
        ESP_LOGI(TAG, "Cargando imagen desde SD...");
        load_image_from_sd(DIRECTORY_PATH, &input_data, &file_size);
        format = (strrchr(DIRECTORY_PATH, '.')) + 1;
    } else {
        ESP_LOGI(TAG, "Capturando imagen con la cámara...");
        capture_image_from_camera(&input_data, &file_size);
    }

    run_pipeline(input_data, file_size, format);
    ESP_LOGI(TAG, "Pipeline finalizado exitosamente");

    free(input_data);
    vTaskDelete(NULL);
}

extern "C" void app_main()
{
    //ESP_ERROR_CHECK( heap_trace_init_standalone(trace_record, NUM_RECORDS) );

    xTaskCreate(start_pipeline, "start_pipeline", 16 * 1024, NULL, 8, NULL);
    vTaskDelete(NULL);
}

