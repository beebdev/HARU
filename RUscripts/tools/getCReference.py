#! /usr/bin/python3
import sys
from Bio import SeqIO
import numpy as np
import sklearn.preprocessing as skprep
import csv

def validate_fasta_file(fasta):
    try:
        for record in SeqIO.parse(fasta, "fasta"):
            print("Fasta record: ", record.id)
    except:
        print("Invalid Fasta. Exiting...", file=sys.stderr)
        sys.exit()

def get_seq_length(ref_fasta):
    seqLens = dict()
    for record in SeqIO.parse(ref_fasta, "fasta"):
        seq = record.seq
        seqLens[record.id] = len(seq)
    return seqLens

def process_model_file(model_file):
    """
    Args:
        - model: filepath to the model file
    Returns:
        - model_kmers: dict with kmer string as key and current value as value
        - kmer_len: length of the kmer string (Example run includes the b'' characters)
    Notes:
        * A model file is the mapping from base to squiggle space
    """
    print("Processing model file: ", model_file)
    model_kmers = dict()
    with open(model_file) as file:
        csv_reader = csv.reader(file, delimiter="\t")
        raw_data = list(csv_reader)
        header = raw_data[0]
        kmer_columnID = header.index("kmer")
        mean_columnID = header.index("level_mean")

        data = np.array(raw_data[1:])
        model_kmers = dict(data[:, [kmer_columnID, mean_columnID]])
        kmer_len = len(data[0][kmer_columnID])
        print(kmer_len)
    return model_kmers, kmer_len

def process_ref_fasta(fasta, model_kmer, kmer_len):
    lengths = []
    kmer_means = []
    kmer_len -= 3

    for record in SeqIO.parse(fasta, "fasta"):
        # kmer_means[record.id] = {}
        forward = []  # Forward reference
        reverse = []  # Reverse reference
        print("Record ID: ", record.id)
        print("Record length: ", len(record.seq), "bases")

        seq = record.seq
        for i in range(len(seq) + 1 - kmer_len):
            # Slide through the base reference sequence
            kmer = "b'" + str(seq[i: i + kmer_len]) + "'"
            # and append the squiggle value of each window
            forward.append(float(model_kmer[kmer]) * 1000)
        print("Forward ok")

        seq = record.seq.reverse_complement()
        for i in range(len(seq) + 1 - kmer_len):
            # Slide through the base reference sequence
            kmer = "b'" + str(seq[i: i + kmer_len]) + "'"
            # and append the squiggle value of each window
            reverse.append(float(model_kmer[kmer]))
        print("Reverse ok")
        
        forward = skprep.scale(
            forward, axis=0, with_mean=True, with_std=True, copy=True
        )
        reverse = skprep.scale(
            reverse, axis=0, with_mean=True, with_std=True, copy=True
        )
        lengths.append(len(forward))
        lengths.append(len(reverse))
        kmer_means = np.concatenate([forward, reverse])
        kmer_means = [int(i*10000) for i in kmer_means]
    return lengths, kmer_means


def save_reference(lengths, kmer_means):
    print("Save reference")
    with open("output/reference.txt", "w") as ref_file:
        ref_file.write("value_t reference[2][{}] = ".format(lengths[0]))
        ref_file.write("{")
        for i, v in enumerate(kmer_means):
            if i % 20 == 0:
                ref_file.write("\n\t")
            ref_file.write(str(v) + ", ")
        ref_file.write("\n};\n")
        ref_file.write("#endif\n")

def main():
    # Hardcoded for now..
    fasta = "../dataset/fasta/J02459.fasta"
    model ="../models/template_r7.3_e6_70bps_6mer_6.model"

    validate_fasta_file(fasta)
    seq_len = get_seq_length(fasta)

    model_kmers, kmer_len = process_model_file(model)
    lengths, kmer_means = process_ref_fasta(
        fasta, model_kmers, kmer_len)
    save_reference(lengths, kmer_means)

if __name__ == "__main__":
    main()