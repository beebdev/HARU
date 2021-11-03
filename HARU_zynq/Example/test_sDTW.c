/***********************************************************************
 * FILENAME: test_sDTW.c
 * DESCRIPTION:
 *      Example program for running the sDTW accelerator in HARU. It calls
 *      the accelerator on predefined reference and query and compares the
 *      obtained results with software executed results.
 * NOTES:
 *
 * AUTHOR:    Elton Shih        START DATE :    28/09/2021
 * CHANGES:
 *
 * VERSION DATE       WHO     DETAIL
 * v0.01   29/09/2021 ES      Init Draft version
 *
 **/

#include <stdio.h>
#include <sys/time.h>
#include "haru/haru.h"
#include "reference.h"

//typedef int16_t value_t;
#define VALUE_INF (1 << 15) - 1
//#define VALUE_INF 10000

value_t min3(value_t a, value_t b, value_t c) {
    value_t min = a;
    if (b < min) {
        min = b;
    }
    if (c < min) {
        min = c;
    }
    return min;
}

void sDTW(value_t x[QUERY_LEN], value_t y[REF_LEN]) {
    value_t cost[QUERY_LEN];

    uint dist = VALUE_INF;
    int end_position = 0;

    /* Init cost column */
    for (int r = 0; r < QUERY_LEN; r++) {
        cost[r] = VALUE_INF;
    }

    /* Compute cost matrix */
    value_t top_left = 0;
    value_t top = 0;
    value_t left;

    for (int c = 0; c < REF_LEN; c++) {
        for (int r = 0; r < QUERY_LEN; r++) {
            left = cost[r];
            cost[r] = abs(x[r] - y[c]) + min3(top, left, top_left);

            // Update dependancy
            top_left = left;
            top = cost[r];
        }

        // Dependancy for new col
        top_left = 0;
        top = 0;

        // Update minimum
        if (cost[QUERY_LEN - 1] < dist) {
            dist = cost[QUERY_LEN - 1];
            end_position = c;
        }
    }
    printf("sw dist: %d\n", dist);
    printf("sw pos: %d\n", end_position);
}

int main() {
	struct timeval t1, t2;
	double sw_duration = 0.0;
	double hw_duration = 0.0;

	/* Software version */
	gettimeofday(&t1, NULL);
	printf("SW version\n");
    sDTW(strand_sample, reference[0]);
    printf("-----\n");
    sDTW(strand_sample, reference[1]);
	gettimeofday(&t2, NULL);
	sw_duration += (t2.tv_sec - t1.tv_sec) * 1000;
	sw_duration += (t2.tv_usec - t1.tv_usec) / 1000;

    printf("======================================\n");

    /* Hardware version */
    printf("HW version:\n");
    haru_t my_haru;
    if (haru_init(&my_haru)){
    	fprintf(stderr, "Error initialising haru\n");
    	return -1;
    }

    query_t my_query;
    for (int i = 0; i < 200; i++) {
        gettimeofday(&t1, NULL);
        query_init(&my_query, 0);
        haru_process_query(&my_haru, strand_sample, &my_query);
        gettimeofday(&t2, NULL);
        hw_duration += (t2.tv_sec - t1.tv_sec) * 1000;
        hw_duration += (t2.tv_usec - t1.tv_usec) / 1000;
    }

    printf("HW: [%d]@%d (%d)\n", my_query.ID, my_query.position, my_query.min_dist);
    printf("SW_time: %lf ms\nHw_time: %lf ms\n", sw_duration, hw_duration);

    /////////////////////
    /* Clean up */
    if (haru_cleanup(&my_haru)) {
    	fprintf(stderr, "Error cleaning up HARU\n");
    	return -1;
    }

    printf("exiting..\n====================\n");

    return 0;
}
