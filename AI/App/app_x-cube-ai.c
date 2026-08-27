/**
  ******************************************************************************
  * @file    app_x-cube-ai.c
  * @brief   Minimal baremetal AI program body
  ******************************************************************************
  */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "app_x-cube-ai.h"
#include "stai.h"

#if defined ( __ICCARM__ )
#define AI_RAM   _Pragma("location=\"AI_RAM\"")
#elif defined ( __CC_ARM ) || ( __GNUC__ )
#define AI_RAM   __attribute__((section(".AI_RAM")))
#endif

/* Global byte buffer to save instantiated C-model network context */
STAI_NETWORK_CONTEXT_DECLARE(network_context, STAI_NETWORK_CONTEXT_SIZE)

/* Activations buffers */
STAI_ALIGNED(32) 
AI_RAM 
static uint8_t RAM[STAI_NETWORK_ACTIVATION_1_SIZE_BYTES];

/* Input ve Output (Tahmin) Buffer'ları (STAI_FLAG_PREALLOCATED) */
STAI_ALIGNED(32) static uint8_t in_data[STAI_NETWORK_IN_1_SIZE_BYTES];
STAI_ALIGNED(32) static uint8_t out_data[STAI_NETWORK_OUT_1_SIZE_BYTES];

/* Global c-array to handle the activations buffer */
stai_ptr data_activations[] = { RAM };
stai_ptr data_ins[] = { in_data };
stai_ptr data_outs[] = { out_data };

static stai_size in_length, out_length;
static stai_ptr stai_input[STAI_NETWORK_IN_NUM];
static stai_ptr stai_output[STAI_NETWORK_OUT_NUM];

int aiInit(void) {
  stai_return_code ret_code;
  ret_code = stai_runtime_init();
  ret_code = user_stai_network_init(network_context);
  ret_code = stai_network_set_activations(network_context, data_activations, STAI_NETWORK_ACTIVATIONS_NUM);
  
  // Modellerin giriş/çıkış adreslerini (pointer) bizim ayırdığımız dizilere bağla
  ret_code = stai_network_set_inputs(network_context, data_ins, STAI_NETWORK_IN_NUM);
  ret_code = stai_network_set_outputs(network_context, data_outs, STAI_NETWORK_OUT_NUM);
  
  ret_code = stai_network_get_inputs(network_context, stai_input, &in_length);
  ret_code = stai_network_get_outputs(network_context, stai_output, &out_length);
  return (ret_code == STAI_SUCCESS) ? 0 : -1;
}

int aiDeinit(void) {
  stai_network_deinit(network_context);
  stai_runtime_deinit();
  return 0;
}

void STM32CubeAI_Studio_AI_Init(void) {
    aiInit();  
}

void STM32CubeAI_Studio_AI_Process(void) {
    /* Basit Inference çağrısı (HAL veya loglama olmadan) */
    stai_network_run(network_context, STAI_MODE_SYNC);
}

void STM32CubeAI_Studio_AI_Deinit(void) {
    aiDeinit();
}

float* STM32CubeAI_GetInputBuffer(void) {
    return (float*)stai_input[0];
}

float* STM32CubeAI_GetOutputBuffer(void) {
    return (float*)stai_output[0];
}
