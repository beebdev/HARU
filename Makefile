# Makefile

# defaults
SIM ?= icarus
TOPLEVEL_LANG ?= verilog

ifneq ($(TOPLEVEL_LANG),verilog)
all:
	@echo "Skipping; DTW accel does not support VHDL at the top-level"
clean::

else

#DUT
VERILOG_SOURCES += $(PWD)/src/dtw_accel.v \
				   $(PWD)/src/axi_lite_slave.v \
				   $(PWD)/src/axis_2_fifo.v \
				   $(PWD)/src/fifo.v \
				   $(PWD)/src/fifo_2_axis.v \

#Test Bench
VERILOG_SOURCES += $(PWD)/src/sim/tb_dtw_accel.v

TOPLEVEL = tb_dtw_accel
GPI_IMPL := vpi

MODULE = test_dut

include $(shell cocotb-config --makefiles)/Makefile.sim

endif
