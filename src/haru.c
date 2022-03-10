#include "haru.h"

int32_t haru_init(haru_t *haru) {
    axi_dma_init(&haru->axi_dma, HARU_DTW_ACCEL_ADDR_BASE, HARU_DTW_ACCEL_SIZE);
    dtw_accel_init(&haru->dtw_accel, HARU_DTW_ACCEL_ADDR_BASE, HARU_DTW_ACCEL_SIZE);
}

void haru_release(haru_t *haru) {
    dtw_accel_release(&haru->dtw_accel);
    axi_dma_release(&haru->axi_dma);
}

int32_t haru_load_reference(haru_t *haru, uint32_t addr, uint32_t size) {
    dtw_accel_reset(&haru->dtw_accel);
    dtw_accel_set_mode(&haru->dtw_accel, DTW_ACCEL_MODE_REF_LOAD);
    axi_dma_mm2s_transfer(&haru->axi_dma, addr, size);
    dtw_accel_run(&haru->dtw_accel);
}

int32_t haru_load_query(haru_t *haru, uint32_t addr, uint32_t size) {
    dtw_accel_set_mode(&haru->dtw_accel, DTW_ACCEL_MODE_QUERY);
    dtw_accel_run(&haru->dtw_accel);
    axi_dma_mm2s_transfer(&haru->axi_dma, addr, size);
}

search_result_t *haru_get_result(haru_t *haru, uint32_t *addr, uint32_t n_res) {
    axi_dma_s2mm_transfer(&haru->axi_dma, (uint32_t) addr, n_res * sizeof(search_result_t));
    return (search_result_t *) addr;
}