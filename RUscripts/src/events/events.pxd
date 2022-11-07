from libc.stdint cimport *

cdef extern from "events.h":

    ctypedef struct event_t:
        uint64_t start
        float length
        float mean
        float stdv

    ctypedef struct event_table:
        size_t n
        size_t start
        size_t end
        event_t* event

    event_table getevents(size_t nsample, float* rawptr, int8_t rna)
