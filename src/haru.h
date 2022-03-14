#ifndef HARU_H
#define HARU_H

#include "axi_dma.h"
#include "dtw_accel.h"

#include <stdint.h>

#define HARU_DTW_ACCEL_ADDR_BASE  0xa0010000
#define HARU_DTW_ACCEL_SIZE       0xffff

#define HARU_AXI_DMA_ADDR_BASE    0xa0000000
#define HARU_AXI_DMA_SIZE         0xffff

#define HARU_AXI_DMA_INIT_ERROR   0x01
#define HARU_DTW_ACCEL_INIT_ERROR 0x02

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

void haru_load_reference(haru_t *haru, int32_t *addr, uint32_t size);
void haru_load_query(haru_t *haru, int32_t *addr, uint32_t size);
void haru_get_result(haru_t *haru, search_result_t *addr, uint32_t n_res);

#endif // HARU_H