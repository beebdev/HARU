# Hardware Accelerated Read Until (HARU)

HARU aims to accelerate the Nanopore selective sequencing tool Read Until with low cost FPGA. This work extends on the Dynamic Time Warping (DTW) based implementation of Read Until (RUscripts).

The current implementation of the subsequence DTW was developed using HLS for software/bioinformatics developers interested in replicating the work to quickly extend and prototype an FPGA accelerator for their targetted genome.

[Add system diagram here]

The HARU system composes of the modified RUscripts implementation that sends the query squiggle through a socket connection to the Zedboard where the PS side hosts a minimal server waiting for incoming queries. When receiving a query squiggle, the HARU server on PS starts the subseekDTW accelerator on the PL side of the Zynq MPSoC and transfers the squiggle sequence via AXI stream. Once the whole sequence is streamed, the accelerator starts processing and the server application polls for completion. On completion, the results are obtained through General Purpose AXI and sent back to the RUscripts Python implementation where the Read Until API interface is being done.

## Repo structure

### HARU_server
This directory contains the C application code being run on the PS side of the Zynq MPSoC.

### SubseekDTW-HLS
This directory contains the HLS implementation source for the custom subsequence DTW used in the selective sequencing decision making logic. It exists within this repo as a submodule.

## Quickstart
### Setup
Clone the repo and the submodules contained within the repo with:

```sh
git clone --recursive https://github.com/beebdev/HARU
```

> If you cloned the repo without ```--recursive``` you can clone the submodules with ```git submodule update --init --recursive```

### Running Offline RUscripts without SubseekDTW
# HARU
Hardware Accelerated Read Until R9 branch


# Quick start

## Getting HARU:
```sh
git clone https://github.com/beebdev/HARU -b r9_slow5 && cd HARU
```

## Setting up dependancies to run the updated RUscripts
```sh
cd RUscripts
python3 -m venv env
source env/bin/activate
pip3 install --upgrade pip
pip3 install numpy==1.18.0 pyslow5 biopython==1.69 scikit-learn==0.20.0 scipy==1.4.0 six==1.16.0 Cython
python3 setup.py install
```

## Running offline RUscripts
To run the software-based updated offline RUscripts on the example Covid-19 dataset:
```sh
cd RUscripts
python3 OfflineReadUntil.py -f dataset/fasta/nCoV-2019.reference.fasta -t MN908947.3:10000-15000 -p 4 -m models/r9.4_450bps.nucleotide.6mer.template.model -w dataset/ncov-testset/slow5 -o RUgOUT -L 3000
```

### Synthesising SubseekDTW HLS implementation
```sh
vivado_hls -f sdtw_hls.tcl
```
## Testing mapping accuracy
With venv activated, install UNCALLED with:
```sh
pip3 install git+https://github.com/skovaka/UNCALLED.git
```

Generate the paf output by running offline RUscripts and redirecting the stdout output to a file
```sh
python3 OfflineReadUntil.py -f dataset/fasta/nCoV-2019.reference.fasta -t MN908947.3:10000-15000 -p 4 -m models/r9.4_450bps.nucleotide.6mer.template.model -w dataset/ncov-testset/slow5 -o RUgOUT -L 3000 > batch_result.paf


python3 OfflineReadUntil.py -f /mnt/d/pr__haru/testset/yeast/GCF_000146045.2_R64_genomic.fna -t NC_001138.5:2000-7000 -p 4 -m models/r9.4_450bps.nucleotide.6mer.template.model -w /mnt/d/pr__haru/testset/yeast/set/ -o RUgOUT -L 3000 > yeast_batch.paf
```

Setup
