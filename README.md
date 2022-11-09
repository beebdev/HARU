# Hardware Accelerated Read Until (HARU)

HARU is a heterogenous compute implementation for Oxford Nanopore Technologies' [Read Until](https://nanoporetech.com/resource-centre/read-until-adaptive-sampling). For sequencing applications targeting regions of a reference, Read Until provides real-time filtering of squiggles. HARU takes advantage on heterogenous edge compute platforms and provide hardware acceleration through the subsequence DTW accelerator running on the reconfigurable hardware of the MPSoC. At the moment the primary target device is Xilinx's Kria AI Starter Kit which uses an Zynq Ultrascale+ MPSoC. This repository provides the verilog source code to the accelerator, userspace software driver for the accelerator, and example system usage of HARU (mainly through [sigfish](https://github.com/beebdev/sigfish/tree/master)).

# Quickstart
To quickly test out HARU and the example system, Sigfish, you can download the pre-built binary package built for Kria from the [latest release](https://github.com/beebdev/HARU/releases).

## What you will need
- [Xilinx's Kria AI Starter Kit](https://www.xilinx.com/member/forms/download/xef.html?filename=petalinux-sdimage.wic.gz)
- Micro SD card with at least 16GB of storage 
## Steps
1. Download the [prebuilt default PetaLinux image](https://www.xilinx.com/member/forms/download/xef.html?filename=petalinux-sdimage.wic.gz) for the Kria AI Starter Kit. Note that you will need to login with your Xilinx account to download.
2. Using your preferred imaging tool (e.g. [Balena Etcher](https://www.balena.io/etcher/)) and flash the image onto the SD card.
3. Once the Micro SD card is prepared, insert it into the SD card slot of the Kria board. Connect the serial console to your host machine and power on the board. You will need go through the setup process on your first power-on. More details 
4. Transfer the prebuilt package of HARU to the device either through `scp` or a drive.
5. Untar the package and run the installation script. Check the output for `haru-core` in the list.
    ```
    tar -xzf haru-v0.1-binaries.tgz
    cd haru-v0.1-binaries
    ./haru_install.sh
    ```
6. Load the accelerator.
    ```
    xmutil unloadapp
    xmutil loadapp haru-core
    xmutil listapps
    ```
7. Run the sigfish software. The package includes example data.
    ```
    ./sigfish dtw -g test_data/nCoV-2019.reference.fasta -s test_data/reads_0_0.blow5 > output.paf
    ```
