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

#include <stdio.h>
#include <stdlib.h>
#include "haru.h"

void test_dtw_accel_key() {
    printf("==================================\n");
    printf("Testing key correctness\n");
    printf("==================================\n");
    int32_t ret;
    haru_t haru;
    ret = haru_init(&haru);
    if (ret) {
        printf("Error: Failed to initialize haru\n");
        return;
    }

    haru_check_key(&haru);
    haru_release(&haru);
}

void test_dtw_accel_init() {
    printf("==================================\n");
    printf("Testing initialization correctness\n");
    printf("==================================\n");
    int32_t ret;
    haru_t haru;
    ret = haru_init(&haru);
    if (ret) {
        printf("Error: Failed to initialize haru\n");
        return;
    }

    haru_release(&haru);
}

void test_dtw_accel_reset() {
    printf("==================================\n");
    printf("Testing reset correctness\n");
    printf("==================================\n");
    int32_t ret;
    haru_t haru;
    ret = haru_init(&haru);
    if (ret) {
        printf("Error: Failed to initialize haru\n");
        return;
    }

    printf("[test_dtw_accel_reset] Resetting dma_mm2s\n");
    dma_mm2s_reset(&haru.axi_dma);
    printf("[test_dtw_accel_reset] Resetting dma_s2mm\n");
    dma_s2mm_reset(&haru.axi_dma);

    printf("[test_dtw_accel_reset] Status\n");
    haru_print_status(&haru);
    haru_release(&haru);
}