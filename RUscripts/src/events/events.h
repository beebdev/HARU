
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// a single event : adapted from taken from scrappie
typedef struct {
    uint64_t start; //start index
    float length; //todo : cant be made int? end = start+length : end is not inclusive
    float mean;
    float stdv;
    //int32_t pos;   //todo : always -1 can be removed
    //int32_t state; //todo : always -1 can be removed
} event_t;

// event table : adapted from scrappie
typedef struct {
    size_t n;     //todo : int32_t not enough?
    size_t start; //todo : always 0?
    size_t end;   //todo : always equal to n?
    event_t* event;
} event_table;

// interface to scrappie functions
event_table getevents(size_t nsample, float* rawptr, int8_t rna);
