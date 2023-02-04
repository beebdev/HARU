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
#include <string.h>

#include "top.h"

#define AXI_DMA_BASEADDR        0xA0000000
#define AXI_DMA_SIZE            0xFFFF
#define ZYNQ_DTW_BASEADDR       0xA0010000
#define ZYNQ_DTW_SIZE           0xFFFF

#define REFERENCE_SIZE 10000
#define QUERY_SIZE 250
#define QUERY_LOCATION 500

int main(int argc, char *argv[]) {
    int ret;

    /* Two arguments: physical address of buffer and size of buffer */
    if (argc != 3) {
        printf("Usage: %s <physical address> <size>\n", argv[0]);
        return -1;
    }

    uint32_t buffer_addr = strtoul(argv[1], NULL, 16);
    uint32_t buffer_size = strtoul(argv[2], NULL, 16);
    fprintf(stderr, "Buffer address: 0x%08x\n", buffer_addr);
    fprintf(stderr, "Buffer size: 0x%08x\n", buffer_size);
    
    struct haru_info haru;
    ret = haru_init(&haru, AXI_DMA_BASEADDR, AXI_DMA_SIZE,
                           ZYNQ_DTW_BASEADDR, ZYNQ_DTW_SIZE,
                           buffer_addr, buffer_size);
    if (ret != 0) {
        printf("haru_init failed\n");
        return -1;
    }

    int32_t ref[REFERENCE_SIZE];
    int32_t query[QUERY_SIZE + 2];

    memset(ref, 0, sizeof(ref));
    memset(query, 0, sizeof(query));

    for (int i = 0; i < REFERENCE_SIZE; i++) {
        ref[i] = rand() % 100;
    }

    for (int i = 2; i < QUERY_SIZE + 2; i++) {
        query[i] = ref[QUERY_LOCATION + i];
    }

    struct search_info search = {
        .query = query,
        .reference = ref,
        .reference_len = REFERENCE_SIZE,
        .best_position = 0,
        .best_score = 0,
    };

    if (haru_search(&haru, &search) != 0) {
        printf("haru_search failed\n");
        haru_release(&haru);
        return -1;
    }

    printf("best position: %d\n", search.best_position);
    printf("best score: %d\n", search.best_score);


    haru_release(&haru);
    return 0;
}