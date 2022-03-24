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

@cocotb.test(skip = False)
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

@cocotb.test(skip = False)
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

@cocotb.test(skip = False)
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

@cocotb.test(skip = False)
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

@cocotb.test(skip = False)
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
    yield axis_source.reset()
    yield axis_sink.reset()
    yield Timer(CLK_PERIOD * 10)

    ## Body
    t0_query_mem_0 = dut.dut.dc.inst_dtw_core_query_mem.MEM[0].value
    assert t0_query_mem_0 == 0


# @cocotb.test(skip = False)
# def test_axis_write(dut):
#     """
#     Description:
#         Read the entire control register

#     Test ID: 5

#     Expected Results:
#         Read from the version register
#     """
#     dut._log.setLevel(logging.WARNING)
#     dut.test_id.value = 5
#     setup_dut(dut)
#     demo_axi_streams = DtwAccelDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
#     axis_source = AXISSource(dut, "axis_in", dut.axis_clk, dut.axis_rst)
#     yield reset_dut(dut)

#     yield axis_source.reset()
#     yield RisingEdge(dut.clk)
#     #yield reset_dut(dut)
#     data = [range(16)]
#     yield axis_source.send_raw_data(data)

#     yield Timer(CLK_PERIOD * 100)


'''
A note about source idles and sink back pressure.

When generating the source and sink idle the values inserted do not always
match up with the data, instead the values will repeat, for example
if you were to insert a source idle at 0th clock cycle in a list that
is 10 elements long and then you used a timer to delay the start by
5 clock cycles before starting a transaction the idle will happen on the
10th CLOCK cycle but will happen on the 6th cycle of the data transaction
'''

# @cocotb.test(skip = False)
# def test_axis_write_and_read(dut):
#     """
#     Description:
#         Read the entire control register

#     Test ID: 6

#     Expected Results:
#         Read from the version register
#     """
#     dut._log.setLevel(logging.WARNING)
#     #dut._log.setLevel(logging.INFO)
#     DATA_COUNT = 16
#     dut.test_id.value = 6
#     setup_dut(dut)
#     demo_axi_streams = DtwAccelDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
#     axis_source = AXISSource(dut, "axis_in",  dut.axis_clk, dut.axis_rst)
#     axis_sink   = AXISSink  (dut, "axis_out", dut.axis_clk, dut.axis_rst)
#     yield reset_dut(dut)
#     #yield axis_source.reset()
#     #yield axis_sink.reset()
#     sdata = [list(range(DATA_COUNT))]

#     cocotb.fork(axis_sink.receive())
#     yield Timer(AXIS_CLK_PERIOD * CLK_PERIOD * 20)
#     yield axis_source.send_raw_data(sdata)
#     yield Timer(AXIS_CLK_PERIOD * CLK_PERIOD * 20)
#     rdata = axis_sink.read_data()
#     assert len(rdata) == len(sdata)
#     for i in range(len(rdata)):
#         assert len(rdata[i]) == len(sdata[i])

#     for i in range(len(rdata)):
#         for j in rdata[i]:
#             assert rdata[i][j] == sdata[i][j]


# @cocotb.test(skip = False)
# def test_axis_write_and_read_with_source_idle(dut):
#     """
#     Description:
#         Read the entire control register

#     Test ID: 7

#     Expected Results:
#         Read from the version register
#     """
#     dut._log.setLevel(logging.WARNING)
#     #dut._log.setLevel(logging.INFO)
#     DATA_COUNT = 16
#     dut.test_id.value = 7
#     setup_dut(dut)
#     demo_axi_streams = DtwAccelDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
#     axis_source = AXISSource(dut, "axis_in",  dut.axis_clk, dut.axis_rst)
#     axis_sink   = AXISSink  (dut, "axis_out", dut.axis_clk, dut.axis_rst)
#     yield reset_dut(dut)
#     #yield axis_source.reset()
#     #yield axis_sink.reset()
#     sdata = [list(range(DATA_COUNT))]
#     idle_list = [0] * DATA_COUNT
#     idle_list[1] = 1
#     idle_list[2] = 1
#     idle_list[4] = 1
#     idle_list[9] = 1

#     axis_source.insert_idle_list(idle_list)

#     cocotb.fork(axis_sink.receive())
#     yield Timer(AXIS_CLK_PERIOD * CLK_PERIOD * 20)
#     yield axis_source.send_raw_data(sdata)
#     yield Timer(AXIS_CLK_PERIOD * CLK_PERIOD * 20)
#     rdata = axis_sink.read_data()
#     assert len(rdata) == len(sdata)
#     for i in range(len(rdata)):
#         assert len(rdata[i]) == len(sdata[i])

#     for i in range(len(rdata)):
#         for j in rdata[i]:
#             assert rdata[i][j] == sdata[i][j]


# @cocotb.test(skip = False)
# def test_axis_write_and_read_with_sink_back_preassure(dut):
#     """
#     Description:
#         Read the entire control register

#     Test ID: 8

#     Expected Results:
#         Read from the version register
#     """
#     dut._log.setLevel(logging.WARNING)
#     #dut._log.setLevel(logging.INFO)
#     DATA_COUNT = 16
#     dut.test_id.value = 8
    # setup_dut(dut)
    # demo_axi_streams = DtwAccelDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
    # axis_source = AXISSource(dut, "axis_in",  dut.axis_clk, dut.axis_rst)
    # axis_sink   = AXISSink  (dut, "axis_out", dut.axis_clk, dut.axis_rst)
    # yield reset_dut(dut)
    # #yield axis_source.reset()
    # #yield axis_sink.reset()

    # sdata = [list(range(DATA_COUNT))]

    # # Adjust sink back pressure here
    # bp_list = [0] * DATA_COUNT

    # bp_list[DATA_COUNT - 1] = 1
    # # Apply back pressure after the 2nd value is read
    # bp_list[2] = 1
    # axis_sink.insert_backpreassure_list(bp_list)

    # cocotb.fork(axis_sink.receive())
    # yield RisingEdge(dut.clk)
    # yield Timer(AXIS_CLK_PERIOD * CLK_PERIOD * 20)
    # yield axis_source.send_raw_data(sdata)
    # yield Timer(AXIS_CLK_PERIOD * CLK_PERIOD * 20)
    # rdata = axis_sink.read_data()
    # assert len(rdata) == len(sdata)
    # for i in range(len(rdata)):
    #     assert len(rdata[i]) == len(sdata[i])

    # for i in range(len(rdata)):
    #     for j in rdata[i]:
    #         assert rdata[i][j] == sdata[i][j]

# @cocotb.test(skip = False)
# def test_axis_write_and_read_with_sink_idle_and_back_preassure(dut):
#     """
#     Description:
#         Read the entire control register

#     Test ID: 9

#     Expected Results:
#         Read from the version register
#     """
#     dut._log.setLevel(logging.WARNING)
#     #dut._log.setLevel(logging.INFO)
#     DATA_COUNT = 16
#     dut.test_id.value = 9
#     setup_dut(dut)
#     demo_axi_streams = DtwAccelDriver(dut, "aximl", dut.clk, dut.rst, debug = False)
#     axis_source = AXISSource(dut, "axis_in",  dut.axis_clk, dut.axis_rst)
#     axis_sink   = AXISSink  (dut, "axis_out", dut.axis_clk, dut.axis_rst)
#     yield reset_dut(dut)

#     sdata = [list(range(DATA_COUNT))]

#     # Adjust sink back pressure here
#     bp_list = [0] * DATA_COUNT

#     bp_list[DATA_COUNT - 1] = 1
#     # Apply back pressure after the 2nd value is read
#     bp_list[2] = 1
#     # Apply back pressure after the 4th value is read
#     #bp_list[4] = 1


#     # Adjust source idle here
#     idle_list = [0] * DATA_COUNT
#     # Insert an IDLE at clock 1
#     idle_list[1] = 1

#     axis_sink.insert_backpreassure_list(bp_list)
#     axis_source.insert_idle_list(idle_list)

#     cocotb.fork(axis_sink.receive())

#     yield Timer(AXIS_CLK_PERIOD * CLK_PERIOD * 20)
#     yield axis_source.send_raw_data(sdata)
#     yield Timer(AXIS_CLK_PERIOD * CLK_PERIOD * 20)
#     rdata = axis_sink.read_data()
#     assert len(rdata) == len(sdata)
#     for i in range(len(rdata)):
#         assert len(rdata[i]) == len(sdata[i])

#     for i in range(len(rdata)):
#         for j in rdata[i]:
#             assert rdata[i][j] == sdata[i][j]

