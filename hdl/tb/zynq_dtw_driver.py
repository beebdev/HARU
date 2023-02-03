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

__author__ = "<beebdev@gmail.com>"

import sys
import os
import time
from tb.axi_driver import Driver

from array import array as Array

import cocotb
from cocotb.result import ReturnValue
from cocotb_bus.drivers.amba import AXI4LiteMaster
from cocotb.triggers import Timer

# Addres map
REG_0_CONTROL = 0 << 2;
REG_1_STATUS  = 1 << 2;
REG_2_VERSION = 2 << 2;
REG_3_KEY     = 3 << 2;
REG_4_REF_LEN = 4 << 2;
REG_5_BEST_SCORE = 5 << 2;
REG_6_BEST_OFFSET = 6 << 2;

# CR bits
REG_0_CONTROL_RST_BIT = 0;
REG_0_CONTROL_RS_BIT = 1;
REG_1_STATUS_RUNNING_BIT = 0;
REG_1_STATUS_DONE_BIT = 1;

#Set/Clear a bit
BIT_CTRL_TEST = 0

#Set/Get a range of bits
BIT_CTRL_TR_HIGH    = 15
BIT_CTRL_TR_LOW     = 8

class ZynqDTWDriver (Driver):
    def __init__(self, dut, name, clock, reset, debug = False):
        super(ZynqDTWDriver, self).__init__(dut, name, clock, reset, debug=debug)

    def __del__(self):
        pass

    # ========================
    # REG_0_CONTROL
    # ========================
    async def set_control(self, data):
        await self.write_register(REG_0_CONTROL, data)

    async def get_control(self):
        data = await self.read_register(REG_0_CONTROL)
        return data
    
    async def core_reset(self):
        await self.enable_register_bit(REG_0_CONTROL, REG_0_CONTROL_RST_BIT, 1)
        await self.enable_register_bit(REG_0_CONTROL, REG_0_CONTROL_RST_BIT, 0)

    async def set_rs(self, enable):
        await self.enable_register_bit(REG_0_CONTROL, REG_0_CONTROL_RS_BIT, enable)

    # ========================
    # REG_1_STATUS
    # ========================
    async def get_status(self):
        data = await self.read_register(REG_1_STATUS)
        return data

    async def is_running(self):
        data = await self.read_register_bit(REG_1_STATUS, REG_1_STATUS_RUNNING_BIT)
        return data

    async def is_done(self):
        data = await self.read_register_bit(REG_1_STATUS, REG_1_STATUS_DONE_BIT)
        return data

    # ========================
    # REG_2_VERSION
    # ========================
    async def get_version(self):
        data = await self.read_register(REG_2_VERSION)
        return data

    # ========================
    # REG_3_KEY
    # ========================
    async def get_key(self):
        data = await self.read_register(REG_3_KEY)
        return data

    # ========================
    # REG_4_REF_LEN
    # ========================
    async def set_ref_len(self, data):
        await self.write_register(REG_4_REF_LEN, data)

    async def get_ref_len(self):
        data = await self.read_register(REG_4_REF_LEN)
        return data

    # ========================
    # REG_5_BEST_SCORE
    # ========================
    async def get_best_score(self):
        data = await self.read_register(REG_5_BEST_SCORE)
        return data

    # ========================
    # REG_6_BEST_OFFSET
    # ========================
    async def get_best_offset(self):
        data = await self.read_register(REG_6_BEST_OFFSET)
        return data
    

    # ========================
    # others
    # ========================

    # Set a bit within a register
    async def enable_test_mode(self, enable):
        await self.enable_register_bit(REG_CONTROL, BIT_CTRL_TEST, enable)

    # Get a bit within a register
    async def is_test_mode(self):
        bit_val = await self.is_register_bit_set(REG_CONTROL, BIT_CTRL_TEST)
        return bit_val

    # Set a range of data withing a register
    async def set_control_test_range(self, data):
        await self.write_register_bit_range(REG_CONTROL, BIT_CTRL_TR_HIGH, BIT_CTRL_TR_LOW, data)

    # Get a range of data within a register
    async def get_control_test_range(self, data):
        data = await self.read_register_bit_range(REG_CONTROL, BIT_CTRL_TR_HIGH, BIT_CTRL_TR_LOW, data)
        return data

