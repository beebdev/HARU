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

from axis_driver import AXISSource
from axis_driver import AXISSink
from cocotb_bus.drivers.amba import AXI4LiteMaster
from dtw_accel_driver import DtwAccelDriver

CLK_PERIOD = 2
AXIS_CLK_PERIOD = 2

MODULE_PATH = os.path.join(os.path.dirname(__file__), os.pardir, "rtl")
MODULE_PATH = os.path.abspath(MODULE_PATH)

def setup_dut(dut):
    cocotb.fork(Clock(dut.clk, CLK_PERIOD).start())
    cocotb.fork(Clock(dut.axis_clk, AXIS_CLK_PERIOD).start())

@cocotb.coroutine
def reset_dut(dut):
    dut.rst.value = 1
    dut.axis_rst.value = 1
    yield Timer(CLK_PERIOD * AXIS_CLK_PERIOD * 2)
    dut.rst.value = 0
    dut.axis_rst.value = 0
    yield Timer(CLK_PERIOD * AXIS_CLK_PERIOD * 2)

###############################################################################
## Test read version
###############################################################################
@cocotb.test(skip = True)
def test_read_version(dut):
    """
    Description:
        Read Back the version

    Test ID: 0

    Expected Results:
        Read from the version register
    """
    dut._log.setLevel(logging.WARNING)
    dut.test_id.value = 0
    setup_dut(dut)
    demo_axi_streams = DtwAccelDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
    yield reset_dut(dut)

    # Read the version
    version = yield demo_axi_streams.get_version()
    # Reach into the design to get the actual version value (bypassing AXI)
    dut_version = dut.dut.w_version.value
    dut._log.info ("Dut Version: %s" % dut_version)
    dut._log.info ("Version: 0x%08X" % version)
    yield Timer(CLK_PERIOD * 20)
    dut._log.debug("Done")
    assert dut_version == version

###############################################################################
## Test write control
###############################################################################
@cocotb.test(skip = True)
def test_write_control(dut):
    """
    Description:
        Write the entire control register

    Test ID: 1

    Expected Results:
        Read from the version register
    """
    ## Init
    dut._log.setLevel(logging.WARNING)
    dut.test_id.value = 1
    setup_dut(dut)
    tester = DtwAccelDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
    yield reset_dut(dut)

    ## Body
    my_control = 0x01234567
    yield tester.set_control(my_control)
    assert dut.dut.r_control.value == my_control

    ## Cleanup
    yield Timer(CLK_PERIOD * 20)
    dut._log.debug("Done")

###############################################################################
## Test read control
###############################################################################
@cocotb.test(skip = True)
def test_read_control(dut):
    """
    Description:
        Read the entire control register

    Test ID: 2

    Expected Results:
        Read from the version register
    """
    ## Init
    dut._log.setLevel(logging.WARNING)
    dut.test_id.value = 2
    setup_dut(dut)
    tester = DtwAccelDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
    yield reset_dut(dut)

    ## body
    my_control = 0xFEDCBA98
    dut.dut.r_control.value = my_control
    control = yield tester.get_control()
    assert control == my_control
    
    ## cleanup
    yield Timer(CLK_PERIOD * 20)
    dut._log.info("Done")

###############################################################################
## Test setting and reading ref len
###############################################################################
@cocotb.test(skip = True)
def test_ref_len(dut):
    """
    Description:
        Set the ref_len register and readt the register
    
    Test ID: 3

    Expected Results:
        Write value equals to the read value
    """
    ## Init
    dut._log.setLevel(logging.WARNING)
    dut.test_id.value = 3
    setup_dut(dut)
    tester = DtwAccelDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
    yield reset_dut(dut)

    ## Body
    my_ref_len = 0x12345678
    yield tester.set_ref_len(my_ref_len)
    assert dut.dut.r_ref_len.value == my_ref_len
    read_ref_len = yield tester.get_ref_len()
    assert read_ref_len == my_ref_len

    ## cleanup
    yield Timer(CLK_PERIOD * 20)
    dut._log.debug("Done")

###############################################################################
## Test get key
###############################################################################
@cocotb.test(skip = True)
def test_get_key(dut):
    """
    Description:
        Get the key register
    
    Test ID: 4

    Expected Results:
        key register == 0x0ca7cafe
    """
    ## Init
    dut._log.setLevel(logging.WARNING)
    dut.test_id.value = 4
    setup_dut(dut)
    tester = DtwAccelDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
    yield reset_dut(dut)

    ## Body
    my_key = 0x0ca7cafe
    read_key = yield tester.get_key()
    assert read_key == my_key

    ## cleanup
    yield Timer(CLK_PERIOD * 20)
    dut._log.debug("Done")

###############################################################################
## Test loading the reference
###############################################################################
@cocotb.test(skip = False)
def test_load_ref(dut):
    """
    Description:
        Test the reference loading functionality

    Test ID: 5

    Expected Results:
        After reset ref mem should be all zeros.
        After loading, the reference should be loaded with
        whatever is loaded into dtw-core.
    """
    ## Init
    dut._log.setLevel(logging.WARNING)
    dut.test_id.value = 5
    setup_dut(dut)
    tester = DtwAccelDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
    axis_source = AXISSource(dut, "axis_in", dut.axis_clk, dut.axis_rst)
    yield reset_dut(dut)
    yield tester.core_reset()
    yield axis_source.reset()
    yield Timer(CLK_PERIOD * 10)

    ## Body
    # Check first few cells of ref mem
    t0_ref_mem_0 = dut.dut.dc.inst_dtw_core_ref_mem.MEM[0].value
    assert t0_ref_mem_0 == 0 # before loading it should be zero
    
    # Set opmode
    yield tester.set_opmode(1) # load ref mode
    cr = yield tester.get_control()
    assert dut.dut.w_dtw_core_mode.value == 1
    assert cr == (1 << 2)

    ## Set ref_len
    my_ref_len = 200
    yield tester.set_ref_len(my_ref_len)
    assert dut.dut.r_ref_len.value == my_ref_len
    dut_ref_len = yield tester.get_ref_len()
    assert my_ref_len == dut_ref_len
    # print("Pre-run state: {}".format(dut.dut.dc.r_state.value.integer))

    ## set run
    yield tester.set_rs(1)
    assert dut.dut.w_dtw_core_rs.value == 1
    assert dut.dut.w_src_fifo_empty == 1

    sdata = [[i + 20 for i in range(200)]]
    yield axis_source.send_raw_data(sdata)
    yield Timer(CLK_PERIOD * 250)

    # with open("ref_dbg.txt", "w") as f:
    #     for i in range(250):
    #         f.write("mem[{}]= {}\n".format(i, dut.dut.dc.inst_dtw_core_ref_mem.MEM[i].value.integer))
    for i in range(1, 200):
        # Known issue: the first cell of ref mem is always zero
        assert dut.dut.dc.inst_dtw_core_ref_mem.MEM[i].value.integer == sdata[0][i]
    assert dut.dut.dc.r_load_done.value.integer == 1
    assert dut.dut.dc.r_state.value.integer == 0

    # print("Ref len: {}".format(dut.dut.dc.ref_len.value.integer))
    # print("w ref: {}".format(dut.dut.dc.addrW_ref.value.integer))
    # print("State: {}".format(dut.dut.dc.r_state.value.integer))
    # print("type: {}".format(type(dut.dut.dc.r_state.value)))


###############################################################################
## Test query processing
###############################################################################
@cocotb.test(skip = False)
def test_load_query(dut):
    """
    Description:
        Test the query loading functionality

    Test ID: 6

    Expected Results:
        After reset query mem should be all zeros.
        After loading, the query should be loaded with
        whatever is loaded into dtw-core.
    """
    ## Init
    dut._log.setLevel(logging.WARNING)
    dut.test_id.value = 6
    setup_dut(dut)
    tester = DtwAccelDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
    axis_source = AXISSource(dut, "axis_in", dut.axis_clk, dut.axis_rst)
    axis_sink = AXISSink(dut, "axis_out", dut.axis_clk, dut.axis_rst)
    yield reset_dut(dut)
    yield tester.core_reset()
    yield axis_source.reset()
    yield axis_sink.reset()
    yield Timer(CLK_PERIOD * 10)

    ## Load a larger reference first
    # Set opmode
    yield tester.set_opmode(1) # load ref mode
    cr = yield tester.get_control()
    assert dut.dut.w_dtw_core_mode.value == 1
    assert cr == (1 << 2)

    # Setup ref and query
    ref = [[]]
    query = [[]]
    with open("data/reference.txt", "r") as f:
        for line in f:
            ref[0].append(int(line, base=2))
    with open("data/query_with_id.txt", "r") as f:
        for i, line in enumerate(f):
            query[0].append(int(line, base=2))
            if i == 250:
                break
    query[0].insert(1, 0)
    print("Ref len: {}".format(len(ref[0])))

    # Set ref_len
    yield tester.set_ref_len(len(ref[0]))
    assert dut.dut.r_ref_len.value == len(ref[0])
    assert dut.dut.dc.r_state.value.integer == 0 # Shouldn't have started

    # Set run
    yield tester.set_rs(1)
    assert dut.dut.w_dtw_core_rs.value == 1
    assert dut.dut.w_src_fifo_empty == 1

    # with open("ref_dbg.txt", "w") as f:
    #     for i in range(my_ref_len):
    #         f.write("mem[{}]= {}\n".format(i, sdata[0][i]))
    # with open("query_dbg.txt", "w") as f:
    #     for i in range(len(qdata[0])):
    #         f.write("mem[{}]= {}\n".format(i, qdata[0][i]))

    # send data
    yield axis_source.send_raw_data(ref)
    yield Timer(CLK_PERIOD * (5+len(ref[0]))) # This takes time!

    # Check first few cells of ref mem
    with open("dbg.txt", "w") as f:
        for i in range(len(ref[0])):
            f.write("mem[{}]= {}; {}\n".format(i, dut.dut.dc.inst_dtw_core_ref_mem.MEM[i].value.integer, ref[0][i]))

    # print("addrW_red: {}".format(dut.dut.dc.addrW_ref.value.integer))
    # print("State: {}".format(dut.dut.dc.r_state.value.integer))
    # assert dut.dut.dc.r_load_done.value.integer == 1
    assert dut.dut.dc.r_state.value.integer == 0
    assert dut.dut.w_src_fifo_empty == 1
    # Set opmode
    yield tester.set_opmode(0) # load query mode
    cr = yield tester.get_control()
    assert dut.dut.w_dtw_core_mode.value == 0
    assert dut.dut.w_src_fifo_empty == 1
    # print("state: {}".format(dut.dut.dc.r_state.value.integer))
    # print("running_d: {}".format(dut.dut.dc.inst_dtw_core_datapath.running_d.value))
    # print("squiggle_buffer[1]: {}".format(dut.dut.dc.inst_dtw_core_datapath.Squiggle_Buffer[1].value))
    # print("squiggle_buffer[2]: {}".format(dut.dut.dc.inst_dtw_core_datapath.Squiggle_Buffer[2].value))
    # print("squiggle_buffer[3]: {}".format(dut.dut.dc.inst_dtw_core_datapath.Squiggle_Buffer[3].value))
    # print("Minval: {}".format(dut.dut.dc.inst_dtw_core_datapath.Minval.value.integer))
    # print("Minpos: {}".format(dut.dut.dc.inst_dtw_core_datapath.Minpos.value.integer))
    # print("DTW_lastrow: {}".format(dut.dut.dc.inst_dtw_core_datapath.DTW_lastrow.value.integer))


    print("=====")
    # Start loading query
    yield axis_source.send_raw_data(query)
    # yield Timer(CLK_PERIOD * 7)
    yield Timer(CLK_PERIOD * (len(ref[0])+255))
    print("state: {}".format(dut.dut.dc.r_state.value.integer))
    print("src_fifo_empty: {}".format(dut.dut.w_src_fifo_empty.value))
    print("curr_qid: {}".format(dut.dut.dc.curr_qid.value.integer))
    print("addrR_ref: {}".format(dut.dut.dc.addrR_ref.value.integer))
    print("running_d: {}".format(dut.dut.dc.inst_dtw_core_datapath.running_d.value))
    print("squiggle_buffer[1]: {}".format(dut.dut.dc.inst_dtw_core_datapath.Squiggle_Buffer[1].value.integer))
    print("squiggle_buffer[2]: {}".format(dut.dut.dc.inst_dtw_core_datapath.Squiggle_Buffer[2].value.integer))
    print("squiggle_buffer[249]: {}".format(dut.dut.dc.inst_dtw_core_datapath.Squiggle_Buffer[249].value.integer))
    print("squiggle_buffer[250]: {}".format(dut.dut.dc.inst_dtw_core_datapath.Squiggle_Buffer[250].value.integer))
    print("state: {}".format(dut.dut.dc.r_state.value.integer))
    print("running: {}".format(dut.dut.dc.inst_dtw_core_datapath.running.value))
    print("running_d[250]: {}".format(dut.dut.dc.inst_dtw_core_datapath.running_d[250].value))
    print("cycle_counter: {}".format(dut.dut.dc.inst_dtw_core_datapath.cycle_counter.value.integer))
    print("ref_len: {}".format(dut.dut.dc.inst_dtw_core_datapath.ref_len.value.integer))
    print("done: {}".format(dut.dut.dc.inst_dtw_core_datapath.done.value))
    print("Minpos: {}".format(dut.dut.dc.curr_position.value.integer))
    print("Minval: {}".format(dut.dut.dc.curr_minval.value.integer))
    print("DTW_lastrow: {}".format(dut.dut.dc.inst_dtw_core_datapath.DTW_lastrow.value.integer))

