#include <stdio.h>
#include <stdlib.h>
#include "haru.h"

uint32_t init_reference(int32_t *ref, uint32_t ref_size, uint32_t pos, int32_t *query, uint32_t query_size) {
    if (query_size > ref_size || pos + query_size > ref_size) {
        return -1;
    }

    uint32_t i;
    int32_t signal[5] = {-2, -1, 0, 1, 2};
    for (i = 0; i < ref_size; i++) {
        ref[i] = signal[i%5];
    }

    for (i = pos; i < pos+query_size; i++) {
        ref[i] = query[i - pos];
    }

    return 0;
}

uint32_t init_query(int32_t *query, uint32_t query_size) {
    uint32_t i;
    int32_t signal[5] = {-2, 1, -1, 1, -2};
    for (i = 0; i < query_size; i++) {
        query[i] = signal[i%5];
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int32_t ret;
    haru_t haru;
    int32_t ref_buffer[0xffff] = {0};
    int32_t query_buffer[251] = {0};

    // Setup query and reference
    init_query(query_buffer+1, 250);

    ret = init_reference(ref_buffer, 0xffff, 0x1000, query_buffer+1, 250);
    if (ret) {
        printf("Error: Failed to initialize reference\n");
        return -1;
    }

    // Initialize haru
    ret = haru_init(&haru);
    if (ret) {
        printf("Error: Failed to initialize haru\n");
        return -1;
    }

    haru_load_reference(&haru, ref_buffer, 0xffff);
    for (int i = 0; i < 10; i++) { // Load 10 queries
        query_buffer[0] = i; // Set query id
        haru_load_query(&haru, query_buffer, 251);
    }

    search_result_t *res = (search_result_t *) malloc(10 * sizeof(search_result_t));
    haru_get_result(&haru, res, 10);

    for (int i = 0; i < 10; i++) {
        printf("QID:%d result -> %d %d %d\n", i, res[i].qid, res[i].position, res[i].score);
    }
    free(res);
    haru_release(&haru);

    return 0;
}