/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#ifndef PIPELINE_RUNNER_H
#define PIPELINE_RUNNER_H

#include <stddef.h> // Agrega esta línea
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void run_pipeline(uint8_t* input_data, size_t input_size, const char* format);

#ifdef __cplusplus
}
#endif

#endif // PIPELINE_RUNNER_H
