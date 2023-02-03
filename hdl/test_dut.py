# MIT License

# Copyright (c) 2022 Po Jui Shih
# Copyright (c) 2022 Hassaan Saadat
# Copyright (c) 2022 Sri Parameswaran
# Copyright (c) 2022 Hasindu Gamaarachchi

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from email.mime import base
import os
import sys
import random
import cocotb
import logging
from cocotb.result import TestFailure
from cocotb.clock import Clock
import time
from array import array as Array
from cocotb.triggers import Timer
from cocotb.triggers import RisingEdge
from cocotb.triggers import FallingEdge

from tb.axis_driver import AXISSource
from tb.axis_driver import AXISSink
from cocotb_bus.drivers.amba import AXI4LiteMaster
from tb.zynq_dtw_driver import ZynqDTWDriver

import numpy as np
import ctypes

CLK_PERIOD = 2
AXIS_CLK_PERIOD = 2

MODULE_PATH = os.path.join(os.path.dirname(__file__), os.pardir, "rtl")
MODULE_PATH = os.path.abspath(MODULE_PATH)

def setup_dut(dut):
    cocotb.fork(Clock(dut.S_AXI_clk, CLK_PERIOD).start())
    cocotb.fork(Clock(dut.S_AXIS_clk, AXIS_CLK_PERIOD).start())

@cocotb.coroutine
def reset_dut(dut):
    dut.S_AXI_rst.value = 1
    dut.S_AXIS_rst.value = 1
    yield Timer(CLK_PERIOD * AXIS_CLK_PERIOD * 2)
    dut.S_AXI_rst.value = 0
    dut.S_AXIS_rst.value = 0
    yield Timer(CLK_PERIOD * AXIS_CLK_PERIOD * 2)

###############################################################################
## Version check
###############################################################################
@cocotb.test(skip = False)
def test_read_version(dut):
    dut._log.setLevel(logging.WARNING)
    dut.test_id.value = 0
    setup_dut(dut)
    zynq_dtw = ZynqDTWDriver(dut, "S_AXI", dut.S_AXI_clk, dut.S_AXI_rst, debug = False)
    yield reset_dut(dut)


    # Read the version register
    version = yield zynq_dtw.get_version()
    # print ("[DEBUG INFO] Version: 0x%08X" % version)
    yield Timer(CLK_PERIOD * 20)

    ## Bit 31:28 - Major version (0)
    assert version >> 28 == 0
    ## Bit 27:20 - Minor version (0)
    assert (version >> 20) & 0xFF == 0
    ## Bit 19:16 - Revision version (2)
    assert (version >> 16) & 0xF == 2
    ## Bit 15:0  - Version page range (0x0000)

###############################################################################
## Key check
###############################################################################
@cocotb.test(skip = False)
def test_read_key(dut):
    dut._log.setLevel(logging.WARNING)
    dut.test_id.value = 0
    setup_dut(dut)
    zynq_dtw = ZynqDTWDriver(dut, "S_AXI", dut.S_AXI_clk, dut.S_AXI_rst, debug = False)
    yield reset_dut(dut)

    # Read the key register
    key = yield zynq_dtw.get_key()
    # print ("[DEBUG INFO] Key: 0x%08X" % key)
    yield Timer(CLK_PERIOD * 20)
    # Expected value: 0x0ca7cafe
    assert key == 0x0ca7cafe

###############################################################################
## Control register (general)
###############################################################################
@cocotb.test(skip = False)
def test_control_reg(dut):
    setup_dut(dut)
    zynq_dtw = ZynqDTWDriver(dut, "S_AXI", dut.S_AXI_clk, dut.S_AXI_rst, debug = False)
    yield reset_dut(dut)

    # Read the control register; expected to be 0
    control = yield zynq_dtw.get_control()
    assert control == 0

    # Set the control register
    yield zynq_dtw.set_control(0x12345678)
    yield Timer(CLK_PERIOD)
    assert dut.dut.reg_0_control.value == 0x12345678
    
###############################################################################
## Ref_len register
###############################################################################
@cocotb.test(skip = False)
def test_ref_len_reg(dut):
    setup_dut(dut)
    zynq_dtw = ZynqDTWDriver(dut, "S_AXI", dut.S_AXI_clk, dut.S_AXI_rst, debug = False)
    yield reset_dut(dut)

    # Write the ref_len register
    yield zynq_dtw.set_ref_len(0x1000)
    yield Timer(CLK_PERIOD)
    assert dut.dut.reg_4_ref_len.value == 0x1000
    assert dut.dut.dtw_core_reference_len.value == 0x1000
    assert dut.dut.inst_dtw_core.dtw_core_reference_len.value == 0x1000
    assert dut.dut.inst_dtw_core.inst_dtw_core_datapath.reference_len.value == dut.dut.reg_4_ref_len.value
    ref_len = yield zynq_dtw.get_ref_len()
    assert ref_len == 0x1000

###############################################################################
## Search
###############################################################################
def print_axis_signals(dut, string):
    print("=====================================")
    print(string + " -v")
    print("tvalid, tready, tlast, tdata")
    print([dut.dut.S_AXIS_tvalid.value, dut.dut.S_AXIS_tready.value, dut.dut.S_AXIS_tlast.value, dut.dut.S_AXIS_tdata.value])
    print("fifo_data, fifo_w_stb, fifo_not_full")
    print([dut.dut.s_axis_fifo_w_data.value, dut.dut.s_axis_fifo_w_stb.value, dut.dut.s_axis_fifo_not_full.value])
    
    print("[!] FIFO rst: " + str(dut.dut.src_fifo.rst.value))
    print("[!] oops: " + str(dut.dut.src_fifo.dbg_oops.value))
    print("FIFO W data: " + str(dut.dut.src_fifo.i_fifo_w_data.value))
    print("FIFO W stb: " + str(dut.dut.src_fifo.i_fifo_w_stb.value))
    print("FIFO R data: " + str(dut.dut.src_fifo.o_fifo_r_data.value))
    print("FIFO R stb: " + str(dut.dut.src_fifo.i_fifo_r_stb.value))
    print("FIFO full: " + str(dut.dut.src_fifo.o_fifo_full.value))
    print("FIFO empty: " + str(dut.dut.src_fifo.o_fifo_empty.value))
    print("FIFO not full: " + str(dut.dut.src_fifo.o_fifo_not_full.value))
    print("FIFO not empty: " + str(dut.dut.src_fifo.o_fifo_not_empty.value))
    print("FIFO read pointer: " + str(dut.dut.src_fifo.read_ptr.value))
    print("FIFO write pointer: " + str(dut.dut.src_fifo.write_ptr.value))
    print("FIFO MEM[write_ptr]: " + str(dut.dut.src_fifo.MEM[dut.dut.src_fifo.write_ptr.value.integer].value))
    print("FIFO content: ")
    print("[0] - "+ str(dut.dut.src_fifo.MEM[0].value))
    print("[1] - "+ str(dut.dut.src_fifo.MEM[1].value))
    print("[2] - "+ str(dut.dut.src_fifo.MEM[2].value))
    print("[3] - "+ str(dut.dut.src_fifo.MEM[3].value))
    print("=====================================")
    
@cocotb.test(skip = False)
def test_dtw_core_control(dut):
    setup_dut(dut)
    zynq_dtw = ZynqDTWDriver(dut, "S_AXI", dut.S_AXI_clk, dut.S_AXI_rst, debug = False)
    src_axis = AXISSource(dut, "S_AXIS", dut.S_AXIS_clk, dut.S_AXIS_rst)
    yield reset_dut(dut)
    yield src_axis.reset()

    # Set ZynqDTW control register's reset bit
    yield zynq_dtw.set_control(0x00000001)
    yield Timer(CLK_PERIOD)
    yield zynq_dtw.set_control(0x00000000)
    yield Timer(CLK_PERIOD)

    # Set the ref_len register
    yield zynq_dtw.set_ref_len(0x1000)
    yield Timer(CLK_PERIOD)
    # print_axis_signals(dut)
    
    # Start the search
    assert dut.dut.inst_dtw_core.r_state.value == 0
    yield zynq_dtw.set_control(0x00000002) # second bit is the rs bit
    yield Timer(CLK_PERIOD)

    # Check the state; expected to be 1
    assert dut.dut.inst_dtw_core.r_state.value == 1

    # Send in query data of type uint32
    query_data = [[i+20, 0, 0, 0] for i in range(0, 250)]
    
    yield src_axis.send_raw_data(query_data)
    yield Timer(CLK_PERIOD *1)
    print_axis_signals(dut, "1 clk")

    yield Timer(CLK_PERIOD *1)
    print_axis_signals(dut, "2 clk")
    yield Timer(CLK_PERIOD *1)
    print_axis_signals(dut, "3 clk")
    
    
    # yield Timer(CLK_PERIOD * 100)
    # print(dut.dut.src_fifo.write_ptr.value)
    # print(dut.dut.src_fifo.read_ptr.value)
    # print(dut.dut.inst_dtw_core.r_state.value)
    # print(dut.dut.inst_dtw_core.n_loaded_query.value.integer)
    # print(dut.dut.inst_dtw_core.dtw_core_squiggle_word.value)
    # print(dut.dut.inst_dtw_core.inst_dtw_core_datapath.sqb_address.value.integer)
    # # TODO: stuck in src_fifo_empty being empty


# ###############################################################################
# ## Test loading the reference
# ###############################################################################
# @cocotb.test(skip = False)
# def test_load_ref(dut):
#     ## Init
#     dut._log.setLevel(logging.WARNING)
#     dut.test_id.value = 5
#     setup_dut(dut)
#     tester = ZynqDTWDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
#     axis_source = AXISSource(dut, "axis_in", dut.axis_clk, dut.axis_rst)
#     yield reset_dut(dut)
#     yield tester.core_reset()
#     yield axis_source.reset()
#     yield Timer(CLK_PERIOD * 10)

#     ## Body
#     # Check first few cells of ref mem
#     # t0_ref_mem_0 = dut.dut.dc.inst_dtw_core_ref_mem.MEM[0].value
#     # assert t0_ref_mem_0 == 0 # before loading it should be zero
    
#     # Set opmode
#     yield tester.set_opmode(1) # load ref mode
#     cr = yield tester.get_control()
#     assert dut.dut.w_dtw_core_mode.value == 1
#     assert cr == (1 << 2)

#     ## Set ref_len
#     my_ref_len = 200
#     yield tester.set_ref_len(my_ref_len)
#     assert dut.dut.r_ref_len.value == my_ref_len
#     dut_ref_len = yield tester.get_ref_len()
#     assert my_ref_len == dut_ref_len

#     ## set run
#     yield tester.set_rs(1)
#     assert dut.dut.w_dtw_core_rs.value == 1
#     assert dut.dut.w_src_fifo_empty == 1

#     sdata = [[i + 20 for i in range(200)]]
#     yield axis_source.send_raw_data(sdata)
#     yield Timer(CLK_PERIOD * 250)

#     for i in range(1, 200):
#         # Known issue: the first cell (index=0) of ref mem is always zero
#         assert dut.dut.dc.inst_dtw_core_ref_mem.MEM[i].value.integer == sdata[0][i]
#     assert dut.dut.dc.r_load_done.value.integer == 1
#     assert dut.dut.dc.r_state.value.integer == 0

#     # print("Ref len: {}".format(len(dut.dut.dc.inst_dtw_core_ref_mem.MEM)))
#     # for i in range(200):
#     #     try:
#     #         print(dut.dut.dc.inst_dtw_core_ref_mem.MEM[i].value.integer)
#     #     except:
#     #         print(dut.dut.dc.inst_dtw_core_ref_mem.MEM[i].value)


# ###############################################################################
# ## Test query processing
# ###############################################################################
# @cocotb.test(skip = True)
# def test_load_query(dut):
#     """
#     Description:
#         Test the query loading functionality

#     Test ID: 6

#     Expected Results:
#         After reset query mem should be all zeros.
#         After loading, the query should be loaded with
#         whatever is loaded into dtw-core.
#     """
#     ## Init
#     dut._log.setLevel(logging.WARNING)
#     dut.test_id.value = 6
#     setup_dut(dut)
#     tester = ZynqDTWDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
#     axis_source = AXISSource(dut, "axis_in", dut.axis_clk, dut.axis_rst)
#     axis_sink = AXISSink(dut, "axis_out", dut.axis_clk, dut.axis_rst)
#     yield reset_dut(dut)
#     yield tester.core_reset()
#     yield axis_source.reset()
#     yield axis_sink.reset()
#     yield Timer(CLK_PERIOD * 10)

#     ## Load a larger reference first
#     # Set opmode
#     yield tester.set_opmode(1) # load ref mode
#     cr = yield tester.get_control()
#     assert dut.dut.w_dtw_core_mode.value == 1
#     assert cr == (1 << 2)

#     # Setup ref and query
#     ref = [[]]
#     query = [[]]
#     with open("data/reference.txt", "r") as f:
#         for line in f:
#             ref[0].append(int(line, base=2))
#     with open("data/query_with_id.txt", "r") as f:
#         for i, line in enumerate(f):
#             query[0].append(int(line, base=2))
#             if i == 250:
#                 break
#     query[0].insert(1, 0)
#     # print("Ref len: {}".format(len(ref[0])))

#     with open("ref_dbg.txt", "w") as f:
#         for i in range(len(ref[0])):
#             f.write("mem[{}]= {}\n".format(i, ref[0][i]))
#     with open("query_dbg.txt", "w") as f:
#         for i in range(len(query[0])):
#             f.write("mem[{}]= {}\n".format(i, query[0][i]))

#     # Set ref_len
#     yield tester.set_ref_len(len(ref[0]))
#     assert dut.dut.r_ref_len.value == len(ref[0])
#     assert dut.dut.dc.r_state.value.integer == 0 # Shouldn't have started

#     # Set run
#     yield tester.set_rs(1)
#     assert dut.dut.w_dtw_core_rs.value == 1
#     assert dut.dut.w_src_fifo_empty == 1

#     # send data
#     yield axis_source.send_raw_data(ref)
#     yield Timer(CLK_PERIOD * (5+len(ref[0]))) # This takes time!
#     assert dut.dut.dc.r_state.value.integer == 0
#     assert dut.dut.w_src_fifo_empty == 1

#     # Set opmode
#     yield tester.set_opmode(0) # load query mode
#     cr = yield tester.get_control()
#     assert dut.dut.w_dtw_core_mode.value == 0
#     assert dut.dut.w_src_fifo_empty == 1

#     # Start loading query
#     cocotb.fork(axis_sink.receive())
#     yield axis_source.send_raw_data(query)
#     yield Timer(CLK_PERIOD * (261 + len(ref[0]))) # This takes time!

#     rdata = axis_sink.read_data()
#     assert len(rdata) == 1
#     assert len(rdata[0]) == 3
#     assert rdata[0][0] == 3
#     assert rdata[0][1] == 24466
#     assert rdata[0][2] == 2639

# ###############################################################################
# ## Test query processing
# ###############################################################################
# @cocotb.test(skip = True)
# def test_load_query_multiple(dut):
#     """
#     Description:
#         Test the query loading functionality

#     Test ID: 6

#     Expected Results:
#         After reset query mem should be all zeros.
#         After loading, the query should be loaded with
#         whatever is loaded into dtw-core.
#     """
#     ## Init
#     dut._log.setLevel(logging.WARNING)
#     dut.test_id.value = 6
#     setup_dut(dut)
#     tester = ZynqDTWDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
#     axis_source = AXISSource(dut, "axis_in", dut.axis_clk, dut.axis_rst)
#     axis_sink = AXISSink(dut, "axis_out", dut.axis_clk, dut.axis_rst)
#     yield reset_dut(dut)
#     yield tester.core_reset()
#     yield axis_source.reset()
#     yield axis_sink.reset()
#     yield Timer(CLK_PERIOD * 10)

#     ## Load a larger reference first
#     # Set opmode
#     yield tester.set_opmode(1) # load ref mode
#     cr = yield tester.get_control()
#     assert dut.dut.w_dtw_core_mode.value == 1
#     assert cr == (1 << 2)

#     # Setup ref and query
#     ref = [[]]
#     query = [[]]
#     with open("data/reference.txt", "r") as f:
#         for line in f:
#             ref[0].append(int(line, base=2))
#     with open("data/query_with_id.txt", "r") as f:
#         for i, line in enumerate(f):
#             query[0].append(int(line, base=2))
#             if i == 250:
#                 break
#     query[0].insert(1, 0)
#     # print("Ref len: {}".format(len(ref[0])))

#     # Set ref_len
#     yield tester.set_ref_len(len(ref[0]))
#     assert dut.dut.r_ref_len.value == len(ref[0])
#     assert dut.dut.dc.r_state.value.integer == 0 # Shouldn't have started

#     # Set run
#     yield tester.set_rs(1)
#     assert dut.dut.w_dtw_core_rs.value == 1
#     assert dut.dut.w_src_fifo_empty == 1

#     # send data
#     yield axis_source.send_raw_data(ref)
#     yield Timer(CLK_PERIOD * (5+len(ref[0]))) # This takes time!
#     assert dut.dut.dc.r_state.value.integer == 0
#     assert dut.dut.w_src_fifo_empty == 1

#     # Set opmode
#     yield tester.set_opmode(0) # load query mode
#     cr = yield tester.get_control()
#     assert dut.dut.w_dtw_core_mode.value == 0
#     assert dut.dut.w_src_fifo_empty == 1

#     # Start loading query
#     cocotb.fork(axis_sink.receive())
#     cocotb.fork(axis_sink.receive())
#     yield axis_source.send_raw_data(query)
#     yield Timer(CLK_PERIOD * (261 + len(ref[0]))) # This takes time!
#     yield axis_source.send_raw_data(query)
#     yield Timer(CLK_PERIOD * (261 + len(ref[0]))) # This takes time!
#     # yield Timer(CLK_PERIOD * 7)
#     rdata = axis_sink.read_data()
#     assert len(rdata) == 2
#     assert len(rdata[0]) == 3
#     assert len(rdata[1]) == 3
#     assert rdata[0][0] == 3
#     assert rdata[0][1] == 24466
#     assert rdata[0][2] == 2639
#     assert rdata[1][0] == 3
#     assert rdata[1][1] == 24466
#     assert rdata[1][2] == 2639

# ###############################################################################
# ## Test query processing
# ###############################################################################
# @cocotb.test(skip = False)
# def test_load_small_query(dut):
#     """
#     Description:
#         Test the query loading functionality

#     Test ID: 6

#     Expected Results:
#         After reset query mem should be all zeros.
#         After loading, the query should be loaded with
#         whatever is loaded into dtw-core.
#     """
#     ## Init
#     dut._log.setLevel(logging.WARNING)
#     dut.test_id.value = 6
#     setup_dut(dut)
#     tester = ZynqDTWDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
#     axis_source = AXISSource(dut, "axis_in", dut.axis_clk, dut.axis_rst)
#     axis_sink = AXISSink(dut, "axis_out", dut.axis_clk, dut.axis_rst)
#     yield reset_dut(dut)
#     yield tester.core_reset()
#     yield axis_source.reset()
#     yield axis_sink.reset()
#     yield Timer(CLK_PERIOD * 10)

#     ## Load a larger reference first
#     # Set opmode
#     yield tester.set_opmode(1) # load ref mode
#     cr = yield tester.get_control()
#     assert dut.dut.w_dtw_core_mode.value == 1
#     assert cr == (1 << 2)

#     # Setup ref and query
#     ref = [[]]
#     query = [[]]

#     # with open("data/ref_dbg.txt", "w") as f:
#     #     for i in range(2000):
#     #         ref[0].append(i)
#     #         f.write("ref[{}] = {}\n".format(i, ref[0][i]))
            
#     # with open("data/query_dbg.txt", "w") as f:
#     #     for i in range(252):
#     #         query[0].append(i)
#     #         f.write("query[{}] = {}\n".format(i, query[0][i]))

#     for i in range(4000):
#         # ref[0].append(random.randint(0, 10))
#         ref[0].append(i%1000)
    
#     query[0].append(1)
#     query[0].append(0)
#     for i in range(250):
#         query[0].append(ref[0][i+200])


#     # Set ref_len
#     yield tester.set_ref_len(len(ref[0]))
#     assert dut.dut.r_ref_len.value == len(ref[0])
#     assert dut.dut.dc.r_state.value.integer == 0 # Shouldn't have started

#     # Set run
#     yield tester.set_rs(1)
#     assert dut.dut.w_dtw_core_rs.value == 1
#     assert dut.dut.w_src_fifo_empty == 1

#     # send data
#     yield axis_source.send_raw_data(ref)
#     yield Timer(CLK_PERIOD * (5+len(ref[0]))) # This takes time!
#     assert dut.dut.dc.r_state.value.integer == 0
#     assert dut.dut.w_src_fifo_empty == 1

#     assert dut.dut.dc.r_state.value.integer == 0

#     # Set opmode
#     yield tester.set_opmode(0) # load query mode
#     assert dut.dut.dc.r_state.value.integer == 2
#     cr = yield tester.get_control()
#     assert dut.dut.w_dtw_core_mode.value == 0
#     assert dut.dut.w_src_fifo_empty == 1
#     yield Timer(CLK_PERIOD * (261 + len(ref[0]))) # This takes time!
#     assert dut.dut.dc.r_state.value.integer == 2
#     assert dut.dut.w_src_fifo_empty == 1

#     # Start loading query
#     cocotb.fork(axis_sink.receive())
#     yield axis_source.send_raw_data(query)
#     yield Timer(CLK_PERIOD * (262 + len(ref[0]))) # This takes time!
#     # yield Timer(CLK_PERIOD * 7)
#     print("state: {}".format(dut.dut.dc.r_state.value.integer))
#     print("running: {}".format(dut.dut.dc.inst_dtw_core_datapath.running.value))
#     print("running_d: {}".format(dut.dut.dc.inst_dtw_core_datapath.running_d.value))
#     print("cycle_counter: {}".format(dut.dut.dc.inst_dtw_core_datapath.cycle_counter.value.integer))
#     print("ref_len: {}".format(dut.dut.dc.inst_dtw_core_datapath.ref_len.value.integer))
    
#     print("load_done: {}".format(dut.dut.dc.r_load_done.value))
#     print("addr_ref: {}".format(dut.dut.dc.addr_ref.value.integer))
    
#     dtw_dp = dut.dut.dc.inst_dtw_core_datapath
#     # for i in range(80):    
#     #     yield Timer(CLK_PERIOD * 50)

#     #     print("============")
#     #     print("running: {}".format(dtw_dp.running.value))
#     #     print("Input_squiggle: {}".format(dtw_dp.Input_squiggle.value.integer))
#     #     print("Rword: {}".format(dtw_dp.Rword.value.integer))
#     #     print("ref_len: {}".format(dtw_dp.ref_len.value.integer))
#     #     print("minval: {}".format(dtw_dp.minval.value.integer))
#     #     try:
#     #         print("DTW_lastrow: {}".format(dtw_dp.DTW_lastrow.value.integer))
#     #     except:
#     #         print("DTW_lastrow: {}".format(dtw_dp.DTW_lastrow.value))

#     #     print("position: {}".format(dtw_dp.position.value.integer))
#     #     print("done: {}".format(dtw_dp.done.value))

#     rdata = axis_sink.read_data()
#     print("rdata: {}".format(rdata))
#     assert len(rdata) == 1
#     assert len(rdata[0]) == 3
#     assert rdata[0][0] == 1
#     assert rdata[0][1] == 450
#     assert rdata[0][2] == 0