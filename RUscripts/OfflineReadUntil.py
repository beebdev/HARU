#!/usr/bin/python3
import os
import sys
import platform
import argparse
import glob
import pyslow5
import events
import multiprocessing as mp
import shutil

from tools import ruutils as ruu
from tools import haruutils as haru

__version__ = "v0.0.99"
__logo__ = """
▄▄▄  ▄▄▄ . ▄▄▄· ·▄▄▄▄  ▄• ▄▌ ▐ ▄ ▄▄▄▄▄▪  ▄▄▌
▀▄ █·▀▄.▀·▐█ ▀█ ██▪ ██ █▪██▌•█▌▐█•██  ██ ██•
▐▀▀▄ ▐▀▀▪▄▄█▀▀█ ▐█· ▐█▌█▌▐█▌▐█▐▐▌ ▐█.▪▐█·██▪
▐█•█▌▐█▄▄▌▐█ ▪▐▌██. ██ ▐█▄█▌██▐█▌ ▐█▌·▐█▌▐█▌▐▌
.▀  ▀ ▀▀▀  ▀  ▀ ▀▀▀▀▀•  ▀▀▀ ▀▀ █▪ ▀▀▀ ▀▀▀.▀▀▀ 
"""


def process_hdf5(arg):
    # Unpack args
    filename, seqIDs, threedarray, proc_ampres, seqLen, args = arg

    # Create generator
    s5 = pyslow5.Open(filename,'r')
    reads = s5.seq_reads(pA=True)

    for read in reads:

        events_means = events.get_events_from_raw(read['signal'], read['len_raw_signal'])

        event_collection = list()
        for i in range(0,len(events_means)):
            event_collection.append(float(events_means[i]))

        # We ignore the first 50 events (Protein driver) and process the following 250 events
        squiggle = event_collection[50:300]

        # Search squiggle in reference squiggle
        # haru.send_squiggle(squiggle) # TODO: move this to a separate function or add a condition for HARU flag
        squiggleres = ruu.squiggle_search(squiggle, seqIDs, threedarray)
        seqid = squiggleres[0]
        direction = squiggleres[2]
        position = squiggleres[3]

        # Output of results
        # TODO: tidy up
        # PAF format for the squiggle dtw results
        res_string = ""
        # Query sequence name
        res_string += str(read['read_id']) + "\t"
        # Query sequence length
        res_string += "250\t"
        # Query start
        # res_string += str(position) + "\t"
        res_string += "50\t"
        # Query end
        # res_string += str(position+250) + "\t"
        res_string += "300\t"
        # Direction
        if direction == "F":
            res_string += "+\t"
        else:
            res_string += "-\t"
        # Target sequence name
        res_string += str(seqid) + "\t"
        # Target sequence length
        res_string += str(seqLen[seqid]) + "\t"
        # Target start
        res_string += str(position) + " \t"
        # Target end
        res_string += str(position + 250) + "\t"
        # Number of residues
        res_string += str(seqLen[seqid]) + "\t"
        # Alignment block length
        res_string += str(seqLen[seqid]) + "\t"
        # Mapping quality
        res_string += "60"
        print(res_string)
        
        # print(read['read_id'], seqid, direction, position)
        if True:
            try:
                result = ruu.go_or_no(
                    seqid, direction, position, seqLen, args)
            except Exception as err:
                print("error occurred", err, file=sys.stderr)
    return (result, filename, squiggleres)


def mycallback(arg):
    (result, filename, squiggleres) = arg
    filetocheck = os.path.split(filename)
    sourcefile = filename

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

        # logger.info("[%s-%s @%s] \033[42;1mSequence found\033[0m\n[%s]", squiggleres[0], squiggleres[2], squiggleres[3], filename)
        print("[{}-{} @{}] [{}] [{}]\t\u001b[32mSequence found\u001b[0m".format(squiggleres[0],
              squiggleres[2], squiggleres[4], squiggleres[1], filename), file=sys.stderr)
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

        print("[{}-{} @{}] [{}]\t\u001b[31mNo match\u001b[0m".format(squiggleres[0],
              squiggleres[2], squiggleres[3], filename), file=sys.stderr)
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

    # HARU logo
    sys.stderr.write(__logo__)

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

    # TODO: send seqIDs and threedarray to HARU_PS
    haru.save_reference(seqIDs, threedarray)

    # TODO: wait for setup

    # Scrap filenames
    data = []
    filenamecounter = 0
    for filename in glob.glob(os.path.join(args.watchdir, '*.blow5')):
        filenamecounter += 1
        data.append([filename, seqIDs, threedarray,
                    proc_ampres, seq_len, args])
    for filename in glob.glob(os.path.join(args.watchdir, "pass", '*.blow5')):
        filenamecounter += 1
        data.append([filename, seqIDs, threedarray,
                    proc_ampres, seq_len, args])
    for filename in glob.glob(os.path.join(args.watchdir, "fail", '*.blow5')):
        filenamecounter += 1
        data.append([filename, seqIDs, threedarray,
                    proc_ampres, seq_len, args])
    procdata = tuple(data)

    # Assign process hdf5 to processes
    print("Start spawing hdf5 processes", file=sys.stderr)
    results = []
    for d in (procdata):
        result = p.apply_async(process_hdf5, args=(d,), callback=mycallback)
        # print(result.get())
        results.append(result)
    for result in results:
        result.wait()

    print("Read until completed. Exiting...", file=sys.stderr)
