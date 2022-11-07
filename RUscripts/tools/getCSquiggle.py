#! /usr/bin/python3

import h5py
import sklearn.preprocessing as skprep

def listToString(s):
    str1 = ", "
    return "{" + str1.join(s) + "}"

## Hardcoded for now..
strand_filename = "../dataset/lambda/llssbzms2p35x_20151004_readuntiludududududu_RU21_lambdaPCR_2922_1_ch8_file14_strand.fast5"
# reference_filename = ""

print(strand_filename)
hdf = h5py.File(strand_filename, 'r')

for read in hdf['Analyses']['EventDetection_000']['Reads']:
    events = hdf['Analyses']['EventDetection_000']['Reads'][read]['Events'][()]
    event_collection = list()
    its_type = None
    for event in events:
        its_type = type(event[0])
        event_collection.append(float(event[0]))

    event_collection = skprep.scale(
        event_collection[50:300],
        axis=0,
        with_mean=True,
        with_std=True,
        copy=True,
    )
    event_collection = [int(i*10000) for i in event_collection]

    # print(event_collection)
    with open("output/squiggle.txt", "w") as strand:
        strand.write('''#ifndef REFERENCE
#define REFERENCE
#include "squiggle_search.h"
''')
        strand.write("value_t strand_sample[{}] = ".format(len(event_collection)))
        strand.write("{")
        for i, v in enumerate(event_collection):
            if i % 10 == 0:
                strand.write("\n\t")
            strand.write(str(v) + ", ")
        strand.write("\n};\n")