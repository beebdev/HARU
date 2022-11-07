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