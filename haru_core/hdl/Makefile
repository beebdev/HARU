# Makefile

# defaults
SIM ?= icarus
TOPLEVEL_LANG ?= verilog

ifneq ($(TOPLEVEL_LANG),verilog)
all:
	@echo "Skipping; DTW accel does not support VHDL at the top-level"
clean::

else

ifeq ($(SIM),icarus)
COMPILE_ARGS+=-I$(PWD)/src/
else
COMPILE_ARGS+=+incdir+$(PWD)/src/
endif

#DUT
VERILOG_SOURCES += $(PWD)/src/dtw_accel.v \
				   $(PWD)/src/axi_lite_slave.v \
				   $(PWD)/src/axis_2_fifo.v \
				   $(PWD)/src/fifo.v \
				   $(PWD)/src/fifo_2_axis.v \
				   $(PWD)/src/dtw_core.v \
				   $(PWD)/src/dtw_core_datapath.v \
				   $(PWD)/src/dtw_core_ref_mem.v \
				   $(PWD)/src/dtw_core_pe.v \

#Test Bench
VERILOG_SOURCES += $(PWD)/src/sim/tb_dtw_accel.v

TOPLEVEL = tb_dtw_accel
GPI_IMPL := vpi

MODULE = test_dut

include $(shell cocotb-config --makefiles)/Makefile.sim

# TODO: Add waveform generation

endif
