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

#include "haru.h"
#include "misc.h"
#include <stdio.h>
#include <string.h>

/*
 * init and release
 */

int32_t haru_init(haru_t *haru) {
    uint32_t ret;

    // Initialize axi_dma
    ret = axi_dma_init(&haru->axi_dma, HARU_AXI_DMA_ADDR_BASE, HARU_AXI_SRC_ADDR, HARU_AXI_DST_ADDR, HARU_AXI_DMA_SIZE);
    if (ret != 0) {
        return -1;
    }

    // Initialize dtw_accel
    ret = dtw_accel_init(&haru->dtw_accel, HARU_DTW_ACCEL_ADDR_BASE, HARU_DTW_ACCEL_SIZE);
    if (ret != 0) {
        return -1;
    }

    haru_check_key(haru);
    uint32_t version = haru_get_version(haru);
    // printf("HARU version: %x\n", version);
    // printf("DTW_ACCEL busy: %x\n", dtw_accel_busy(&haru->dtw_accel));
    // printf("DTW_ACCEL ref_load_done: %x\n", dtw_accel_ref_load_done(&haru->dtw_accel));
    // printf("DTW_ACCEL src_fifo_empty: %x\n", dtw_accel_src_fifo_empty(&haru->dtw_accel));
    // printf("DTW_ACCEL src_fifo_full: %x\n", dtw_accel_src_fifo_full(&haru->dtw_accel));
    // printf("DTW_ACCEL sink_fifo_empty: %x\n", dtw_accel_sink_fifo_empty(&haru->dtw_accel));
    // printf("DTW_ACCEL sink_fifo_full: %x\n", dtw_accel_sink_fifo_full(&haru->dtw_accel));
    // printf("DTW_ACCEL state: %x\n", dtw_accel_state(&haru->dtw_accel));
    // printf("DTW_ACCEL addrw_ref: %x\n", dtw_accel_addrw_ref(&haru->dtw_accel));
    return 0;
}

void haru_release(haru_t *haru) {
    axi_dma_release(&haru->axi_dma);
    dtw_accel_release(&haru->dtw_accel);
}

void haru_check_key(haru_t *haru) {
    uint32_t key = dtw_accel_get_key(&haru->dtw_accel);
    if (key != 0x0ca7cafe) {
        fprintf(stderr, "Error: Invalid key (%x)\n", key);
        return;
    }
    fprintf(stderr, "Key is correct (%x)\n", key);
}

uint32_t haru_get_version(haru_t *haru) {
    uint32_t version = dtw_accel_get_version(&haru->dtw_accel);
    return version;
}

void haru_get_load_done(haru_t *haru) {
    uint32_t done = dtw_accel_ref_load_done(&haru->dtw_accel);
    if (done == 0) {
        fprintf(stderr, "Error: Load not done\n");
        return;
    }
    fprintf(stderr, "Load done\n");
}

int32_t haru_load_reference(haru_t *haru, int32_t *ref, uint32_t size) {

    // Reset dtw_accel
    dtw_accel_reset(&haru->dtw_accel);
    dtw_accel_set_mode(&haru->dtw_accel, DTW_ACCEL_MODE_REF_LOAD);
    dtw_accel_set_ref_len(&haru->dtw_accel, size);
    dtw_accel_run(&haru->dtw_accel);

    uint32_t size_left = size;
    int32_t *curr_ref = ref;
    while (size_left > 0) {
        uint32_t transfer_size = size_left < HARU_AXIS_BATCH_MAX_SIZE ? size_left : HARU_AXIS_BATCH_MAX_SIZE;
        memset((void *) haru->axi_dma.v_src_addr, 0, 0xffff);
        memcpy((void *) haru->axi_dma.v_src_addr, (void *) curr_ref, transfer_size * sizeof(int32_t));
        axi_dma_mm2s_transfer(&haru->axi_dma, (transfer_size) * sizeof(int32_t));
        
        size_left -= transfer_size;
        curr_ref += transfer_size;
    }
    // fprintf(stderr, "ref_addr: %d\n", dtw_accel_addrw_ref(&haru->dtw_accel));
    return dtw_accel_ref_load_done(&haru->dtw_accel);
}

void haru_process_query(haru_t *haru, int32_t *query, uint32_t size, search_result_t *results) {
    // Copy query into src buffer
    memset((void *)haru->axi_dma.v_src_addr, 0, 0xffff);
    memcpy(haru->axi_dma.v_src_addr, query, size * sizeof(int32_t));
    memset(haru->axi_dma.v_dst_addr, 0, 0xffff);

    dtw_accel_set_mode(&haru->dtw_accel, DTW_ACCEL_MODE_QUERY);
    axi_dma_haru_query_transfer(&haru->axi_dma, size * sizeof(int32_t), sizeof(search_result_t));
    memcpy(results, haru->axi_dma.v_dst_addr, sizeof(search_result_t));
}