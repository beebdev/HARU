

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

# Addres map
REG_CONTROL = 0 << 2;
REG_STATUS  = 1 << 2;
REG_REF_LEN = 2 << 2;
REG_VERSION = 3 << 2;
REG_KEY     = 4 << 2;

# CR bits
CR_RESET    = 0;
CR_RS       = 1;
CR_OPMODE   = 2;

#Set/Clear a bit
BIT_CTRL_TEST = 0

#Set/Get a range of bits
BIT_CTRL_TR_HIGH    = 15
BIT_CTRL_TR_LOW     = 8

class DtwAccelDriver (Driver):
    def __init__(self, dut, name, clock, reset, debug = False):
        super(DtwAccelDriver, self).__init__(dut, name, clock, reset, debug=debug)

    def __del__(self):
        pass

    ## Control Register
    async def set_control(self, data):
        """
        Set the control register
        """
        await self.write_register(REG_CONTROL, data)

    # Get CR
    async def get_control(self):
        """
        Get the control register
        """
        data = await self.read_register(REG_CONTROL)
        return data
    
    # reset
    async def core_reset(self):
        await self.enable_register_bit(REG_CONTROL, CR_RESET, 1)
        await self.enable_register_bit(REG_CONTROL, CR_RESET, 0)

    # Set RS
    async def set_rs(self, enable):
        await self.enable_register_bit(REG_CONTROL, CR_RS, enable)

    # Set opmode
    async def set_opmode(self, opmode):
        await self.enable_register_bit(REG_CONTROL, CR_OPMODE, opmode)

    ## Status Register
    async def get_status(self):
        """
        Get the status register
        """
        data = await self.read_register(REG_STATUS)
        return data

    ## REF_LEN
    async def set_ref_len(self, data):
        """
        Set the ref_len register
        """
        await self.write_register(REG_REF_LEN, data)

    async def get_ref_len(self):
        """
        Get the ref_len register
        """
        data = await self.read_register(REG_REF_LEN)
        return data

    ## Version
    async def get_version(self):
        """
        Get the version register
        """
        data = await self.read_register(REG_VERSION)
        return data

    ## Key
    async def get_key(self):
        """
        Get the key register
        """
        data = await self.read_register(REG_KEY)
        return data

    ## others

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

