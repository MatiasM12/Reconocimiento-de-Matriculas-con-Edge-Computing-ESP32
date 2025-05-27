#ifndef IMAGE_PROVIDER_H
#define IMAGE_PROVIDER_H

#include <stdint.h>
#include "esp_err.h"

void load_image_from_sd(const char* directory_path, uint8_t** output_data, size_t* output_size);
void capture_image_from_camera(uint8_t** output_data, size_t* output_size);

#endif // IMAGE_PROVIDER_H
