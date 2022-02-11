from asyncio import events
from ctypes import Structure
import socket
from ctypes import *
import sys
import pyslow5
import events
import numpy as np
import sklearn.preprocessing as skprep

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 3490        # The port used by the server

# def send_seqID_prep(seqIDs):
#     msg = "0" # msg type: seqID
#     msg += str(len())

# def send_reference(seqIDs, threedarray):
#     with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#         # TCP connection
#         s.connect((HOST, PORT))
#         # Send seqIDs
#         msg = bytes(";[0 {};]".format(len(seqIDs)), 'utf-8')
#         bytes_sent = s.sendall(msg)
#         print("Sent {} bytes".format(bytes_sent))
#         payload = pickle.dumps(seqIDs)
#         s.send(payload)
#         # msg = bytes(";1 {}".format(threedarray.shape), 'utf-8')
#         # bytes_sent = s.sendall(msg)
#         # print("Sent {} bytes".format(bytes_sent))
#         # payload = pickle.dumps(threedarray)
#         # s.send(payload)
#     print("Reference sent!")


class Payload(Structure):
    _fields_ = [("id", c_uint32),
                ("query_seq", c_double * 250)]


def save_c_reference(seqIDs, threedarray):
    with open("reference.h", "w") as f:
        f.write("#define SEQLEN {}\n".format(len(seqIDs)))
        f.write("float threeddarray[{}][{}][{}] = ".format(
            threedarray.shape[0], threedarray.shape[1], threedarray.shape[2]))
        f.write("{\n\t")
        for ref in threedarray:
            for direction in ref:
                for v_cnt, val in enumerate(direction):
                    f.write(str(val) + ", ")
                    if (v_cnt+1) % 10 == 0:
                        f.write("\n\t")
        f.write("}\n")
        f.close()

def save_reference_bram(seqIDs, threedarray):
    print("Saving reference for bram initialization", file=sys.stderr)
    for sID in seqIDs:
        print(sID, len(threedarray[0][0]))

    with open("reference.txt", "w") as f, open("reference_dec.txt", "w") as f_dec:
        for value in threedarray[0][0]:
            value = int(value * 2**5)
            f.write(dec_2_binary(value, 16) + "\n")
            f_dec.write(str(value) + "\n")

def save_query_bram(filename):
    print("Saving query read for bram initialization", file=sys.stderr)
    s5 = pyslow5.Open(filename, 'r')
    reads = s5.seq_reads(pA = True)
    for read in reads:
        events_means = events.get_events_from_raw(read['signal'], read['len_raw_signal'])
        events_collection = []
        for i in range(0, len(events_means)):
            events_collection.append(events_means[i])
        squiggle = events_collection[50:300]
        print(min(squiggle), max(squiggle))

        squiggle_norm = skprep.scale(
            np.array(squiggle, dtype=float),
            axis=0,
            with_mean=True,
            with_std=True,
            copy=True,
        )
        squiggle_norm *= 2**5
        with open("query.txt", "w") as f, open("query_dec.txt", "w") as f_dec:
            for val in squiggle_norm:
                f.write(dec_2_binary(int(val), 16) + "\n")
                f_dec.write(str(int(val)) + "\n")
        break

def dec_2_binary(dec, width):
    value = str(bin(dec & int("1"*width, 2)))[2:]
    if len(value) < width:
        value = "0"*(width-len(value)) + value
    return value

def send_squiggle(squiggle):
    '''
    Description: Sends squiggle over to HARU and receives yes or no
    '''
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # TCP connection
        s.connect((HOST, PORT))
        payload = Payload(1, (c_double * 250)(*squiggle))
        bytes_sent = s.send(payload)
        if bytes_sent != 2008:
            print("Incomplete send")
