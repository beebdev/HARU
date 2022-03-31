# zynq-dtw-accelerator
Accelerator for the Dynamic Time Warping (DTW) algorithm for sequence matching designed to run on the Zynq platform.


# Setup
### Setting up for testing the HDL source:
```
cd zynq-dtw-accelerator
python3 -m venv ./venv
source venv/bin/activate
python3 -m pip install -U pip
pip3 install pytest
pip3 install cocotb
pip3 install cocotbext-axi
```

### Prereq
Install a verilog simulator of your choice. By default we use [Icarus verilog](https://iverilog.fandom.com/wiki/Installation_Guide#Obtaining_Source_From_git).
To install icarus verilog:
```
brew install icarus-verilog
```

Installing gtkwave: (Not integrated yet)
```
# ubuntu
sudo apt-get install gtkwave

# mac
brew install --cask gtkwave
``` 

# Running tests
```
source venv/bin/activate
make
```