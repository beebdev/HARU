# Hardware Accelerated Read Until (HARU)

HARU is a heterogenous compute solution for Oxford Nanopore Technologies' adaptive sampling (also known as selective sequencing and [Read Until](https://nanoporetech.com/resource-centre/read-until-adaptive-sampling)). Read Until allows genomic reads to be analyzed in real-time and abandoned halfway, if not belonging to a genomic region of 'interest'. HARU takes advantage of heterogenous edge compute platforms and provide hardware acceleration using reconfigurable hardware on an Multiprocessor system on a chip  (MPSoC). 

Our current proof-of-concept implementation of HARU utilises a custom subsequence DTW accelerator primarily targeted for a [Xilinx's Kria AI Starter Kit](https://www.xilinx.com/products/som/kria/kv260-vision-starter-kit.html) which uses an Zynq Ultrascale+ MPSoC. This repository contains the source code for this accelerator, including the Verilog HDL core accelerator and user space device driver. The use of our hardware accelerator is demonstrated through an example application called *sigfish-haru* for which the source code is available [here](https://github.com/beebdev/sigfish-haru). The instructions for setting up *sigfish-haru* are given below. 


# Quickstart
To quickly test out HARU, you can download the pre-built binary package built for Kria from the [latest release](https://github.com/beebdev/HARU/releases).

## What you will need
- [Xilinx's Kria AI Starter Kit](https://www.xilinx.com/products/som/kria/kv260-vision-starter-kit.html)
- Micro SD card with at least 16GB of storage 

## Steps
1. Download the prebuilt PetaLinux image for the Kria AI Starter Kit available from the [releases]((https://github.com/beebdev/HARU/releases) (named petalinux-sdimage.wic.gz).
2. Using your preferred imaging tool (e.g. [Balena Etcher](https://www.balena.io/etcher/)), flash the image onto the SD card.
3. Once the Micro SD card is prepared, insert it into the Micro SD card slot on the Kria board. 
4. Connect the USB serial port on the Kria board (micro USB slot) to your host machine USB port. Two serial devices (COM ports) with consecutive numbers should appear (e.g., COM5 and COM6 on Window) where the lower numbered COM port is associated with the UART.
5. Using your preferred serial terminal software (e.g., [TeraTerm](https://ttssh2.osdn.jp/index.html.en)) on your host machine, open the COM port with the lower number (e.g., COM5) with BAUD rate of 115000.  
6. Power on the kria board and go through the setup process on your first power-on. You can connect the Kria board to Internet using Ethernet and SSH to it if you wish.
4. Transfer the prebuilt package of HARU available under [releases]((https://github.com/beebdev/HARU/releases) (named haru-<version>-binaries.tgz
) to the kria board either through `scp` command or a USB drive. If you connected Kria board to Internet using Ethernet, you can simply use `wget` to download.
5. On the Kria board, untar the package and run the installation script.
    ```
    tar -xzf haru-v0.1.0-beta-binaries.tgz
    cd haru-v0.1.0-beta-binaries
    ./haru_install.sh
    ```
6. Load the accelerator on the Kria board.
    ```
    xmutil unloadapp
    xmutil loadapp haru-core
    xmutil listapps
    ```
7. Run the included *sigfish-haru* software binary that uses the hardware accelerator. The binary package includes example data.
    ```
    ./sigfish-haru dtw -g test_data/nCoV-2019.reference.fasta -s test_data/reads_0_0.blow5 > output.paf
    ```
8. If you wish, you can run the *sigfish-cpu* binary that does not use the hardware accelerator and see how slow itis.
    ```
    ./sigfish-cpu dtw -g test_data/nCoV-2019.reference.fasta -s test_data/reads_0_0.blow5 > output.paf
    ```
