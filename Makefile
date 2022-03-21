# Makefile

# defaults
SIM ?= icarus
TOPLEVEL_LANG ?= verilog

VERILOG_SOURCES += $(PWD)/src/dtw_accel.v \
				  $(PWD)/src/dtw_accel_M00_AXIS.v \
				  $(PWD)/src/dtw_accel_S00_AXIS.v \
				  $(PWD)/src/dtw_accel_S00_AXI.v \
				  $(PWD)/src/dtw_core.v \
				  $(PWD)/src/dtw_core_ref_mem.v \
				  $(PWD)/src/dtw_core_datapath.v \
				  $(PWD)/src/dtw_core_pe.v \

# use VHDL_SOURCES for VHDL files

# TOPLEVEL is the name of the toplevel module in your Verilog or VHDL file
TOPLEVEL = dtw_accel

# MODULE is the basename of the Python test file
MODULE = test_dtw_accel

# include cocotb's make rules to take care of the simulator setup
include $(shell cocotb-config --makefiles)/Makefile.sim
