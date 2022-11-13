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

import logging
import itertools
import time
import cocotb
from array import array as Array

from cocotb.clock import Clock
from cocotb.result import TestFailure
from cocotb.triggers import Timer
from cocotb.triggers import RisingEdge
from cocotb.triggers import FallingEdge

from cocotbext.axi import AxiStreamFrame
from cocotbext.axi import AxiStreamBus
from cocotbext.axi import AxiStreamSource
from cocotbext.axi import AxiStreamSink
from cocotbext.axi import AxiStreamMonitor

class AXISSource:

    def __init__(self, dut, BUS_NAME, clk, rst):
        self.dut = dut
        self.clk = clk
        self.rst = rst
        self.cycle_list = None
        self.log = logging.getLogger("cocotb.tb")
        self.log.setLevel(logging.DEBUG)
        self.cur_id = 1

        self.source = AxiStreamSource(AxiStreamBus.from_prefix(dut, BUS_NAME), clk, rst)

    async def reset(self):
        self.rst.setimmediatevalue(0)
        await RisingEdge(self.clk)
        await RisingEdge(self.clk)
        self.rst.value = 1
        await RisingEdge(self.clk)
        await RisingEdge(self.clk)
        self.rst.value = 0
        await RisingEdge(self.clk)
        await RisingEdge(self.clk)

    def insert_idle_list(self, cycle_list = None):
        if type(cycle_list) is not list:
            print ("Cycle List needs to be a list")
        self.source.set_pause_generator(itertools.cycle(cycle_list))

    async def send_raw_data(self, data_list):
        for data in data_list:
            d = AxiStreamFrame(data, tid = self.cur_id, tdest = self.cur_id)
            await self.source.send(d)
            self.cur_id += 1

    async def send_frame_data(self, frame):
        first = True
        for data in data_list:
            u = [0] * len(data)
            if first:
                u[0] = 1
                first = False

            d = AxiStreamFrame(data, tid = self.cur_id, tdest = self.cur_id, tuser = u)
            await self.source.send(d)
            self.cur_id += 1

class AXISSink:
    def __init__(self, dut, BUS_NAME, clk, rst):
        self.dut = dut
        self.clk = clk
        self.rst = rst
        self.log = logging.getLogger("cocotb.tb")
        self.log.setLevel(logging.DEBUG)
        self.recv_frames = []

        self.sink = AxiStreamSink(AxiStreamBus.from_prefix(dut, BUS_NAME), clk, rst)

    def set_backpressure_generator(self, generator=None):
        if generator:
            self.sink.set_pause_generator(generator())

    async def reset(self):
        self.recv_frames = []
        self.rst.setimmediatevalue(0)
        await RisingEdge(self.clk)
        await RisingEdge(self.clk)
        self.rst.value = 1
        await RisingEdge(self.clk)
        await RisingEdge(self.clk)
        self.rst.value = 0
        await RisingEdge(self.clk)
        await RisingEdge(self.clk)

    def insert_backpreassure_list(self, cycle_list = None):
        if type(cycle_list) is not list:
            print ("Cycle List needs to be a list")
        self.sink.set_pause_generator(itertools.cycle(cycle_list))

    async def receive(self):
        self.recv_frames.append(await self.sink.recv())

    def read_data(self):
        frames = [d for d in self.recv_frames]
        data = []
        for f in frames:
            data.append([d for d in f])
        return data

class AXISMonitor:
    def __init__(self, dut, BUS_NAME, clk, rst):
        self.dut = dut
        self.clk = clk
        self.rst = rst
        self.log = logging.getLogger("cocotb.tb")
        self.log.setLevel(logging.DEBUG)

        self.monitor = AxiStreamMonitor(AxiStreamBus.from_prefix(dut, BUS_NAME), clk, rst)

    async def reset(self):
        self.rst.setimmediatevalue(0)
        await RisingEdge(self.clk)
        await RisingEdge(self.clk)
        self.rst <= 1
        await RisingEdge(self.clk)
        await RisingEdge(self.clk)
        self.rst <= 0
        await RisingEdge(self.clk)
        await RisingEdge(self.clk)

