#include "haru.h"

int32_t haru_init(haru_t *haru) {
    uint32_t ret;
    ret = axi_dma_init(&haru->axi_dma, HARU_DTW_ACCEL_ADDR_BASE, HARU_DTW_ACCEL_SIZE);
    if (ret != 0) {
        haru_err = HARU_AXI_DMA_INIT_ERROR;
        return -1;
    }

    ret = dtw_accel_init(&haru->dtw_accel, HARU_DTW_ACCEL_ADDR_BASE, HARU_DTW_ACCEL_SIZE);
    if (ret != 0) {
        haru_err = HARU_DTW_ACCEL_INIT_ERROR;
        return -1;
    }

    return 0;
}

void haru_release(haru_t *haru) {
    dtw_accel_release(&haru->dtw_accel);
    axi_dma_release(&haru->axi_dma);
}

void haru_load_reference(haru_t *haru, uint32_t addr, uint32_t size) {
    uint32_t ret;
    dtw_accel_reset(&haru->dtw_accel);
    dtw_accel_set_mode(&haru->dtw_accel, DTW_ACCEL_MODE_REF_LOAD);
    axi_dma_mm2s_transfer(&haru->axi_dma, addr, size);
    dtw_accel_run(&haru->dtw_accel);
}

void haru_load_query(haru_t *haru, uint32_t addr, uint32_t size) {
    dtw_accel_set_mode(&haru->dtw_accel, DTW_ACCEL_MODE_QUERY);
    dtw_accel_run(&haru->dtw_accel);
    axi_dma_mm2s_transfer(&haru->axi_dma, addr, size);
}

void haru_get_result(haru_t *haru, search_result_t *addr, uint32_t n_res) {
    axi_dma_s2mm_transfer(&haru->axi_dma, (uint32_t) addr, n_res * sizeof(search_result_t));
}