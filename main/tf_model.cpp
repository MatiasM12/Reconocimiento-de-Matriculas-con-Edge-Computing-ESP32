#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_timer.h>
#include "tf_model_data.h"
#include "esp_heap_caps.h"

#define IMAGE_WIDTH 20
#define IMAGE_HEIGHT 32
#define MODEL_TAG "MODELS"

uint8_t output_buffer[IMAGE_WIDTH * IMAGE_HEIGHT];

constexpr int kTensorArenaSize = 44 * 1024 ;
EXT_RAM_BSS_ATTR static uint8_t tensor_arena[kTensorArenaSize];

// Declaración de los tensores y el intérprete
tflite::ErrorReporter *error_reporter = nullptr;
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;

TfLiteStatus RegisterOps(tflite::MicroMutableOpResolver<10> &op_resolver, bool is_letter)
{
    TF_LITE_ENSURE_STATUS(op_resolver.AddFullyConnected());
    TF_LITE_ENSURE_STATUS(op_resolver.AddQuantize());
    TF_LITE_ENSURE_STATUS(op_resolver.AddReshape());
    TF_LITE_ENSURE_STATUS(op_resolver.AddConv2D());
    TF_LITE_ENSURE_STATUS(op_resolver.AddMaxPool2D());
    TF_LITE_ENSURE_STATUS(op_resolver.AddSoftmax());
    TF_LITE_ENSURE_STATUS(op_resolver.AddDequantize());
    TF_LITE_ENSURE_STATUS(op_resolver.AddMul());
    TF_LITE_ENSURE_STATUS(op_resolver.AddAdd());

    if (is_letter)
    {
        TF_LITE_ENSURE_STATUS(op_resolver.AddLeakyRelu());
    }
    return kTfLiteOk;
}

// Preprocesar la imagen: normalizar los valores de los píxeles
void preprocess_image(uint8_t *raw_data, int8_t *image_data, int width, int height, bool is_int8)
{
    for (int i = 0; i < width * height; i++) {
        if (is_int8) {
            image_data[i] = static_cast<int8_t>(raw_data[i] - 128);
        } else {
            image_data[i] = static_cast<uint8_t>(raw_data[i]);
        }
    }
}

void printPredictedClass(bool is_letter, char* result)
{
    int output_size = is_letter ? 26 : 11; // Tamaño de salida ajustado según si es letra o número
    int8_t max_value = -128;
    int predicted_class = -1;

    for (int i = 0; i < output_size; i++)
    {
        int8_t value = output->data.int8[i];
        if (value > max_value)
        {
            max_value = value;
            predicted_class = i;
        }
    }

    if (predicted_class != -1)
    {
        if (is_letter)
        {
            char predicted_letter = 'A' + predicted_class;
            *result = predicted_letter; 
            ESP_LOGI(MODEL_TAG, "The image belongs to letter: %c", predicted_letter);
        }
        else
        {
            char predicted_number = '0' + predicted_class; 
            *result = predicted_number;
            ESP_LOGI(MODEL_TAG, "The image belongs to number: %c", predicted_number);
        }
    }
    else
    {
        *result = '\0'; 
        ESP_LOGE(MODEL_TAG, "No class predicted.");
    }
}

void run_inference(bool is_letter, const unsigned char *model_data, uint8_t* input_buffer, char* result)
{
    // Cargar el modelo
    int64_t start_time = esp_timer_get_time();
    model = tflite::GetModel(model_data);
    int64_t end_time = esp_timer_get_time();
    ESP_LOGI(MODEL_TAG, "Model load time: %.6f s", (end_time - start_time)/ 1000000.0);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        ESP_LOGE(MODEL_TAG, "Model schema version mismatch");
        return;
    }

    // Configurar el intérprete y registrar operaciones
    tflite::MicroMutableOpResolver<10> micro_op_resolver;
    RegisterOps(micro_op_resolver, is_letter);

    tflite::MicroInterpreter static_interpreter(model, micro_op_resolver, tensor_arena, kTensorArenaSize);
    interpreter = &static_interpreter;

    // Asignar tensores
    if (interpreter->AllocateTensors() != kTfLiteOk)
    {
        ESP_LOGE(MODEL_TAG, "Failed to allocate tensors.");
        return;
    }

    // Obtener tensores de entrada y salida
    input = interpreter->input(0);
    output = interpreter->output(0);
    bool is_int8 = (input->type == kTfLiteInt8);

    // Preprocesar la imagen
    int8_t image_data[IMAGE_WIDTH * IMAGE_HEIGHT];
    preprocess_image(input_buffer, image_data, IMAGE_WIDTH, IMAGE_HEIGHT, is_int8);

    // Copiar datos preprocesados al tensor de entrada
    memcpy(input->data.int8, image_data, sizeof(image_data));

    // Ejecutar la inferencia
    start_time = esp_timer_get_time();
    interpreter->Invoke();
    end_time = esp_timer_get_time();
    ESP_LOGI(MODEL_TAG, "Inference time: %.6f s", (end_time - start_time)/ 1000000.0);

    // Manejar la salida del modelo
    printPredictedClass(is_letter, result);
}

void run_model(bool is_letter, uint8_t* input_buffer, char* result)
{
    const unsigned char* model_to_use = is_letter ? model_tflite : number_model_tflite;
    run_inference(is_letter, model_to_use, input_buffer, result);
}

