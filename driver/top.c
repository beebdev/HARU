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
#include <stdint.h>
#include <string.h>

#include "top.h"

int haru_init(struct haru_info *haru, 
              uint32_t axi_dma_addr_base, uint32_t axi_dma_size,
              uint32_t zynq_dtw_addr_base, uint32_t zynq_dtw_size,
              uint32_t source_mm2s_addr_base, uint32_t source_mm2s_size) {
    if (haru == NULL) {
        fprintf(stderr, "%s: invalid haru_info pointer\n", __func__);
        return -1;
    }

    if (axi_dma_addr_base == 0) {
        fprintf(stderr, "%s: invalid axi_dma_addr_base\n", __func__);
        return -1;
    }

    if (axi_dma_size == 0) {
        fprintf(stderr, "%s: invalid axi_dma_size\n", __func__);
        return -1;
    }

    if (zynq_dtw_addr_base == 0) {
        fprintf(stderr, "%s: invalid zynq_dtw_addr_base\n", __func__);
        return -1;
    }

    if (zynq_dtw_size == 0) {
        fprintf(stderr, "%s: invalid zynq_dtw_size\n", __func__);
        return -1;
    }

    if (source_mm2s_addr_base == 0) {
        fprintf(stderr, "%s: invalid source_mm2s_addr_base\n", __func__);
        return -1;
    }

    if (source_mm2s_size == 0) {
        fprintf(stderr, "%s: invalid source_mm2s_size\n", __func__);
        return -1;
    }

    haru->axi_dma.phys_baseaddr = axi_dma_addr_base;
    haru->axi_dma.device_size = axi_dma_size;
    haru->axi_dma.phys_src_buf_addr = source_mm2s_addr_base;
    haru->axi_dma.src_buf_size = source_mm2s_size;
    haru->zynq_dtw.phys_baseaddr = zynq_dtw_addr_base;
    haru->zynq_dtw.device_size = zynq_dtw_size;


    if (axi_dma_init(&haru->axi_dma) < 0) {
        fprintf(stderr, "%s: axi_dma_init failed\n", __func__);
        return -1;
    }

    if (zynq_dtw_init(&haru->zynq_dtw) < 0) {
        fprintf(stderr, "%s: zynq_dtw_init failed\n", __func__);
        axi_dma_release(&haru->axi_dma);
        return -1;
    }

    int key = zynq_dtw_get_key(&haru->zynq_dtw);
    if (key != 0x0ca7cafe) {
        fprintf(stderr, "%s: zynq_dtw_get_key failed\n", __func__);
        axi_dma_release(&haru->axi_dma);
        zynq_dtw_release(&haru->zynq_dtw);
        return -1;
    }

    return 0;
}

void haru_release(struct haru_info *haru) {
    if (haru == NULL) {
        fprintf(stderr, "%s: invalid haru_info pointer\n", __func__);
        return;
    }

    zynq_dtw_release(&haru->zynq_dtw);
    axi_dma_release(&haru->axi_dma);
}

uint32_t haru_get_ip_core_version(struct haru_info *haru) {
    if (haru == NULL) {
        fprintf(stderr, "%s: invalid haru_info pointer\n", __func__);
        return 0;
    }

    return zynq_dtw_get_version(&haru->zynq_dtw);
}

void haru_print_ip_core_version(struct haru_info *haru) {
    uint32_t version, major, minor, revision;

    if (haru == NULL) {
        fprintf(stderr, "%s: invalid haru_info pointer\n", __func__);
        return;
    }

    // `define MAJOR_RANGE         31:28
    // `define MINOR_RANGE         27:20
    // `define REVISION_RANGE      19:16
    // `define VERSION_PAD_RANGE   15:0
    version = zynq_dtw_get_version(&haru->zynq_dtw);
    major = (version >> 28) & 0xf;
    minor = (version >> 20) & 0xff;
    revision = (version >> 16) & 0xf;
    printf("IP core version: %d.%d.%d\n", major, minor, revision);
}

int haru_set_reference_len(struct haru_info *haru, uint32_t length) {
    if (haru == NULL) {
        fprintf(stderr, "%s: invalid haru_info pointer\n", __func__);
        return -1;
    }

    if (length <  250) {
        fprintf(stderr, "%s: invalid reference length\n", __func__);
        return -1;
    }

    zynq_dtw_set_ref_len(&haru->zynq_dtw, length);
    return 0;
}

uint32_t haru_get_reference_len(struct haru_info *haru) {
    if (haru == NULL) {
        fprintf(stderr, "%s: invalid haru_info pointer\n", __func__);
        return 0;
    }

    return zynq_dtw_get_ref_len(&haru->zynq_dtw);
}

int haru_search(struct haru_info *haru, struct search_info *search) {
    if (haru == NULL) {
        fprintf(stderr, "%s: invalid haru_info pointer\n", __func__);
        return -1;
    }

    if (search == NULL) {
        fprintf(stderr, "%s: invalid search_info pointer\n", __func__);
        return -1;
    }

    if (search->query == NULL) {
        fprintf(stderr, "%s: invalid query pointer\n", __func__);
        return -1;
    }

    if (search->reference == NULL) {
        fprintf(stderr, "%s: invalid reference pointer\n", __func__);
        return -1;
    }
    
    zynq_dtw_reset(&haru->zynq_dtw);

    zynq_dtw_run(&haru->zynq_dtw);

    uint32_t size_left = HARU_QUERY_LEN;
    int32_t *curr_word = search->query;
    while (size_left > 0) {
        uint32_t transfer_size = size_left > 0x0fff ? 0x0fff : size_left;
        memcpy(haru->axi_dma.virt_baseaddr, (void *) curr_word, transfer_size * sizeof(int32_t));
        axi_dma_mm2s_transfer(&haru->axi_dma, transfer_size * sizeof(int32_t));
        curr_word += transfer_size;
        size_left -= transfer_size;
    }

    size_left = search->reference_len;
    curr_word = search->reference;

    while (size_left > 0) { // the 0x0fff is in words if you get unexpected results, check buffer size in mm2s
        uint32_t transfer_size = size_left > 0x0fff ? 0x0fff : size_left;
        memcpy(haru->axi_dma.virt_baseaddr, (void *) curr_word, transfer_size * sizeof(int32_t));
        axi_dma_mm2s_transfer(&haru->axi_dma, transfer_size * sizeof(int32_t));
        curr_word += transfer_size;
        size_left -= transfer_size;
    }

    /* done, get results */
    if (zynq_dtw_is_done(&haru->zynq_dtw)) {
        search->best_score = zynq_dtw_get_best_score(&haru->zynq_dtw);
        search->best_position = zynq_dtw_get_best_position(&haru->zynq_dtw);
    }

    return 0;
}
