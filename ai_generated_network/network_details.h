/**
  ******************************************************************************
  * @file    network.h
  * @date    2026-08-21T15:44:22+0300
  * @brief   ST.AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */
#ifndef STAI_NETWORK_DETAILS_H
#define STAI_NETWORK_DETAILS_H

#include "stai.h"
#include "layers.h"

const stai_network_details g_network_details = {
  .tensors = (const stai_tensor[14]) {
   { .size_bytes = 3136, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 28, 28, 1}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "input_1_output" },
   { .size_bytes = 12544, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 14, 14, 16}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "conv2d_output" },
   { .size_bytes = 12544, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 14, 14, 16}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "re_lu_output" },
   { .size_bytes = 12544, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 14, 14, 16}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "depthwise_conv2d_output" },
   { .size_bytes = 12544, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 14, 14, 16}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "re_lu_1_output" },
   { .size_bytes = 25088, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 14, 14, 32}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "conv2d_1_output" },
   { .size_bytes = 25088, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 14, 14, 32}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "re_lu_2_output" },
   { .size_bytes = 6272, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 7, 7, 32}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "depthwise_conv2d_1_output" },
   { .size_bytes = 6272, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 7, 7, 32}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "re_lu_3_output" },
   { .size_bytes = 12544, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 7, 7, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "conv2d_2_output" },
   { .size_bytes = 12544, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 7, 7, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "re_lu_4_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 1, 1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "global_average_pooling2d_pool_output" },
   { .size_bytes = 144, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 36}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "dense_dense_output" },
   { .size_bytes = 144, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 36}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "dense_output" }
  },
  .nodes = (const stai_node_details[13]){
    {.id = 2, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){0}}, .output_tensors = {1, (const int32_t[1]){1}} }, /* conv2d */
    {.id = 3, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){1}}, .output_tensors = {1, (const int32_t[1]){2}} }, /* re_lu */
    {.id = 5, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){2}}, .output_tensors = {1, (const int32_t[1]){3}} }, /* depthwise_conv2d */
    {.id = 6, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){3}}, .output_tensors = {1, (const int32_t[1]){4}} }, /* re_lu_1 */
    {.id = 8, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){4}}, .output_tensors = {1, (const int32_t[1]){5}} }, /* conv2d_1 */
    {.id = 9, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){5}}, .output_tensors = {1, (const int32_t[1]){6}} }, /* re_lu_2 */
    {.id = 11, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){6}}, .output_tensors = {1, (const int32_t[1]){7}} }, /* depthwise_conv2d_1 */
    {.id = 12, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){7}}, .output_tensors = {1, (const int32_t[1]){8}} }, /* re_lu_3 */
    {.id = 14, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){8}}, .output_tensors = {1, (const int32_t[1]){9}} }, /* conv2d_2 */
    {.id = 15, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){9}}, .output_tensors = {1, (const int32_t[1]){10}} }, /* re_lu_4 */
    {.id = 16, .type = AI_LAYER_POOL_TYPE, .input_tensors = {1, (const int32_t[1]){10}}, .output_tensors = {1, (const int32_t[1]){11}} }, /* global_average_pooling2d_pool */
    {.id = 18, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){11}}, .output_tensors = {1, (const int32_t[1]){12}} }, /* dense_dense */
    {.id = 18, .type = AI_LAYER_SM_TYPE, .input_tensors = {1, (const int32_t[1]){12}}, .output_tensors = {1, (const int32_t[1]){13}} } /* dense */
  },
  .n_nodes = 13
};
#endif

