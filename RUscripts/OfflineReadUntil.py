#!/usr/bin/python3

import os
import sys
import platform
import argparse
import glob
import h5py
import multiprocessing as mp
import shutil
import time

from tools import ruutils as ruu
from tools import haruutils as haru

__version__ = "0.1"
__logo__ = """▄▄▄  ▄▄▄ . ▄▄▄· ·▄▄▄▄  ▄• ▄▌ ▐ ▄ ▄▄▄▄▄▪  ▄▄▌  
▀▄ █·▀▄.▀·▐█ ▀█ ██▪ ██ █▪██▌•█▌▐█•██  ██ ██•  
▐▀▀▄ ▐▀▀▪▄▄█▀▀█ ▐█· ▐█▌█▌▐█▌▐█▐▐▌ ▐█.▪▐█·██▪  
▐█•█▌▐█▄▄▌▐█ ▪▐▌██. ██ ▐█▄█▌██▐█▌ ▐█▌·▐█▌▐█▌▐▌
.▀  ▀ ▀▀▀  ▀  ▀ ▀▀▀▀▀•  ▀▀▀ ▀▀ █▪ ▀▀▀ ▀▀▀.▀▀▀ """


max_cost = []
swDTW_time = []
hwDTW_time = []

def process_hdf5(arg):
    filename, seqIDs, threedarray, proc_ampres, seqLen, args, RID = arg
    hdf = h5py.File(filename, 'r')

    for read in hdf['Analyses']['EventDetection_000']['Reads']:
        events = hdf['Analyses']['EventDetection_000']['Reads'][read]['Events'][()]
        event_collection = list()
        for event in events:
            event_collection.append(float(event[0]))

        # We ignore the first 50 events (Protein driver) and process the following 250 events
        squiggle = event_collection[50:300]

        t1 = time.time()
        hw_result = haru.squiggle_search(squiggle, RID)
        t2 = time.time()
        hw_time = t2 - t1

        # Subsequence search for the squiggle
        sw_result = ruu.squiggle_search(squiggle, seqIDs, threedarray)
        seqID = sw_result[0]
        direction = sw_result[2]
        position = sw_result[3]
        
        # Determine rejection decision
        try:
            result = ruu.go_or_no(
                seqID, direction, position, args)
        except Exception as err:
            print("error occurred", err, file=sys.stderr)
    hdf.close()
    return (result, filename, sw_result, hw_result, hw_time)


def mycallback(arg):
    (result, filename, sw_result, hw_result, hw_time) = arg
    filetocheck = os.path.split(filename)
    sourcefile = filename
    swDTW_time.append(sw_result[6])
    hwDTW_time.append(hw_time)

    if result == "Sequence":
        path_output = os.path.join(args.output_folder, 'sequence')
        path_downloads = os.path.join(path_output, 'downloads')
        path_pass = os.path.join(path_downloads, 'pass')
        path_fail = os.path.join(path_downloads, 'fail')

        if not os.path.exists(path_output):
            os.makedirs(path_output)
        if not os.path.exists(path_downloads):
            os.makedirs(path_downloads)
        if not os.path.exists(path_pass):
            os.makedirs(path_pass)
        if not os.path.exists(path_fail):
            os.makedirs(path_fail)

        print("[{}-{} @{}] [{}]\t\u001b[32mSequence found\u001b[0m max {}".format(sw_result[0],
              sw_result[2], sw_result[4], filename, sw_result[5]))
        max_cost.append(sw_result[5])
        
        if "pass" in filename:
            destfile = os.path.join(path_pass, filetocheck[1])
        else:
            destfile = os.path.join(path_fail, filetocheck[1])
        try:
            shutil.copy(sourcefile, destfile)
        except Exception as err:
            print("File copy failed..", file=sys.stderr)
    else:
        path_output = os.path.join(args.output_folder, 'reject')
        path_downloads = os.path.join(path_output, 'downloads')
        path_pass = os.path.join(path_downloads, 'pass')
        path_fail = os.path.join(path_downloads, 'fail')

        if not os.path.exists(path_output):
            os.makedirs(path_output)
        if not os.path.exists(path_downloads):
            os.makedirs(path_downloads)
        if not os.path.exists(path_pass):
            os.makedirs(path_pass)
        if not os.path.exists(path_fail):
            os.makedirs(path_fail)

        print("[{}-{} @{}] [{}]\t\u001b[31mNo match\u001b[0m max{}".format(sw_result[0],
              sw_result[2], sw_result[3], filename, sw_result[5]))
        max_cost.append(sw_result[5])

        if "pass" in filename:
            destfile = os.path.join(path_pass, filetocheck[1])
        else:
            destfile = os.path.join(path_fail, filetocheck[1])
        try:
            shutil.copy(sourcefile, destfile)
        except Exception as err:
            print("File copy failed...", file=sys.stderr)


# Main code
if __name__ == "__main__":
    global os_platform
    os_platform = platform.system()

    if os_platform != "Linux":
        print("Platform not supported", file=sys.stderr)
    else:
        config_fpath = os.path.join(os.path.sep, os.path.dirname(
            os.path.realpath('__file__')), 'amp.config')

    # Parse arguments
    parser = argparse.ArgumentParser(
        description='Offline Read Until simulation.')
    parser.add_argument('-f', '--fasta', metavar='FILE', required=True,
                        dest='fasta', type=str, default=None, action='store',
                        help='The fasta format file describing the reference sequence for your organism.')
    parser.add_argument('-t', '--targets', required=True,
                        dest='targets', type=str, nargs='*',
                        help='Positional IDs to enrich for in the form seqid:start-stop . Can be space seperated eg: J02459:10000-15000 J02459:35000-40000')
    parser.add_argument('-p', '--num-process', required=True,
                        dest='procs', type=int,
                        help='The number of processors to run this on.')
    parser.add_argument('-m', '--model', required=True,
                        dest='model', type=str,
                        help='The appropriate template model file to use')
    parser.add_argument('-l', '--log',
                        dest='logfile', type=str, default='readuntil.log',
                        help='The name of the log file that data will be written to regarding the decision made by this program to process read until.')
    parser.add_argument('-w', '--watch-directory', required=True,
                        dest='watchdir', type=str, default=None,
                        help='The path to the folder containing the downloads directory with fast5 reads to analyse - e.g. C:\data\minion\downloads (for windows).')
    parser.add_argument('-L', '--lib-length', required=False,
                        dest='length', type=int, default=0,
                        help='Provide the average expected length of your library. This offset will be applied to reads that are likely to extend into your region of interest on either strand.')
    parser.add_argument('-o', '--output', required=False,
                        dest='output_folder', type=str, default='test_ru_out',
                        help='Path to a folder to symbolically place reads representing match and not match.')
    parser.add_argument('-V', '--verbose',
                        dest='verbose', action='store_true', default=False,
                        help='Print detailed messages while processing files.')
    parser.add_argument('-v', '--version', action='version',
                        version=('%(prog)s version={version}').format(version=__version__))
    args = parser.parse_args()

    print(__logo__)

    # Validate fasta and model files
    ruu.file_existance_check((args.fasta, args.model))
    ruu.validate_fasta_file(args.fasta)

    # Multiprocess setup
    p = mp.Pool(args.procs)
    manager = mp.Manager()
    proc_ampres = manager.dict()
    fast_file = args.fasta
    seq_len = ruu.get_seq_len(fast_file)

    # Process model and reference fasta
    model_file = args.model
    global model_ker_means
    global kmer_len
    model_ker_means, kmer_len = ruu.process_model_file(model_file)
    seqIDs, threedarray = ruu.process_ref_fasta(
        fast_file, model_ker_means, kmer_len)

    # haru.save_reference(seqIDs, threedarray)

    # Scrap filenames
    data = []
    filenamecounter = 0
    for filename in glob.glob(os.path.join(args.watchdir, '*.fast5')):
        filenamecounter += 1
        data.append([filename, seqIDs, threedarray,
                    proc_ampres, seq_len, args, filenamecounter])
    for filename in glob.glob(os.path.join(args.watchdir, "pass", '*.fast5')):
        filenamecounter += 1
        data.append([filename, seqIDs, threedarray,
                    proc_ampres, seq_len, args, filenamecounter])
    for filename in glob.glob(os.path.join(args.watchdir, "fail", '*.fast5')):
        filenamecounter += 1
        data.append([filename, seqIDs, threedarray,
                    proc_ampres, seq_len, args, filenamecounter])
    procdata = tuple(data)

    # Start multiprocessing
    print("Start spawing hdf5 processes")
    results = []
    for d in (procdata):
        result = p.apply_async(process_hdf5, args=(d,), callback=mycallback)
        # print(result.get())
        results.append(result)
    for result in results:
        result.wait()

    print("max cost value: ", max(max_cost))
    print("Total runs: ", len(swDTW_time))
    print("Total sw time: ", sum(swDTW_time), "sec")
    print("Average sw time per squiggle: ", sum(swDTW_time) / len(swDTW_time), "sec")
    print("Total hw time: ", sum(hwDTW_time), "sec")
    print("Average hw time per squiggle: ", sum(hwDTW_time) / len(hwDTW_time), "sec")
    print("Read until completed. Exiting...")
