from ctypes import Structure
import socket
import struct
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
                ("query_seq", c_double * 250)]


def save_reference(seqIDs, threedarray):
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


def send_squiggle(squiggle):
    '''
    Description: Sends squiggle over to HARU and receives yes or no
    '''
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # TCP connection
        s.connect((HOST, PORT))
        payload = Payload(1, (c_double * 250)(*squiggle))
        bytes_sent = s.send(payload)
        if bytes_sent != 1000:
            print("Incomplete send")
        # print("Sent {} bytes".format(bytes_sent))
