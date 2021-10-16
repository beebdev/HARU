# Hardware Accelerated Read Until (HARU)

HARU aims to accelerate the Nanopore selective sequencing tool Read Until with low cost FPGA. This work extends on the Dynamic Time Warping (DTW) based implementation of Read Until (RUscripts).

The current implementation of the subsequence DTW was developed using HLS for software/bioinformatics developers interested in replicating the work to quickly extend and prototype an FPGA accelerator for their targetted genome.

[Add system diagram here]

The HARU system composes of the modified RUscripts implementation that sends the query squiggle through a socket connection to the Zedboard where the PS side hosts a minimal server waiting for incoming queries. When receiving a query squiggle, the HARU server on PS starts the subseekDTW accelerator on the PL side of the Zynq MPSoC and transfers the squiggle sequence via AXI stream. Once the whole sequence is streamed, the accelerator starts processing and the server application polls for completion. On completion, the results are obtained through General Purpose AXI and sent back to the RUscripts Python implementation where the Read Until API interface is being done.

## Repo structure
### RUscripts
This is an extended implementation of the original [RUscripts](https://github.com/mattloose/RUscripts). The script included here has been updated to work with Python 3.8 and has simple socket code to send the squiggle through ethernet.

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

```sh
python3 OfflineReadUntil.py -f ./J02459.fasta -t J02459:10000-15000 -p 4  -m models/template_r7.3_e6_70bps_6mer_6.model -w ./RUtestset/ -o RUgOUT -L 3000
```

### Synthesising SubseekDTW HLS implementation
```sh
vivado_hls -f sdtw_hls.tcl
```