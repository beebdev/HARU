#include <stdio.h>
#include "haru.h"
#include "../axi_dma/axi_dma.h"
#include "../sDTW_accel/xsubseek_dtw.h"

const char* RefInstance = "reference_loader_0";
const char* sDTWInstance = "subseek_dtw_0";

int haru_init(haru_t* haru) {
    /* Query AXI DMA init */
    haru->query_dma = axi_dma_init(AXI_DMA0_ADDR);
    dma_set(haru->query_dma, MM2S_CONTROL_REGISTER, 4); // reset
    dma_set(haru->query_dma, MM2S_CONTROL_REGISTER, 0); // halt
    printf("Initialised Query AXI DMA\n");

    /* sDTW init */
    int status = XSubseek_dtw_Initialize(&haru->sdtw, sDTWInstance);
    if (status != XST_SUCCESS) {
        fprintf(stderr, "Error initialising subseek dtw (err=%d)\n", status);
        return -1;
    }
    printf("Initialised subseek dtw\n");

    /* mmap physical address 0x0e000000 to virtual address */
    int dev_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dev_fd == -1) {
        printf("Error\n");
        return -1;
    }
    haru->buffer_vaddr =
        mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, BUFFER_PADDR);
    close(dev_fd);
    printf("Mapped 0x0e000000 to virtual address\n");

    printf("============================\n");
    return 0;
}

int haru_cleanup(haru_t* haru) {
    /* Unmap AXI DMA addresses */
    axi_dma_release(haru->query_dma);

    /* Release sDTW accelerator */
    int status = XSubseek_dtw_Release(&haru->sdtw);
    if (status != XST_SUCCESS) {
        fprintf(stderr, "Error releasing Subseek DTW\n");
        return -1;
    }

    /* Unmap buffer virtual address */
    munmap(haru->buffer_vaddr, BUFFER_SIZE);

    printf("Finished cleanup\n");
    printf("============================\n");
    return 0;
}

int query_init(query_t* query, int ID) {
    query->ID = ID;
    query->position = 0;
    query->min_dist = 0;
    return 0;
}

//void haru_load_reference(haru_t* haru, value_t* reference) {
//    printf("Load reference\n");
//
//    /* Start Reference Loader */
//    XReference_loader_Start(&haru->ref_loader);
//
//    /* The reference size would be too long to store in the virtually
//     * mapped address, we send the reference in chuncks by copying segment
//     * by segment */
//
//    printf("Streaming reference\n");
//    long int transfer_len = 0;
//    char* ref_ptr = (char*)reference;
//    for (int i = REF_LEN * sizeof(value_t); i > 0; i -= 0xffff) {
//        int len = (i < 0xffff) ? i : 0xffff;
//        memcpy(haru->buffer_vaddr, ref_ptr, len);
//
//        /* Send vbuffer through axis */
//        axi_mm2s_transfer(haru->ref_dma, BUFFER_PADDR, len);
//        while (dma_mm2s_busy(haru->ref_dma))
//            ;
//
//        dma_mm2s_status(haru->ref_dma);
//        /* Increment reference pointer to move along the reference */
//        ref_ptr += len;
//        transfer_len += len;
//    }
//    printf("Reference streamed (%ld)\n", transfer_len);
//
//    /* Wait for sDTW Accelerator to finish */
//    printf("Waiting for ref loader to finish\n");
//    int status;
//    do {
//        status = XReference_loader_IsDone(&haru->ref_loader);
//    } while (!status);
//
//    printf("Finished loading reference to sDTW\n");
//    printf("============================\n");
//}

void haru_process_query(haru_t* haru, value_t* query_addr, query_t* results) {
    printf("Load query\n");

    /* Copy over data to buffer */
    memcpy(haru->buffer_vaddr, query_addr, QUERY_LEN * sizeof(value_t));

    /* Start SubseekDTW Accelerator */
    XSubseek_dtw_Start(&haru->sdtw);

    /* Send query */
    axi_mm2s_transfer(haru->query_dma, BUFFER_PADDR, QUERY_LEN * sizeof(value_t));
    while (dma_mm2s_busy(haru->query_dma))
        ;

    /* Wait for sDTW Accelerator to finish */
    printf("Waiting for sDTW to finish\n");
    int status;
    do {
        status = XSubseek_dtw_IsDone(&haru->sdtw);
    } while (!status);

    printf("Query completed\n");
    results->ID = XSubseek_dtw_Get_agg_result_ID(&haru->sdtw);
    results->min_dist = XSubseek_dtw_Get_agg_result_dist(&haru->sdtw);
    results->position = XSubseek_dtw_Get_agg_result_pos(&haru->sdtw);
    printf("============================\n");
}
