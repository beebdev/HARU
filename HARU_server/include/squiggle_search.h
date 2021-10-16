// #include "ap_fixed.h"

#ifndef SQUIGGLE_SEARCH
#define SQUIGGLE_SEARCH

#include "ap_cint.h"
// #include <stdint.h>

#define BLOCKSIZE 200000
#define OVERLAP (BLOCKSIZE-500)
#define START 10000
#define END 15000
#define LIBLENGTH 3000
#define BALANCE (LIBLENGTH/2)
#define NREF 1
#define REF_LEN 48497
// #define REF_LEN 4000
#define QUERY_LEN 250

#define ISFLOAT 0

#if ISFLOAT
    typedef float value_t;
#else
    typedef int16_t value_t;
#endif
// typedef ap_fixed<16,2> value_t;

typedef struct search_result_t {
    value_t dist;
    int end_position;
} search_result_t;

typedef struct Path {
    int k;
    int *px;
    int *py;
} Path;

search_result_t subsequence_search(value_t query[QUERY_LEN], value_t ref[REF_LEN]);

#endif
