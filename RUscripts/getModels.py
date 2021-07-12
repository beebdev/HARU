import argparse
import h5py
import sys
import os

__version__ = "0.1v"


### Functions
# def get_model_type(hdf, strand, args):
#     for element in hdf:


if __name__ == "__main__":
    ## Parse arguments
    parser = argparse.ArgumentParser(description="Fasta to raw squiggle model converter")
    parser.add_argument("-r", "--read", required=True,
                        dest="read", type=str,
                        help="Provide a read file to extract the current model from.")
    parser.add_argument("-V", "--verbose-true",
                        dest="debug", default=False, action="store_true",
                        help="Print detailed messages while processing files to aid in debugging.")
    parser.add_argument("-v", "--version", action="version", version=("%(prog)s version={version}".format(version=__version__)))
    args = parser.parse_args()

    try:
        hdf = h5py.File(args.read, "r")
        print(hdf)
    except:
        print("h5py File open failed, exiting...", file=sys.stderr)
        sys.exit(1)

    model_strand = ("template", "complement")
    for model in model_strand:
        model_type = get_model_type(hdf, model, args)
        print("Looking for ", model, " model.")
