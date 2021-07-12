import os
import sys
import csv
import ctypes
import time
import numpy as np
import multiprocessing as mp
import sklearn.preprocessing as skprep
from Bio import SeqIO

def file_existance_check(files):
    print('Checking files...')
    for file in files:
        if not os.path.isfile(file):
            print('One or more files cannot be found. Exiting..', file=sys.stderr)
            sys.exit()
    print('File check complete.')

def validate_fasta_file(fasta):
    try:
        for record in SeqIO.parse(fasta, 'fasta'):
            print("Fasta record: ", record.id)
    except:
        print('Invalid Fasta. Exiting...', file=sys.stderr)
        sys.exit()

def get_seq_len(ref_fasta):
    seqLens = dict()
    for record in SeqIO.parse(ref_fasta, 'fasta'):
        seq = record.seq
        seqLens[record.id] = len(seq)
    return seqLens

def process_model_file(model_file):
    print("Processing model file: ", model_file)
    model_kmers = dict()
    with open(model_file) as file:
        csv_reader = csv.reader(file, delimiter='\t')
        raw_data = list(csv_reader)
        header = raw_data[0]
        kmer_columnID = header.index("kmer")
        mean_columnID = header.index("level_mean")

        data = np.array(raw_data[1:])
        model_kmers = dict(data[:, [kmer_columnID, mean_columnID]])
        kmer_len = len(data[0][kmer_columnID])
    return model_kmers, kmer_len

def process_ref_fasta(ref_fasta, model_kmer_means, kmer_len):
    kmer_means = {}

    for record in SeqIO.parse(ref_fasta, "fasta"):
        kmer_means[record.id] = {}
        forward = []
        reverse = []
        print("Record ID: ", record.id)
        print("Record length: ", len(record.seq))

        seq = record.seq
        for i in range(len(seq) + 1):
            kmer = "b'"+str(seq[i:i+kmer_len])+"'"
            print(kmer)
            forward.append(float(model_kmer_means[kmer]))
        print("Forward ok")

        seq = record.seq.reverse_complement()
        for i in range(len(seq)+1):
            kmer = str(seq[i:i+kmer_len])
            reverse.append(float(model_kmer_means[kmer]))
        print("Reverse ok")

        kmer_means[record.id]["Fprime"] = skprep.scale(forward, axis=0, with_mean=True, with_std=True, copy=True)
        kmer_means[record.id]["Rprime"] = skprep.scale(reverse, axis=0, with_mean=True, with_std=True, copy=True)
    
    # Iterator for kmer_means
    items = kmer_means.items()
    items_ = map(process_items, items)
    seqIDs, arrays = zip(*items_)

    # 3d Array containing [nSeq][nLists][ListValues]
    nSeq = len(seqIDs)
    row, col = list(arrays)[0].shape
    threedarray = mp.Array(ctypes.c_double, nSeq*row*col)
    # TODO: Check what threedarray_arry is
    threedarrayshared_array = np.ctypeslib.as_array(threedarray.get_obj())
    return seqIDs, threedarrayshared_array
    
def process_items(d):
    seqid = d[0]
    result = []
    for k, v in d[1].items():
        result.append(v)
    return seqid, np.array(result)

def squiggle_search(squiggle, channel_id, read_id, args, seqIDs, threedarray, seqLen):
    result = []
    blocksize = 200000
    overlap = blocksize - 500

    # Search in each seqIDs
    for ref in seqIDs:
        # Obtain reference ID and Rprime, Fprime lists
        refID = seqIDs.index(ref)
        Rprime, Fprime = threedarray[refID]

        # Preprocess squiggle
        queryarray = skprep.scale(np.array(squiggle, dtype=float), axis=0, with_mean=True, with_std=True, copy=True)

        # Run search on forward reference
        refsubset = Fprime
        indexes = np.array(range(len(refsubset)))
        subrefs = [refsubset[i:i+blocksize] for i in indexes[::overlap]]
        for blockID, ref_ in enumerate(subrefs):
            # Get pre-DTW time
            tic = time.time()

            # Run DTW for squiggle and forward reference
            ## TODO: change dtw_subsequence to a wrapper instead of mlpy
            dist, cost, path = mlpy.dtw_subsequence(queryarray, ref_)
            result.append((dist, ref, "F", path[1][0] + (blockID*overlap), path[1][-1] + (blockID*overlap), path[0][0], path[0][-1]))

            # Print time spent in DTW
            # logger.info("Ftime_%s: %s sec", blockID, (time.time() - tic))

        # Run search on reverse reference
        refsubset = Rprime
        subrefs = [refsubset[i:i+blocksize] for i in indexes[::overlap]]
        for blockID, ref_ in enumerate(subrefs):
            # Get pre-DTW time
            tic = time.time()

            # Run DTW for squiggle and reverse reference
            ## TODO: change dtw_subsequence to a wrapper instead of mlpy
            dist, cost, path = mlpy.dtw_subsequence(queryarray, ref_)
            # Corrected for the fact that this is a reverse complement
            result.append((dist,ref,"R",(len(Rprime)-(path[1][0]+(blockID*overlap))),(len(Rprime)-(path[1][-1]+(blockID*overlap))),path[0][0],path[0][-1]))
            
            # Print time spent in DTW
            # logger.info("Rtime_%s: %s sec", blockID, (time.time() - tic))

    # Note first two elements flipped for return deliberately.
    distanceR, seqmatchnameR, frR, rsR, reR, qsR, qeR = sorted(result, key=lambda result: result[0])[0]
    return seqmatchnameR, distanceR, frR, rsR, reR, qsR, qeR


def go_or_no(seqid, direction, position, seqlen, args):
    for sequence in args.targets:
        if args.verbose:
            print(sequence)

        start = int(float(sequence.split(':', 1 )[1].split('-',1)[0]))
        stop = int(float(sequence.split(':', 1 )[1].split('-',1)[1]))
        length = seqlen[seqid]

        if args.verbose:
            print (start, stop, length)
            print (sequence.split(':', 1 )[0])
            print (type(seqid))

        #We note that the average template read length is 6kb for the test lambda dataset. Therefore we are interested in reads which start at least 3kb in advance of our position of interest
        balance = args.length/2
        if seqid.find(sequence.split(':', 1 )[0]) >= 0:
            if args.verbose:
                print("Found it")
            
            if direction == "F":
                if args.verbose:
                    print("Forward Strand")

                if position >= ( start - balance ) and position <= stop:
                    return "Sequence"
            elif direction == "R":
                if args.verbose:
                    print("Reverse Strand")
                ### We assume that coordinates are reported with respect to the forward strand regardless of
                ### wether you are matching to forward or reverse.
                if position >= ( start - balance ) and position <= stop:
                    return "Sequence"
    return "Skip"