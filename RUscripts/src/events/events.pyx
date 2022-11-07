
# distutils: language = c
# cython: language_level=3

import numpy as np
cimport numpy as np
from libc.stdlib cimport *
from libc.stdint cimport *
from libc.stdlib cimport malloc, free
from events cimport *

np.import_array()

def get_events_from_raw(raw, nsample):

    rna = 0
    samples_floats = [float(i) for i in raw]
    cdef np.ndarray[np.float32_t,ndim=1] samples_array
    samples_array = np.ascontiguousarray(samples_floats, dtype=np.float32)


    cdef event_table et = getevents(<size_t> nsample, <float*> samples_array.data, <int8_t> rna)

    ret = {}
    if et.event:
        for i in xrange(et.n):
            ret[i] = et.event[i].mean
    else:
        ret['FAIL'] = ''

    free(et.event);


    return ret
