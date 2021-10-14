#include <stdio.h>
#include <sys/time.h>
#include "squiggle_search.h"

#if ISFLOAT
    #include "reference.h"
#else
    #include "reference_int.h"
#endif

int main() {
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
    search_result_t res = subsequence_search(strand_sample, reference[0]);
    gettimeofday(&t2, NULL);
    double total_duration = (t2.tv_sec - t1.tv_sec) * 1000;
    total_duration += (t2.tv_usec - t1.tv_usec) / 1000;
    printf("Time: %lfms\n", total_duration);

    #if ISFLOAT
        printf("Distance: %f\n", res.dist);
    #else
        printf("Distance: %d\n", res.dist);
    #endif

    printf("Position: %d\n", res.end_position);
    res = subsequence_search(strand_sample, reference[1]);
    
    #if ISFLOAT
        printf("Distance: %f\n", res.dist);
    #else
        printf("Distance: %d\n", res.dist);
    #endif

    printf("Position: %d\n", res.end_position);
    return 0;
}
