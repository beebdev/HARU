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
    printf("HARU version: %x\n", version);
    printf("DTW_ACCEL busy: %x\n", dtw_accel_busy(&haru->dtw_accel));
    printf("DTW_ACCEL ref_load_done: %x\n", dtw_accel_ref_load_done(&haru->dtw_accel));
    printf("DTW_ACCEL src_fifo_empty: %x\n", dtw_accel_src_fifo_empty(&haru->dtw_accel));
    printf("DTW_ACCEL src_fifo_full: %x\n", dtw_accel_src_fifo_full(&haru->dtw_accel));
    printf("DTW_ACCEL sink_fifo_empty: %x\n", dtw_accel_sink_fifo_empty(&haru->dtw_accel));
    printf("DTW_ACCEL sink_fifo_full: %x\n", dtw_accel_sink_fifo_full(&haru->dtw_accel));
    printf("DTW_ACCEL state: %x\n", dtw_accel_state(&haru->dtw_accel));
    printf("DTW_ACCEL addrw_ref: %x\n", dtw_accel_addrw_ref(&haru->dtw_accel));
    return 0;
}

void haru_release(haru_t *haru) {
    axi_dma_release(&haru->axi_dma);
    dtw_accel_release(&haru->dtw_accel);
}

void haru_check_key(haru_t *haru) {
    uint32_t key = dtw_accel_get_key(&haru->dtw_accel);
    if (key != 0x0ca7cafe) {
        printf("Error: Invalid key (%x)\n", key);
        return;
    }
    printf("Key is correct (%x)\n", key);
}

uint32_t haru_get_version(haru_t *haru) {
    uint32_t version = dtw_accel_get_version(&haru->dtw_accel);
    return version;
}

void haru_get_load_done(haru_t *haru) {
    uint32_t done = dtw_accel_ref_load_done(&haru->dtw_accel);
    if (done == 0) {
        printf("Error: Load not done\n");
        return;
    }
    printf("Load done\n");
}

void haru_load_reference(haru_t *haru, int32_t *ref, uint32_t size) {
    // Copy reference into src buffer
    memset((void *)haru->axi_dma.v_src_addr, 0, 0xffff);
    memcpy(haru->axi_dma.v_src_addr, ref, size * sizeof(int32_t));
    printf("size: %d\n", size);

    // Reset dtw_accel
    dtw_accel_reset(&haru->dtw_accel);
    dtw_accel_set_mode(&haru->dtw_accel, DTW_ACCEL_MODE_REF_LOAD);
    dtw_accel_set_ref_len(&haru->dtw_accel, size);
    // printf("Ref len: %u\n", dtw_accel_get_ref_len(&haru->dtw_accel));
    // printf("cr: %x\n", dtw_accel_get_cr(&haru->dtw_accel));
    // printf("sr: %x\n", dtw_accel_get_sr(&haru->dtw_accel));
    // printf("DTW_ACCEL busy: %x\n", dtw_accel_busy(&haru->dtw_accel));
    // printf("DTW_ACCEL ref_load_done: %x\n", dtw_accel_ref_load_done(&haru->dtw_accel));
    // printf("DTW_ACCEL src_fifo_empty: %x\n", dtw_accel_src_fifo_empty(&haru->dtw_accel));
    // printf("DTW_ACCEL src_fifo_full: %x\n", dtw_accel_src_fifo_full(&haru->dtw_accel));
    // printf("DTW_ACCEL sink_fifo_empty: %x\n", dtw_accel_sink_fifo_empty(&haru->dtw_accel));
    // printf("DTW_ACCEL sink_fifo_full: %x\n", dtw_accel_sink_fifo_full(&haru->dtw_accel));
    // printf("DTW_ACCEL state: %x\n", dtw_accel_state(&haru->dtw_accel));
    // printf("DTW_ACCEL addrw_ref: %d\n", dtw_accel_addrw_ref(&haru->dtw_accel));
    dtw_accel_run(&haru->dtw_accel);

    axi_dma_mm2s_transfer(&haru->axi_dma, (size-200) * sizeof(uint32_t));
    printf("After ref load: %d (mode=%d)\n",
        dtw_accel_ref_load_done(&haru->dtw_accel),
        (dtw_accel_get_cr(&haru->dtw_accel) >> DTW_ACCEL_CR_OFFSET_MODE) & 0x1
    );
    // printf("Ref len: %u\n", dtw_accel_get_ref_len(&haru->dtw_accel));
    // printf("DTW_ACCEL busy: %x\n", dtw_accel_busy(&haru->dtw_accel));
    // printf("DTW_ACCEL ref_load_done: %x\n", dtw_accel_ref_load_done(&haru->dtw_accel));
    // printf("DTW_ACCEL src_fifo_empty: %x\n", dtw_accel_src_fifo_empty(&haru->dtw_accel));
    // printf("DTW_ACCEL src_fifo_full: %x\n", dtw_accel_src_fifo_full(&haru->dtw_accel));
    // printf("DTW_ACCEL sink_fifo_empty: %x\n", dtw_accel_sink_fifo_empty(&haru->dtw_accel));
    // printf("DTW_ACCEL sink_fifo_full: %x\n", dtw_accel_sink_fifo_full(&haru->dtw_accel));
    // printf("DTW_ACCEL state: %x\n", dtw_accel_state(&haru->dtw_accel));
    // printf("DTW_ACCEL addrw_ref: %d\n", dtw_accel_addrw_ref(&haru->dtw_accel));

    axi_dma_mm2s_transfer(&haru->axi_dma, (200) * sizeof(uint32_t));
    printf("After ref load: %d (mode=%d)\n",
        dtw_accel_ref_load_done(&haru->dtw_accel),
        (dtw_accel_get_cr(&haru->dtw_accel) >> DTW_ACCEL_CR_OFFSET_MODE) & 0x1
    );
    printf("Ref len: %u\n", dtw_accel_get_ref_len(&haru->dtw_accel));
    printf("DTW_ACCEL busy: %x\n", dtw_accel_busy(&haru->dtw_accel));
    printf("DTW_ACCEL ref_load_done: %x\n", dtw_accel_ref_load_done(&haru->dtw_accel));
    printf("DTW_ACCEL src_fifo_empty: %x\n", dtw_accel_src_fifo_empty(&haru->dtw_accel));
    printf("DTW_ACCEL src_fifo_full: %x\n", dtw_accel_src_fifo_full(&haru->dtw_accel));
    printf("DTW_ACCEL sink_fifo_empty: %x\n", dtw_accel_sink_fifo_empty(&haru->dtw_accel));
    printf("DTW_ACCEL sink_fifo_full: %x\n", dtw_accel_sink_fifo_full(&haru->dtw_accel));
    printf("DTW_ACCEL state: %x\n", dtw_accel_state(&haru->dtw_accel));
    printf("DTW_ACCEL addrw_ref: %d\n", dtw_accel_addrw_ref(&haru->dtw_accel));
    printf("========================================================\n");
    // for (unsigned int i = 0; i <size; i++) {
    //     dtw_accel_dbg_addrR_ref(&haru->dtw_accel, i);
    //     printf("MEM[%d]: %d\n", i, dtw_accel_dbg_dout(&haru->dtw_accel));
    // }

}

void haru_process_query(haru_t *haru, int32_t *query, uint32_t size, search_result_t *results) {
    // Copy query into src buffer
    printf("QUERY PROCESSING\n");

    memset((void *)haru->axi_dma.v_src_addr, 0, 0xffff);
    memcpy(haru->axi_dma.v_src_addr, query, size * sizeof (int32_t));
    memset(haru->axi_dma.v_dst_addr, 0, 0xffff);
    // printf("Ref len: %u\n", dtw_accel_get_ref_len(&haru->dtw_accel));
    // printf("CR: %x\n", dtw_accel_get_cr(&haru->dtw_accel));
    // printf("DTW_ACCEL busy: %x\n", dtw_accel_busy(&haru->dtw_accel));
    // printf("DTW_ACCEL ref_load_done: %x\n", dtw_accel_ref_load_done(&haru->dtw_accel));
    // printf("DTW_ACCEL src_fifo_empty: %x\n", dtw_accel_src_fifo_empty(&haru->dtw_accel));
    // printf("DTW_ACCEL src_fifo_full: %x\n", dtw_accel_src_fifo_full(&haru->dtw_accel));
    // printf("DTW_ACCEL sink_fifo_empty: %x\n", dtw_accel_sink_fifo_empty(&haru->dtw_accel));
    // printf("DTW_ACCEL sink_fifo_full: %x\n", dtw_accel_sink_fifo_full(&haru->dtw_accel));
    // printf("DTW_ACCEL state: %x\n", dtw_accel_state(&haru->dtw_accel));
    // printf("DTW_ACCEL addrw_ref: %d\n", dtw_accel_addrw_ref(&haru->dtw_accel));
    // printf("DTW_ACCEL cur_qid: %d\n", dtw_accel_dbg_curr_qid(&haru->dtw_accel));

    printf("========================================================\n");

    dtw_accel_set_mode(&haru->dtw_accel, DTW_ACCEL_MODE_QUERY);
    axi_dma_haru_query_transfer(&haru->axi_dma, size * sizeof(int32_t), sizeof(search_result_t));
    printf("DTW_ACCEL nquery: %d\n", dtw_accel_dbg_nquery(&haru->dtw_accel));
    // printf("Ref len: %u\n", dtw_accel_get_ref_len(&haru->dtw_accel));
    // printf("DTW_ACCEL busy: %x\n", dtw_accel_busy(&haru->dtw_accel));
    // printf("DTW_ACCEL ref_load_done: %x\n", dtw_accel_ref_load_done(&haru->dtw_accel));
    // printf("DTW_ACCEL src_fifo_empty: %x\n", dtw_accel_src_fifo_empty(&haru->dtw_accel));
    // printf("DTW_ACCEL src_fifo_full: %x\n", dtw_accel_src_fifo_full(&haru->dtw_accel));
    // printf("DTW_ACCEL sink_fifo_empty: %x\n", dtw_accel_sink_fifo_empty(&haru->dtw_accel));
    // printf("DTW_ACCEL sink_fifo_full: %x\n", dtw_accel_sink_fifo_full(&haru->dtw_accel));
    // printf("DTW_ACCEL state: %x\n", dtw_accel_state(&haru->dtw_accel));
    // printf("DTW_ACCEL addrw_ref: %d\n", dtw_accel_addrw_ref(&haru->dtw_accel));
    // printf("DTW_ACCEL addrr_ref: %d\n", dtw_accel_addrr_ref(&haru->dtw_accel));
    // printf("DTW_ACCEL nquery: %d\n", dtw_accel_dbg_nquery(&haru->dtw_accel));
    // printf("DTW_ACCEL cur_qid: %d\n", dtw_accel_dbg_curr_qid(&haru->dtw_accel));
    memcpy(results, haru->axi_dma.v_dst_addr, sizeof(search_result_t)); // * n_res);
    printf("res.qid: %u\n", results[0].qid);
    printf("res.position: %u\n", results[0].position);
    printf("res.score: %u\n", results[0].score);

}