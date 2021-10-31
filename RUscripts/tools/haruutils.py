import socket
import sklearn.preprocessing as skprep
import numpy as np
from ctypes import Structure
from ctypes import *

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
                ("query_seq", c_int16 * 250)]


# At the moment the reference is being stored directly in the on-chip memory
# of the Zynq device since the reference is small. For larger references, the
# reference would be streamed to the device on each query search.
# def save_reference(seqIDs, threedarray):
#     with open("reference.h", "w") as f:
#         f.write("#define SEQLEN {}\n".format(len(seqIDs)))
#         f.write("float threeddarray[{}][{}][{}] = ".format(
#             threedarray.shape[0], threedarray.shape[1], threedarray.shape[2]))
#         f.write("{\n\t")
#         for ref in threedarray:
#             for direction in ref:
#                 for v_cnt, val in enumerate(direction):
#                     f.write(str(val) + ", ")
#                     if (v_cnt+1) % 10 == 0:
#                         f.write("\n\t")
#         f.write("}\n")
#         f.close()


def squiggle_search(squiggle):
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
    # print(query)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # TCP connection
        s.connect((HOST, PORT))
        payload = Payload(1, (c_int16 * 250)(*query))
        bytes_sent = s.send(payload)
        if bytes_sent != 2008:
            print("Incomplete send, sent", bytes_sent)
