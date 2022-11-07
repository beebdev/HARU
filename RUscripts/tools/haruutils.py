from asyncio import events
from ctypes import Structure
import socket
import sklearn.preprocessing as skprep
import numpy as np
# import h5py
import time
from ctypes import Structure
from ctypes import *
import sys
import pyslow5
import events
import numpy as np
import sklearn.preprocessing as skprep

HOST = '10.10.130.3'  # The server's hostname or IP address
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
    Sends the squiggle to the haru server and returns the direction,
    position and score of the best match.
    '''
    # Normalise squiggle
    query = skprep.scale(
        np.array(squiggle, dtype=float),
        axis=0,
        with_mean=True,
        with_std=True,
        copy=True,
    )

    # Scale squiggle
    query = [c_int16(int(i*(2**5))) for i in query]

    # Send squiggle to server
    start = time.time()
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # TCP connection
        s.connect((HOST, PORT))
        payload = Payload(RID, (c_int16 * 250)(*query))
        _ = s.send(payload) # Should send out 504 bytes
        
        # Recieve response
        data = s.recv(2048)
        result = Results.from_buffer_copy(data)

        hw_time = time.time() - start
        return hw_time, result.id, result.direction, result.position, result.score


# def send_batch_to_hw(args):
#     '''
#     Send the batch of queries to the haru server.
#     '''
#     print("Sending batch to haru server...")

#     IDs = []
#     data = []

#     # Read each squiggle from the file
#     count = 0
#     for read in args:
#         filename, RID = read
#         hdf = h5py.File(filename, 'r')

#         for read in hdf['Analyses']['EventDetection_000']['Reads']:
#             events = hdf['Analyses']['EventDetection_000']['Reads'][read]['Events'][()]
#             event_collection = list()
#             for event in events:
#                 event_collection.append(float(event[0]))
#             IDs.append(RID)

#             # Normalise squiggle
#             squiggle = skprep.scale(
#                 np.array(event_collection[50:300], dtype=float),
#                 axis=0,
#                 with_mean=True,
#                 with_std=True,
#                 copy=True,
#             )

#             # Scale squiggle
#             scaled_data = [c_int16(int(i*(2**5))) for i in squiggle]
#             data += scaled_data
#             count += 1
#             print(count)
#             print(scaled_data)
#             print("===========================")

#     IDs = [c_uint32(i) for i in IDs]

#     # Send batch to haru server
#     with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#         # TCP connection
#         s.connect((HOST, PORT))
#         payload = Payload((c_uint32 * 55)(*IDs), (c_int16 * 13750)(*data))
#         s.send(payload)

#     return 0
#         # Receive results
#         # data = s.recv(8192)