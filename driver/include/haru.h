/* MIT License

Copyright (c) 2022 Po Jui Shih
Copyright (c) 2022 Hassaan Saadat
Copyright (c) 2022 Sri Parameswaran
Copyright (c) 2022 Hasindu Gamaarachchi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#ifndef HARU_H
#define HARU_H

#include "axi_dma.h"
#include "dtw_accel.h"

#include <stdint.h>

#define HARU_AXI_DMA_ADDR_BASE      0xa0000000
#define HARU_AXI_DMA_SIZE           0xffff
#define HARU_DTW_ACCEL_ADDR_BASE    0xa0010000
#define HARU_DTW_ACCEL_SIZE         0xffff

#define HARU_AXI_DMA_INIT_ERROR     0x01
#define HARU_DTW_ACCEL_INIT_ERROR   0x02

#define HARU_AXI_SRC_ADDR           0x10000000
#define HARU_AXI_DST_ADDR           0x20000000

#define HARU_AXI_BUFFER_SIZE        0xffff

#define HARU_AXIS_BATCH_MAX_SIZE    0x0fff

typedef struct {
    dtw_accel_t dtw_accel;
    axi_dma_t axi_dma;
} haru_t;

typedef struct {
    uint32_t qid;
    uint32_t position;
    uint32_t score;
} search_result_t;

int32_t haru_init(haru_t *haru);
void haru_release(haru_t *haru);
void haru_check_key(haru_t *haru);
uint32_t haru_get_version(haru_t *haru);
void haru_get_load_done(haru_t *haru);

int32_t haru_load_reference(haru_t *haru, int32_t *ref, uint32_t size);
void haru_process_query(haru_t *haru, int32_t *query, uint32_t size, search_result_t *results);

#endif // HARU_H