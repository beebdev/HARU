#include <stdio.h>
#include <haru.h>

uint32_t init_reference(uint32_t *ref, uint32_t ref_size, uint32_t pos, uint32_t *query, uint32_t query_size) {
    if (query_size > ref_size || pos + query_size > ref_size) {
        return -1;
    }

    uint32_t i;
    uint32_t signal[5] = {-2, -1, 0, 1, 2};
    for (i = 0; i < ref_size; i++) {
        ref[i] = signal[i%5];
    }

    for (i = pos; i < pos+query_size; i++) {
        ref[i] = query[i - pos];
    }

    return 0;
}

uint32_t init_query(uint32_t *query, uint32_t query_size) {
    uint32_t i;
    uint32_t signal[5] = {-2, 1, -1, 1, -2};
    for (i = 0; i < query_size; i++) {
        query[i] = signal[i%5];
    }

    return 0;
}

int main(int argc, char *argv[]) {
    haru_t haru;
    uint32_t ref_buffer[0xffff] = {0};
    uint32_t query_buffer[251] = {0};
    search_result_t result[10] = {0};

    // Setup query and reference
    init_query(query_buffer+1, 250);
    if (init_reference(ref_buffer, 0xffff, 0x0123, query_buffer+1, 250) {
        printf("Error: Failed to initialize reference\n");
        return -1;
    }

    // Initialize haru
    if (haru_init(&haru)) {
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