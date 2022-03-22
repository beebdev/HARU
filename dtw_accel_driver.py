

__author__ = "<your@email.here>"

import sys
import os
import time
from axi_driver import Driver

from array import array as Array

import cocotb
from cocotb.result import ReturnValue
from cocotb_bus.drivers.amba import AXI4LiteMaster
from cocotb.triggers import Timer

REG_CONTROL             = 0  << 2
REG_VERSION             = 1  << 2



#Set/Clear a bit
BIT_CTRL_TEST           = 0

#Set/Get a range of bits
BIT_CTRL_TR_HIGH        = 15
BIT_CTRL_TR_LOW         = 8

class DemoAXIStreamsDriver (Driver):
    def __init__(self, dut, name, clock, reset, debug = False):
        super(DemoAXIStreamsDriver, self).__init__(dut, name, clock, reset, debug=debug)

    def __del__(self):
        pass

    async def get_version(self):
        data = await self.read_register(REG_VERSION)
        return data

    # Set an entire Register
    async def set_control(self, data):
        await self.write_register(REG_CONTROL, data)

    # Get Entire Register
    async def get_control(self):
        data = await self.read_register(REG_CONTROL)
        return data

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

