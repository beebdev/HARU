#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "haru.h"
#include "haru_test.h"

#define SRC_BUFFER_ADDR 0x10000000
#define DST_BUFFER_ADDR 0x20000000

// uint32_t init_reference(int32_t *ref, uint32_t ref_size, uint32_t pos, int32_t *query, uint32_t query_size) {
//     if (query_size > ref_size || pos + query_size > ref_size) {
//         return -1;
//     }

//     uint32_t i;
//     int32_t signal[5] = {-2, -1, 0, 1, 2};
//     for (i = 0; i < ref_size; i++) {
//         ref[i] = signal[i%5];
//     }

//     for (i = pos; i < pos+query_size; i++) {
//         ref[i] = query[i - pos];
//     }

//     return 0;
// }

// uint32_t init_query(int32_t *query, uint32_t query_size) {
//     uint32_t i;
//     int32_t signal[5] = {-2, 1, -1, 1, -2};
//     for (i = 0; i < query_size; i++) {
//         query[i] = signal[i%5];
//     }

//     return 0;
// }

int main(int argc, char *argv[]) {
    int32_t ret;
    
    haru_t haru;
    ret = haru_init(&haru);
    if (ret != 0) {
        printf("haru_init failed\n");
        return -1;
    }

    int32_t ref[2000];
    int32_t query[252];
    search_result_t results[10];

    memset(ref, 0, sizeof(ref));
    memset(query, 0, sizeof(query));

    for (int i = 0; i < 2000; i++) {
        ref[i] = i;
        // printf("ref[%d] = %d (%p)\n", i, ref[i], &ref[i]);
    }

    for (int i = 0; i < 252; i++) {
        query[i] = 400 + i;
    }

    // init_query(query+2, 250); // init after quid
    // ret = init_reference(ref, 2000, 0, query, 250);
    // if (ret != 0) {
    //     printf("init_reference failed\n");
    //     haru_release(&haru);
    //     return -1;
    // }
    // for (int i = 0; i < 2000; i++) {
    //     printf("%d ", ref[i]);
    // }
    // printf("\n");

    // for (int i = 0; i < 252; i++) {
    //     printf("%d ", query[i]);
    // }

    printf("load reference\n");
    haru_load_reference(&haru, ref, 2000);
    printf("Load done: %x\n", dtw_accel_ref_load_done(&haru.dtw_accel));
    // haru_get_load_done(&haru);

    haru_process_query(&haru, query, 252, results);

    haru_release(&haru);
    return 0;
}