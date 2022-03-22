__author__ = "<beebdev@gmail.com>"

import sys
import os
import time
from axilite_driver import Driver

from array import array as Array

import cocotb
from cocotb.result import ReturnValue
from cocotb_bus.drivers.amba import AXI4LiteMaster
from cocotb.triggers import Timer

REG_CONTROL             = 0 << 2
REG_STATUS              = 1 << 2
REG_REF_LENGTH          = 2 << 2
REG_CAT                 = 3 << 2
REG_CAFE                = 4 << 2


#Set/Clear a bit
BIT_CR_RESET            = 0
BIT_CR_RS               = 1
BIT_CR_MODE             = 2

BIT_SR_BUSY             = 0
BIT_SR_LOAD_DONE        = 1

class DtwAccelDriver(Driver):
    def __init__(self, dut, name, clock, reset, debug = False):
        super(DtwAccelDriver, self).__init__(dut, name, clock, reset, debug=debug)

    def __del__(self):
        pass

    # async def get_version(self): # TODO: Implement this
    #     data = await self.read_register(REG_VERSION)
    #     return data

    # Set an entire Register
    async def set_control(self, data):
        await self.write_register(REG_CONTROL, data)

    # Get Entire Register
    async def get_control(self):
        data = await self.read_register(REG_CONTROL)
        return data

    async def get_status(self):
        data = await self.read_register(REG_STATUS)
        return data

    # CR
    async def reset(self, enable):
        await self.enable_register_bit(REG_CONTROL, BIT_CR_RESET, enable)

    async def run(self):
        await self.enable_register_bit(REG_CONTROL, BIT_CR_RS, True)

    async def stop(self):
        await self.enable_register_bit(REG_CONTROL, BIT_CR_RS, False)

    async def set_mode(self, mode):
        await self.write_register_bit(REG_CONTROL, BIT_CR_MODE, mode)

    async def get_mode(self):
        data = await self.read_register_bit(REG_CONTROL, BIT_CR_MODE)
        return data

    # SR
    async def is_busy(self):
        data = await self.read_register_bit(REG_STATUS, BIT_SR_BUSY)
        return data
    
    async def is_load_done(self):
        data = await self.read_register_bit(REG_STATUS, BIT_SR_LOAD_DONE)
        return data

    # ref len
    async def set_ref_len(self, length):
        await self.write_register(REG_REF_LENGTH, length)
    
    async def get_ref_len(self):
        data = await self.read_register(REG_REF_LENGTH)
        return data

    # cat cafe
    async def get_cat(self):
        data = await self.read_register(REG_CAT)
        return data

    async def get_cafe(self):
        data = await self.read_register(REG_CAFE)
        return data
