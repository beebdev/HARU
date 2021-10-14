#! /usr/bin/python3

import h5py

def listToString(s):
    str1 = ", "
    return "{" + str1.join(s) + "}"

strand_filename = "./RUtestset/llssbzms2p35x_20151004_readuntiludududududu_RU21_lambdaPCR_2922_1_ch8_file14_strand.fast5"
reference_filename = ""

print(strand_filename)
hdf = h5py.File(strand_filename, 'r')
for read in hdf['Analyses']['EventDetection_000']['Reads']:
    events = hdf['Analyses']['EventDetection_000']['Reads'][read]['Events'][()]
    event_collection = list()
    its_type = None
    for event in events:
        its_type = type(event[0])
        event_collection.append(int(event[0] * 10000))

    # print(event_collection)
    with open("sample.txt", "w") as strand:
        strand.write("int strand_sample[{}] = ".format(len(event_collection)))
        strand.write("{")
        for i, v in enumerate(event_collection):
            if i % 10 == 0:
                strand.write("\n\t")
            strand.write(str(v) + ", ")
        strand.write("\n}")