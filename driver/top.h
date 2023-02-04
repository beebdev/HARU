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

#ifndef HARU_H
#define HARU_H

#include <stdint.h>

#include "axi_dma.h"
#include "zynq_dtw.h"

#define HARU_QUERY_LEN        250

struct haru_info {
    struct axi_dma_info axi_dma;
    struct zynq_dtw_info zynq_dtw;
};

struct search_info {
    int32_t *query;
    int32_t *reference;

    uint32_t reference_len;

    uint32_t best_position;
    uint32_t best_score;
};

int haru_init(struct haru_info *haru, 
              uint32_t axi_dma_addr_base, uint32_t axi_dma_size,
              uint32_t zynq_dtw_addr_base, uint32_t zynq_dtw_size,
              uint32_t source_mm2s_addr_base, uint32_t source_mm2s_size);
void haru_release(struct haru_info *haru);

uint32_t haru_get_ip_core_version(struct haru_info *haru);
void haru_print_ip_core_version(struct haru_info *haru);

int haru_set_reference_len(struct haru_info *haru, uint32_t length);
uint32_t haru_get_reference_len(struct haru_info *haru);

int haru_search(struct haru_info *haru, struct search_info *search);


#endif // HARU_H