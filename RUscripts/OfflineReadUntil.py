import os
import sys
import platform
import argparse
import glob
import h5py
import ruutils as ruu
import multiprocessing as mp

__version__ = "0.1"
__logo__ = """▄▄▄  ▄▄▄ . ▄▄▄· ·▄▄▄▄  ▄• ▄▌ ▐ ▄ ▄▄▄▄▄▪  ▄▄▌  
▀▄ █·▀▄.▀·▐█ ▀█ ██▪ ██ █▪██▌•█▌▐█•██  ██ ██•  
▐▀▀▄ ▐▀▀▪▄▄█▀▀█ ▐█· ▐█▌█▌▐█▌▐█▐▐▌ ▐█.▪▐█·██▪  
▐█•█▌▐█▄▄▌▐█ ▪▐▌██. ██ ▐█▄█▌██▐█▌ ▐█▌·▐█▌▐█▌▐▌
.▀  ▀ ▀▀▀  ▀  ▀ ▀▀▀▀▀•  ▀▀▀ ▀▀ █▪ ▀▀▀ ▀▀▀.▀▀▀ """

def process_hdf5(arg):
    filename, seqIDs, threedarray, proc_ampres, seqLen, args = arg
    hdf = h5py.File(filename, 'r')

    for read in hdf['Analyses']['EventDetection_000']['Reads']:
        events = hdf['Analyses']['EventDetection_000']['Reads'][read]['Events'][()]
        event_collection = list()
        for event in events:
            event_collection.append(float(event[0]))
        
        # We ignore the first 50 events (Protein driver) and process the following 250 events
        squiggle = event_collection[50:300]

        # Search squiggle in reference squiggle
        squiggleres = ruu.squiggle_search(squiggle, 0, 0, args, seqIDs, threedarray, seqLen)

        if True:
            # logger.info("[%s, %s, %s]", squiggleres[0], squiggleres[2], squiggleres[3])
            try:
                result = ruu.go_or_no(squiggleres[0], squiggleres[2], squiggleres[3], seqLen, args)
            except Exception as err:
                print("error occurred", err, file=sys.stderr)
    hdf.close()
    return (result, filename, squiggleres)

if __name__ == "__main__":
    global os_platform
    os_platform = platform.system()

    if os_platform != "Linux":
        print("Platform not supported", file=sys.stderr)
    else:
        config_fpath = os.path.join(os.path.sep, os.path.dirname(os.path.realpath('__file__')), 'amp.config')

    ## Parse arguments
    parser = argparse.ArgumentParser(description='Offline Read Until simulation.')
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
    parser.add_argument('-v', '--version', action='version', version=('%(prog)s version={version}').format(version=__version__))
    args = parser.parse_args()

    print(__logo__)

    ## Validate fasta and model files
    ruu.file_existance_check((args.fasta, args.model))
    ruu.validate_fasta_file(args.fasta)

    ## Multiprocess setup
    p = mp.Pool(args.procs)
    manager = mp.Manager()
    proc_ampres = manager.dict()
    fast_file = args.fasta
    seq_len = ruu.get_seq_len(fast_file)

    ## Process model and reference fasta
    model_file = args.model
    global model_ker_means
    global kmer_len
    model_ker_means, kmer_len = ruu.process_model_file(model_file)
    # print(model_ker_means)
    # seqIDs, threedarray = ruu.process_ref_fasta(fast_file, model_ker_means, kmer_len)

    # ## Scrap filenames
    # data = []
    # filenamecounter = 0
    # for filename in glob.glob(os.path.join(args.watchdir, '*.fast5')):
    #     filenamecounter += 1
    #     data.append([filename, seqIDs, threedarray, proc_ampres, seq_len, args])
    # for filename in glob.glob(os.path.join(args.watchdir, "pass",'*.fast5')):
    #     filenamecounter += 1
    #     data.append([filename, seqIDs, threedarray, proc_ampres, seq_len, args])
    # for filename in glob.glob(os.path.join(args.watchdir, "fail",'*.fast5')):
    #     filenamecounter += 1
    #     data.append([filename, seqIDs, threedarray, proc_ampres, seq_len, args])
    # procdata = tuple(data)

    # # Assign process hdf5 to processes
    # print("Start spawing hdf5 processes")
    # results = []
    # for d in (procdata):
    #     result = p.apply_async(process_hdf5, args=(d,), callback=mycallback)
    #     results.append(result)
    # for result in results:
    #     result.wait()

    # print("Read until completed. Exiting...")