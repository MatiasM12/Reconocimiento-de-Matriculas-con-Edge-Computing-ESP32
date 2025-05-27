#include "pipeline_runner.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <esp_system.h>
#include <string>
#include <sstream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include <esp_timer.h> 
#include "tf_model.h"
#include "esp_heap_trace.h"

#define PIPELINE_TAG "PIPELINE_RUNNER"

using namespace cv;

cv::Mat load_image_by_format(const char* format, const uint8_t* input_data, size_t input_size) {
    cv::Mat input_mat;
    uint64_t start_time, end_time;

    if (strcmp(format, "JPG") == 0 || strcmp(format, "jpg") == 0) {
        start_time = esp_timer_get_time();
        std::vector<uint8_t> buffer(input_data, input_data + input_size);
        input_mat = cv::imdecode(buffer, cv::IMREAD_COLOR); 

        if (input_mat.empty()) {
            ESP_LOGE(PIPELINE_TAG, "Error: No se pudo descomprimir la imagen JPG");
            return input_mat; 
        }

        end_time = esp_timer_get_time();
        ESP_LOGI(PIPELINE_TAG, "Tiempo de descomprimir JPG: %.6f s", (end_time - start_time) / 1000000.0);
        ESP_LOGI(PIPELINE_TAG, "Imagen JPG descomprimida correctamente. Dimensiones: %dx%d", input_mat.cols, input_mat.rows);
    } else if (strcmp(format, "BMP") == 0 || strcmp(format, "bmp") == 0) {
        int width = 640;  
        int height = 480; 

        int channels = (input_size == width * height * 3) ? 3 : (input_size == width * height * 1) ? 1 : 0;

        if (channels == 0) {
            ESP_LOGE(PIPELINE_TAG, "Error: Tamaño de BMP (%zu) no coincide con las dimensiones esperadas (%dx%d)", input_size, width, height);
            return input_mat; 
        }
        
        input_mat = cv::Mat(height, width, (channels == 3) ? CV_8UC3 : CV_8UC1, (void*)input_data);

        ESP_LOGI(PIPELINE_TAG, "Imagen BMP cargada correctamente. Dimensiones: %dx%d", input_mat.cols, input_mat.rows);
    } else {
        ESP_LOGE(PIPELINE_TAG, "Formato no soportado: %s", format);
        return input_mat; 
    }

    return input_mat;
}

void apply_grayscale(cv::Mat &input_mat) {
    if (input_mat.channels() == 3) {
        int64_t start_time = esp_timer_get_time();
        cv::cvtColor(input_mat, input_mat, cv::COLOR_BGR2GRAY);
        int64_t end_time = esp_timer_get_time();
        ESP_LOGI(PIPELINE_TAG, "Convertir a escala de grises (Paso 1): %.6f s", (end_time - start_time) / 1000000.0);
    }
}

void resize_image(cv::Mat &input_mat, int new_width) {
    double aspect_ratio_image = static_cast<double>(input_mat.cols) / input_mat.rows;
    int new_height = static_cast<int>(new_width / aspect_ratio_image);

    int64_t start_time = esp_timer_get_time();
    cv::resize(input_mat, input_mat, cv::Size(new_width, new_height));
    int64_t end_time = esp_timer_get_time();

    ESP_LOGI(PIPELINE_TAG, "Redimensionada a: %dx%d (Paso 2): %.6f s", 
             new_width, new_height, (end_time - start_time) / 1000000.0);
}

void apply_gaussian_blur(const cv::Mat &input_mat, cv::Mat &output_mat, cv::Size kernel_size = cv::Size(5, 5)) {
    int64_t start_time = esp_timer_get_time();
    cv::GaussianBlur(input_mat, output_mat, kernel_size, 0);
    int64_t end_time = esp_timer_get_time();

    ESP_LOGI(PIPELINE_TAG, "Tiempo de desenfoque gaussiano (Paso 3): %.6f s", 
             (end_time - start_time) / 1000000.0);
}

void apply_bilateral_filter(const cv::Mat &input_mat, cv::Mat &output_mat, int d = 9, double sigmaColor = 36, double sigmaSpace = 36) {
    int64_t start_time = esp_timer_get_time();
    cv::bilateralFilter(input_mat, output_mat, d, sigmaColor, sigmaSpace);
    int64_t end_time = esp_timer_get_time();

    ESP_LOGI(PIPELINE_TAG, "Tiempo de bilateral filter (Paso 4): %.6f s", 
             (end_time - start_time) / 1000000.0);
}

void apply_canny_edge_detection(cv::Mat &input_mat, double threshold1 = 75, double threshold2 = 200) {
    int64_t start_time = esp_timer_get_time();
    cv::Canny(input_mat, input_mat, threshold1, threshold2);
    int64_t end_time = esp_timer_get_time();

    ESP_LOGI(PIPELINE_TAG, "Tiempo de filtro Canny (Paso 5): %.6f s", 
             (end_time - start_time) / 1000000.0);
}

void apply_dilation(cv::Mat &input_mat, int kernel_size = 3) {
    cv::Mat kernel = cv::Mat::ones(kernel_size, kernel_size, CV_8U);

    int64_t start_time = esp_timer_get_time();
    cv::dilate(input_mat, input_mat, kernel);
    int64_t end_time = esp_timer_get_time();

    ESP_LOGI(PIPELINE_TAG, "Tiempo de dilatación (Paso 6): %.6f s", 
             (end_time - start_time) / 1000000.0);
}

cv::Mat find_license_plate_candidate(const cv::Mat &input_mat) {
    int64_t start_time = esp_timer_get_time();

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(input_mat, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    ESP_LOGI(PIPELINE_TAG, "Número de contornos encontrados: %d", (int)contours.size());

    // Definir la relación de aspecto y área de la patente
    float aspect_ratio_min = 2.1, aspect_ratio_max = 4.5;
    float min_area = 13224, max_area = 52500;

    cv::Rect best_rect;
    float min_area_lsplate = 300000;
    cv::Mat best_candidate_mat;

    for (const auto& contour : contours) {
        // Obtener el rectángulo delimitador
        cv::Rect bounding_rect = cv::boundingRect(contour);
        double aspect_ratio = static_cast<double>(bounding_rect.width) / bounding_rect.height;
        double area = bounding_rect.width * bounding_rect.height;
        int x = bounding_rect.x;
        int y = bounding_rect.y;
        int w = bounding_rect.width;
        int h = bounding_rect.height;

        // Aproximar el contorno
        double epsilon = 0.07 * cv::arcLength(contour, true);
        std::vector<cv::Point> approx;
        cv::approxPolyDP(contour, approx, epsilon, true);

        // Verificar las condiciones
        if ((approx.size() == 4 || approx.size() == 2 || approx.size() == 6) &&
            area > min_area && area < max_area &&
            aspect_ratio > aspect_ratio_min && aspect_ratio < aspect_ratio_max) {

            // Actualizar el área mínima y el mejor contorno si cumple
            if (area < min_area_lsplate) {
                min_area_lsplate = area;
                best_candidate_mat = input_mat(cv::Rect(x + 3, y, w - 3, h - 3)).clone();
                best_rect = bounding_rect;
            }
        }
    }

    int64_t end_time = esp_timer_get_time();
    ESP_LOGI(PIPELINE_TAG, "Tiempo de localizar y recortar matrícula (Paso 7): %.6f s", 
             (end_time - start_time) / 1000000.0);

    return best_candidate_mat;
}

void apply_erosion(cv::Mat &input_mat) {
    cv::Mat kernel = Mat::ones(3, 2, CV_8U);

    int64_t start_time = esp_timer_get_time();
    cv::erode(input_mat, input_mat, kernel);
    int64_t end_time = esp_timer_get_time();

    ESP_LOGI(PIPELINE_TAG, "Tiempo de erosión (Paso 8): %.6f s", 
             (end_time - start_time) / 1000000.0);
}

void apply_close(cv::Mat &input_mat) {
    cv::Mat kernel = Mat::ones(3, 2, CV_8U);

    int64_t start_time = esp_timer_get_time();
    cv::morphologyEx(input_mat, input_mat, cv::MORPH_CLOSE, kernel);
    int64_t end_time = esp_timer_get_time();

    ESP_LOGI(PIPELINE_TAG, "Tiempo de operación de cierre (Paso 9): %.6f s", 
             (end_time - start_time) / 1000000.0);
}

void find_characters_candidate(cv::Mat &input_mat, std::vector<cv::Mat> &character_images, std::vector<cv::Rect> potential_char_rects) {
    int64_t start_time = esp_timer_get_time();
    
    std::vector<std::vector<cv::Point>> contours_chars;
    std::vector<cv::Vec4i> hierarchy_chars;
    cv::findContours(input_mat, contours_chars, hierarchy_chars, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Filtrar contornos que podrían representar caracteres
    for (const auto& contour : contours_chars) {
        cv::Rect bounding_rect = cv::boundingRect(contour);
        float aspect_ratio_char = static_cast<float>(bounding_rect.width) / bounding_rect.height; 
        float area = cv::contourArea(contour);

        if ((0.015 < aspect_ratio_char && aspect_ratio_char < 0.7) && (151 < area && area < 100000)) {
            potential_char_rects.push_back(bounding_rect);
        }
    }

    // Ordenar los rectángulos de izquierda a derecha (por coordenada x)
    std::sort(potential_char_rects.begin(), potential_char_rects.end(),
              [](const cv::Rect& a, const cv::Rect& b) {
                  return a.x < b.x;
              });

    // Recortar y redimensionar caracteres detectados
    for (const auto& rect : potential_char_rects) {
        cv::Mat char_crop = input_mat(rect);
        cv::resize(char_crop, char_crop, cv::Size(20, 32));

        if (!char_crop.isContinuous()) {
            char_crop = char_crop.clone();
        }

        character_images.push_back(char_crop);
    }

    int64_t end_time = esp_timer_get_time();
    ESP_LOGI(PIPELINE_TAG, "Tiempo de encontrar contornos de caracteres (Paso 10): %.6f s", 
             (end_time - start_time) / 1000000.0);
}

bool is_letter_for_plate_format(size_t char_index, bool is_new_format) {
    if (is_new_format) {
        // Formato nuevo: LL-NNN-LL
        return (char_index < 2 || char_index > 4); // Las primeras 2 y las últimas 2 son letras
    } else {
        // Formato viejo: LLL-NNN
        return (char_index < 3); // Las primeras 3 son letras, el resto números
    }
}

void process_character_inference(bool is_letter, uint8_t* image_buffer, size_t char_index, char* predicted_result) {
    
    uint64_t start_time = esp_timer_get_time();
    run_model(is_letter, image_buffer, predicted_result); 
    uint64_t end_time = esp_timer_get_time();
    ESP_LOGI("TF-MODEL", "Tiempo total de ejecución del carácter %zu: %.6f segundos", 
        char_index + 1, (end_time - start_time) / 1000000.0);
}

void log_memory() {
    size_t free_heap_before = xPortGetFreeHeapSize();
    size_t free_spiram_before = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

    // Mostrar los resultados
    ESP_LOGI("Memory Monitor", "Memoria antes de aplicar filtros:");
    ESP_LOGI("Memory Monitor", "Free Heap: %u bytes", free_heap_before);
    ESP_LOGI("Memory Monitor", "Free SPIRAM: %u bytes", free_spiram_before);
}

extern "C" void run_pipeline(uint8_t* input_data, size_t input_size, const char* format) {
    ESP_LOGI(PIPELINE_TAG, "Iniciando procesamiento de filtros");
    log_memory();

    // Comprobar si los datos comprimidos son válidos
    if (input_data == nullptr || input_size == 0 || format == nullptr) {
        ESP_LOGE(PIPELINE_TAG, "Datos comprimidos o formato inválidos");
        return;
    }

    //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
    // Comprobar el formato del archivo
    cv::Mat input_mat = load_image_by_format(format, input_data, input_size);
    //ESP_ERROR_CHECK( heap_trace_stop() );
    //heap_trace_dump();

    //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
    // Paso 1
    apply_grayscale(input_mat);
    //ESP_ERROR_CHECK( heap_trace_stop() );
    //heap_trace_dump();

    //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
    // Paso 2
    resize_image(input_mat, 450);
    //ESP_ERROR_CHECK( heap_trace_stop() );
    //heap_trace_dump();

    //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
    // Paso 3
    cv::Mat gaussian_mat;
    apply_gaussian_blur(input_mat, gaussian_mat);
    //ESP_ERROR_CHECK( heap_trace_stop() );
    //heap_trace_dump();

    //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
    // Paso 4
    cv::Mat input_mat_copy;
    apply_bilateral_filter(gaussian_mat, input_mat_copy);
    //ESP_ERROR_CHECK( heap_trace_stop() );
    //heap_trace_dump();

    //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
    // Paso 5
    apply_canny_edge_detection(input_mat_copy);
    //ESP_ERROR_CHECK( heap_trace_stop() );
    //heap_trace_dump();

    //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
    // Paso 6
    apply_dilation(input_mat_copy);
    //ESP_ERROR_CHECK( heap_trace_stop() );
    //heap_trace_dump();

    //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
    // Paso 7
    Mat best_candidate_mat = find_license_plate_candidate(input_mat_copy);
    //ESP_ERROR_CHECK( heap_trace_stop() );
    //heap_trace_dump();

    // Vector para almacenar las imágenes en memoria
    std::vector<cv::Mat> character_images;
    std::vector<cv::Rect> potential_char_rects;
    
    if (!best_candidate_mat.empty()) {

        //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
        // Paso 8
        apply_erosion(best_candidate_mat);
        //ESP_ERROR_CHECK( heap_trace_stop() );
        //heap_trace_dump();

        //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
        // Paso 9
        apply_close(best_candidate_mat);
        //ESP_ERROR_CHECK( heap_trace_stop() );
        //heap_trace_dump();
    
        //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
        // Paso 10
        find_characters_candidate(best_candidate_mat, character_images, potential_char_rects);
    } else {
        ESP_LOGI(PIPELINE_TAG, "No se encontró ningún rectángulo adecuado");
    }
    //ESP_ERROR_CHECK( heap_trace_stop() );
    //heap_trace_dump();

    // Determinar el formato de la patente (vieja o nueva)
    size_t total_chars = character_images.size();
    ESP_LOGI(PIPELINE_TAG, "Cantidad de caracteres encontrados: %d", total_chars);
    bool is_new_format = (total_chars == 7); // 7 caracteres indican formato nuevo
    std::vector<char> predictions; 

    for (size_t i = 0; i < character_images.size(); ++i) {
        const cv::Mat& char_image = character_images[i];
        uint8_t* image_buffer = char_image.data;

        // Determinar si es letra o número según el formato de la patente
        bool is_letter = is_letter_for_plate_format(i, is_new_format);

        //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
        char predicted_result;
        process_character_inference(is_letter, image_buffer, i, &predicted_result);
        //ESP_ERROR_CHECK( heap_trace_stop() );
        //heap_trace_dump();

        // Almacenar la predicción
        predictions.push_back(predicted_result);     
    }

    // Convertir el vector de predicciones en una cadena
    std::string final_prediction(predictions.begin(), predictions.end());
    ESP_LOGI(PIPELINE_TAG, "Predicción final: %s", final_prediction.c_str());
    log_memory();
}
