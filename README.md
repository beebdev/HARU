# Hardware Accelerated Read Until (HARU)

HARU is a heterogenous compute solution for Oxford Nanopore Technologies' adaptive sampling (also known as selective sequencing and [Read Until](https://nanoporetech.com/resource-centre/read-until-adaptive-sampling)). Read Until allows genomic reads to be analyzed in real-time and abandoned halfway, if not belonging to a genomic region of 'interest'. HARU takes advantage of heterogenous edge compute platforms and provide hardware acceleration using reconfigurable hardware on an Multiprocessor system on a chip  (MPSoC). 

Our current proof-of-concept implementation of HARU utilises a custom subsequence DTW accelerator primarily targeted for a [Xilinx's Kria AI Starter Kit](https://www.xilinx.com/products/som/kria/kv260-vision-starter-kit.html) which uses an Zynq Ultrascale+ MPSoC. This repository contains the source code for this accelerator, including the [Verilog HDL core accelerator](https://github.com/beebdev/HARU/tree/main/hdl) and [user space device driver](https://github.com/beebdev/HARU/tree/main/driver). The use of our hardware accelerator is demonstrated through an example application called *sigfish-haru* for which the source code is available [here](https://github.com/beebdev/sigfish-haru). The instructions for setting up *sigfish-haru* are given below. 

**What you will need**:
- [Xilinx's Kria AI Starter Kit](https://www.xilinx.com/products/som/kria/kv260-vision-starter-kit.html)
- Micro SD card with at least 16GB of storage 
- a host computer

# Quickstart
To quickly test out HARU, you can download the pre-built binary package built for Kria from the [latest release](https://github.com/beebdev/HARU/releases).

## Steps
1. Download the prebuilt PetaLinux image for the Kria AI Starter Kit available from the [releases](https://github.com/beebdev/HARU/releases) (named *petalinux-sdimage.wic.gz*).
2. Using your preferred imaging tool (e.g. [Balena Etcher](https://www.balena.io/etcher/)), flash the image onto the micro SD card.
3. Once the Micro SD card is prepared, insert it into the Micro SD card slot on the Kria board. 
4. Connect the USB serial port on the Kria board (micro USB slot) to your host machine's USB port. Two serial devices (COM ports) with consecutive numbers should appear (e.g., COM5 and COM6 on Windows) where the lower numbered COM port is associated with the USART.
5. Using your preferred serial terminal software (e.g., [TeraTerm](https://ttssh2.osdn.jp/index.html.en)) on your host machine, open the COM port with the lower number (e.g., COM5) with BAUD rate of 115000.  
6. Power on the Kria board and go through the setup process on your first power-on. The dafault username is *root* which does not require a password. You can optionally connect the Kria board to Internet using Ethernet and SSH to it if you wish. **IMPORTANT: to avoid any security issues make sure at least you set a password using `passwd` command if you are connecting to a network.**
4. Transfer the prebuilt package of HARU available under [releases](https://github.com/beebdev/HARU/releases) (named *haru-\<version\>-binaries.tgz*
) to the Kria board either through `scp` command or a USB drive. If you connected the Kria board to Internet using Ethernet, you can simply use `wget` to download directly from the GitHub link.
5. On the Kria board, untar the package and run the installation script to install the accelerator.
    ```
    tar -xzf haru-<version>-binaries.tgz
    cd haru-<version>-binaries
    ./haru_install.sh
    ```
6. Now, load the accelerator on to the FPGA on the Kria board.
    ```
    # unload the existing accelerator
    xmutil unloadapp
    # load our HARU sDTW accelerator
    xmutil loadapp haru-dtw-firmware
    # list teh accelerators to verify if the loading was successfull
    xmutil listapps
    ```
7. Run the included *sigfish-haru* software binary that uses the hardware accelerator. The binary package includes example data.
    ```
    ./sigfish-haru dtw -g test_data/nCoV-2019.reference.fasta -s test_data/reads_0_0.blow5 > output.paf
    ```
8. If you wish, you can run the *sigfish-cpu* binary that does not use the hardware accelerator and see how slow it is.
    ```
    ./sigfish-cpu dtw -g test_data/nCoV-2019.reference.fasta -s test_data/reads_0_0.blow5 > output.paf
    ```

## Building
> Warnings:
> - The building of the core acccelerator is not intuitive and require proprietary sowtware from AMD Xilinx.
> - The build steps tested and described below uses the 2021.1 version of Xilinx tools (Vivado and PetaLinux image). For developers with versions lower than 2020.2 you will need to update your tools to a platform supporting Kria.

To build HARU for Xilinx's Kria AI Starter Kit, you will need to build two components:
- Core Accelerator (HDL, build with Vivado)
- Sigfish + driver (C, build with cross-compilation toolchain or build on Kria)

### Prerequisites
- Vitis 2021.1 - we install Vitis so that the Xilinx Command Line Tool (XSTC) is included in the installation
    - Download [Vivado (hw developer) 2021.1 installer](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/2021-1.html). We suggest downloading the Self Extracting Web Installer.
    - Select Vitis during the installation wizard.
- device-tree-xlnx - make sure to checkout the version to align with other tools.
    ```sh
    git clone https://github.com/Xilinx/device-tree-xlnx
    cd device-tree-xlnx
    git checkout xlnx_rel_v2021.1
    ```
- `dtc` - can use a Linux terminal such as BASH (WSL will also work). You may install detc using your package manager, but make sure it is version 1.5 or higher (e.g., `sudo apt install device-tree-compiler` on Ubuntu). If you want to build and install from source:
    ```sh
    git clone https://git.kernel.org/pub/scm/utils/dtc/dtc.git
    cd dtc
    make
    export PATH=$PATH:/<path-to-dtc>/ # optionally, add this to your .bashrc 
    ```
### Core Accelerator
1. Clone the repository if you have not done so.
    ```sh
    git clone https://github.com/beebdev/HARU
    ```
2. Start Vivado, click on create project and follow the prompt to setup project. When selecting parts, navigate to *Boards* and search "kria" in the search bar and select *Kria KV260 Vision AI Starter Kit*.
3. Once the project is created, click on Settings -> General, select *Verilog* as the target language. Navigate to *Bitstream* and tick on *-bin_file* for headerless bitstream to be generated later.
4. Click on *Create Block Design* and provide a name for your design.
5. Under *Sources*, click on *Add Sources* -> select *Add or create design sources*, -> navigate to `<path-to>/HARU/hdl/src/` and select the Verilog files (not including the simulation subdirectory).
6. Add the following IP with the corresponding configurations:
    - **Zynq Ultrascale+ MPSoC**; Run *Block Automation* for board preset, double click to configure and navigate to *PS-PL Configuration* -> *PS-PL Interfaces* -> *Slave Interface* -> *AXI HP* -> enable *AXI HPC0 FPD*.
    - **AXI DMA**; Double click to configure and untick *Enable Scatter Gather Engine*.
7. Right click on the block design diagram and select *Add Module*. Select *dtw_accel* and click OK.
8. Click on *Run Connection Automation* and click OK. This should connect the AXI Lite slaves of the controller for the AXI DMA and dtw_accel modules to Zynq Ultrascale+ MPSoC's master AXI interface. Repeat again to connect the Zynq's other AXI master to the AXI interconnect.
9. Connect the AXI Stream connections between AXI DMA and dtw_accel.
    - Connect `SINK_AXIS` of *dtw_accel* to `S_AXIS_S2MM` of *AXI Direct Memory Access*.
    - Connect `SRC_AXIS` of *dtw_accel* to `M_AXIS_MM2S` of *AXI Direct Memory Access*.
    - Click on *Run Connection Automation* and tick *All Automation* to configure clock of `SRC_AXIS` and `SINK_AXIS`' clock.
10. Under *Sources*, right click on *design_1*, click on *Create HDL Wrapper*, and select *Let Vivado manage wrapper and auto-update*. This will create a Verilog wrapper for the design block configured above. It may take some time to complete and update in the *Sources* window.
11. Right click on the newly generated *design_1_wrapper* under *Sources* and click *Set as Top* .
12. Run synthesis, implementation, and generate bitstream. The `design_1_wrapper.bin` generated under `<path-to-project>/<project-name>/<project-name>.runs/impl_1/` is the headerless bitstream for the accelerator. Rename it into `haru-dtw-firmware.bit.bin`.
13. Click on *File* -> *Export Hardware* -> Select *Pre-synthesis* -> leave name as default (`design_1_wrapper.xsa`) and *Finish*. Note that if you intend to use the PetaLinux tool to generate an image with the accelerator, you need to select *include bitstream*, however, this is not within the scope of this README.
14. Start the **Xilinx Command Line Tool** under start. Navigate to the location of your vivado project and run the following commands:
    ```sh
    cd <path-to-vivado-project>
    hsi open_hw_design design_1_wrapper.xsa
    hsi set_repo_path <path-to>/device-tree-xlnx
    hsi create_sw_design device-tree -os device_tree -proc psu_cortexa53_0
    hsi set_property CONFIG.dt_overlay true [hsi::get_os]
    hsi generate_target -dir haru_dtconfig
    hsi close_hw_design design_1_wrapper
    ```
15. Using the device tree compiler tool `dtc` (either in WSL or other terminals), build the device tree overlay `.dtsi` file into `.dtbo` binary. This will generated the needed device tree overlay for loading your accelerator to the PetaLinux OS during system runtime.
    ```sh
    cd <path-to-vivado-project>/haru_dtconfig
    dtc -@ -O dtb -o haru-dtw-firmware.dtbo pl.dtsi
    ```
16. Transfer the bitstream (`haru-dtw-firmware.bit.bin`) and device tree overlay blob (`haru-dtw-firmware.dtbo`) to your Kria device.
17. On your Kria, create the `haru-dtw-firmware` directory under `/lib/firmware/xilinx/` and copy the bitstream and device tree overlay blob into it.
    ```sh
    mkdir /lib/firmware/xilinx/haru-dtw-firmware
    cp haru-dtw-firmware.bit.bin haru-dtw-firmware.dtbo /lib/firmware/xilinx/haru-dtw-firmware/
    ```
18. In the `haru-dtw-firmware` directory, create a `shell.json` file with the following content:
    ```json
    {
        "shell_type": "XRT_FLAT",
        "num_slots": "1"
    }
    ```
19. Check if `haru-dtw-firmware` is in the list of accelerators and load it.
    ```sh
    xmutil listapps         # List the available accelerators and status
    xmutil unloadapp        # Unload currently loaded accelerators
    xmutil loadapp haru-dtw-firmware # Load haru-dtw-firmware
    xmutil listapps         # List the accelerators and check status for haru

### Sigfish
There are two ways to build sigfish: cross-compilation on another machine, natively build the Kria board.

For cross-compilation, you will need to setup the cross-compilation toolchain for the Kria board, which is included in the release as `sdk.sh`.
```sh
$ <path-to>/sdk.sh
PetaLinux SDK installer version 2021.1_SOM
============================================
Enter target directory for SDK (default: /opt/petalinux/2021.1_SOM): <desired-installation-dir>
You are about to install the SDK to "<desired-installation-dir>". Proceed [Y/n]? Y
```
When you want to cross-compile in a new terminal session, source the following file to setup the environment variables.
```sh
`. <sdk-installation-dir>/environment-setup-cortexa72-cortexa53-xilinx-linux`
echo $CC # to double check the configuration 
```

Steps to build sigfish:

1. Clone the sigfish repo.
    ```sh
    git clone https://github.com/beebdev/sigfish-haru
    git submodule init --update --remote
    ```
2. Source environment script if cross-compiling.
3. Build with `make`.
    ```sh
    # Building sigfish WITHOUT hardware acceleration
    make 

    # Building sigfish WITH hardware acceleration
    make FPGA=1
    ```
4. Run `sigfish` with accelerator loaded (see above for steps).


## For developers

Developers can use our sDTW accelerator core in their own applications. To do so, refer to the driver API as explained [here](https://github.com/beebdev/HARU/tree/main/driver) or browse through the code for [sigfish-haru](https://github.com/beebdev/sigfish-haru). 
