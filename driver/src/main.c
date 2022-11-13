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
#include "haru.h"
#include "haru_test.h"

#define SRC_BUFFER_ADDR 0x10000000
#define DST_BUFFER_ADDR 0x20000000
#define REFERENCE_SIZE 10000
#define QUERY_SIZE 250
#define QUERY_LOCATION 500

int main(int argc, char *argv[]) {
    int32_t ret;
    
    haru_t haru;
    ret = haru_init(&haru);
    if (ret != 0) {
        printf("haru_init failed\n");
        return -1;
    }

    int32_t ref[REFERENCE_SIZE];
    int32_t query[QUERY_SIZE + 2];
    search_result_t results;

    memset(ref, 0, sizeof(ref));
    memset(query, 0, sizeof(query));

    for (int i = 0; i < REFERENCE_SIZE; i++) {
        ref[i] = rand() % 100;
    }

    for (int i = 2; i < QUERY_SIZE + 2; i++) {
        query[i] = ref[QUERY_LOCATION + i];
    }

    printf("load reference\n");
    if (haru_load_reference(&haru, ref, REFERENCE_SIZE)) {
        printf("Load done: %x\n", dtw_accel_ref_load_done(&haru.dtw_accel));
        haru_process_query(&haru, query, QUERY_SIZE+2, &results);
        printf("results:\n\tqid:%d\n\tposition: %d\n\tscore: %d\n", results.qid, results.position, results.score);
    } else {
        printf("Load not done\n");
    }

    haru_release(&haru);
    return 0;
}