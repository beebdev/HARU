


from logging import fatal
import os
import sys
import platform
import argparse

__version__ = "0.1"

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
                        dest='temp_model', type=str,
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
