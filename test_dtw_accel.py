# This file is public domain, it can be freely copied without restrictions.
# SPDX-License-Identifier: CC0-1.0

# test_my_design.py (simple)

import cocotb
from cocotb.triggers import Timer
from cocotbext.axi import AxiLiteBus, AxiLiteMaster
from cocotbext.axi import AxiStreamBus, AxiStreamSource, AxiStreamSink

async def generate_clock(dut):
    """Generate clock pulses."""
    for cycle in range(10):
        dut.aclk.value = 0
        await Timer(1, units="ns")
        dut.aclk.value = 1
        await Timer(1, units="ns")

@cocotb.test(0)
async def my_first_test(dut):
    """Try accessing the design."""
    axilite_m = AxiLiteMaster(AxiLiteBus.from_prefix(dut, "s00_axi"), dut.aclk, dut.aresetn)
    axis_source = AxiStreamSource(AxiStreamBus.from_prefix(dut, "s00_axis"), dut.aclk, dut.aresetn)
    axis_sink = AxiStreamSink(AxiStreamBus.from_prefix(dut, "m00_axis"), dut.aclk, dut.aresetn)

    await cocotb.start(generate_clock(dut))  # run the clock "in the background"

    # dut._log.info("my_signal_1 is %s", dut.my_signal_1.value)
    dut._log.info("s00_dtw_cr is %s", dut.s00_dtw_cr.value)
    assert 1 == 1, "my_first_test failed"


# # test_my_design.py (extended)

# import cocotb
# from cocotb.triggers import Timer
# from cocotb.triggers import FallingEdge




# @cocotb.test()
# async def my_second_test(dut):
#     """Try accessing the design."""

#     await cocotb.start(generate_clock(dut))  # run the clock "in the background"

#     await Timer(5, units="ns")  # wait a bit
#     await FallingEdge(dut.clk)  # wait for falling edge/"negedge"

#     dut._log.info("my_signal_1 is %s", dut.my_signal_1.value)
#     assert dut.my_signal_2.value[0] == 0, "my_signal_2[0] is not 0!"
