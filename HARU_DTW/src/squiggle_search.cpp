#include <stdio.h>
#include <math.h>
#include "../include/squiggle_search.h"

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

search_result_t subsequence_search(value_t x[QUERY_LEN], value_t y[REF_LEN]) {
    search_result_t res;
    value_t cost_x[QUERY_LEN];
    value_t min_dist;
    int min_position;

    /* First column of cost */
    cost_x[0] = abs(x[0] - y[0]);
    sdtw_col_0_label: for (int r = 1; r < QUERY_LEN; r++) {
        cost_x[r] = abs(x[r] - y[0]) + cost_x[r-1];
    }

    res.dist = cost_x[QUERY_LEN - 1];
    res.end_position = 0;

    /* The rest */
    sdtw_col_j_label: for (int j = 1; j < REF_LEN; j++) {
        value_t top_left = cost_x[0];
        value_t left = cost_x[1];
        value_t top = abs(x[0] - y[j]);

        /* First row */
        cost_x[0] = top;

        /* The rest */
        sdtw_row_i_label: for (int i = 1; i < QUERY_LEN - 1; i++) {
            cost_x[i] = abs(x[i] - y[j]) + min3(top, left, top_left);
            top_left = left;
            left = cost_x[i+1];
            top = cost_x[i];
        }

        /* Last row */
        cost_x[QUERY_LEN - 1] = abs(x[QUERY_LEN - 1] - y[j]) + min3(top, left, top_left);
        if (cost_x[QUERY_LEN - 1] < res.dist) {
        	res.dist = cost_x[QUERY_LEN - 1];
        	res.end_position = j;
        }
    }

    return res;
}

// function subseekDTW(x[QUERY_LEN], y[REF_LEN]) {
//     cost[QUERY_LEN] stores 1 column of cost

//     // Compute first column
//     cost[0] = abs(x[0] - y[0]);
//     for (r = 1; r < QUERY_LEN; r++) {
//         cost[r] = abs(x[r] - y[0]) + cost[r-1];
//     }
    
//     min = cost[QUERY_LEN-1];
//     min_position = 0;

//     // Compute all other columns
//     for (c = 1; c < REF_LEN; c++) {
//         top_left = cost[0];
//         left = cost[1];
//         top = abs(x[0] - y[c]);
//         cost[0] = top;

//         // All rows of current column
//         for (r = 1; r < QUERY_LEN; r++) {
//             cost[r] = abs(x[r] - y[c]) + min(top_left, left, top);
            
//             // Update dependancy
//             top_left = left;
//             top = cost[r];
//             left = cost[r+1];
//         }

//         // Update minimum
//         if (cost[QUERY_LEN-1] < min) {
//             min = cost[QUERY_LEN-1];
//             min_position = c;
//         }
//     }
    
//     return min_position
// }