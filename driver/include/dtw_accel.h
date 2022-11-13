/* MIT License

Copyright (c) 2022 Po Jui Shih
Copyright (c) 2022 Hassaan Saadat
Copyright (c) 2022 Sri Parameswaran
Copyright (c) 2022 Hasindu Gamaarachchi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#ifndef DTW_ACCEL_H
#define DTW_ACCEL_H

#include <stdint.h>

// DTW Accel register addresses
#define DTW_ACCEL_CR_ADDR                   0 << 2
#define DTW_ACCEL_SR_ADDR                   1 << 2
#define DTW_ACCEL_REF_LEN_ADDR              2 << 2
#define DTW_ACCEL_VERSION_ADDR              3 << 2
#define DTW_ACCEL_KEY_ADDR                  4 << 2
#define DTW_ACCEL_DBG_REF_ADDR              5 << 2
#define DTW_ACCEL_DBG_REF_DIN_ADDR          6 << 2
#define DTW_ACCEL_DBG_REF_DOUT_ADDR         7 << 2
#define DTW_ACCEL_DBG_CYCLE_CNT             8 << 2
#define DTW_ACCEL_DBG_CORE_ADDR             9 << 2
#define DTW_ACCEL_DBG_NQUERY                10 << 2
#define DTW_ACCEL_DBG_CURR_QID              11 << 2

// Control register bit offsets
#define DTW_ACCEL_CR_OFFSET_RESET           0x00
#define DTW_ACCEL_CR_OFFSET_RS              0x01
#define DTW_ACCEL_CR_OFFSET_MODE            0x02

// Status register bit offsets
#define DTW_ACCEL_SR_OFFSET_BUSY            0x00
#define DTW_ACCEL_SR_OFFSET_REF_LOAD_DONE   0x01
#define DTW_ACCEL_SR_OFFSET_SRC_FIFO_EMPTY  0x02
#define DTW_ACCEL_SR_OFFSET_SRC_FIFO_FULL   0x03
#define DTW_ACCEL_SR_OFFSET_SINK_FIFO_EMPTY 0x04
#define DTW_ACCEL_SR_OFFSET_SINK_FIFO_FULL  0x05
#define DTW_ACCEL_SR_OFFSET_STATE_LSB       0x06
#define DTW_ACCEL_SR_OFFSET_STATE_MSB       0x08

// Mode bit values
#define DTW_ACCEL_MODE_QUERY                0x00
#define DTW_ACCEL_MODE_REF_LOAD             0x01

// Key value
#define DTW_ACCEL_KEY                       0x0ca7cafe

// DEBUG
#define DTW_ACCEL_DBG_REF_ADDR_WREN         31
#define DTW_ACCEL_DBG_REF_ADDR_W_MSB        29
#define DTW_ACCEL_DBG_REF_ADDR_W_LSB        15
#define DTW_ACCEL_DBG_REF_ADDR_R_MSB        14
#define DTW_ACCEL_DBG_REF_ADDR_R_LSB        0
#define DTW_ACCEL_DBG_REF_DIN_MSB           15
#define DTW_ACCEL_DBG_REF_DIN_LSB           0
#define DTW_ACCEL_DBG_REF_DIN_MSB           15
#define DTW_ACCEL_DBG_REF_DIN_LSB           0
#define DTW_ACCEL_CORE_WADDR_LSB            0x00
#define DTW_ACCEL_CORE_RADDR_LSB            0x15

typedef struct {
    uint32_t *v_baseaddr;    // Memory mapped virtual base address
    uint32_t p_baseaddr;    // Physical base address
    uint32_t size;          // Size of device
} dtw_accel_t;

int32_t dtw_accel_init(dtw_accel_t *device, uint32_t baseaddr, uint32_t size);
void dtw_accel_release(dtw_accel_t *device);

void dtw_accel_reset(dtw_accel_t *device);
void dtw_accel_run(dtw_accel_t *device);
void dtw_accel_stop(dtw_accel_t *device);
void dtw_accel_set_mode(dtw_accel_t *device, uint8_t mode);
void dtw_accel_set_ref_len(dtw_accel_t *device, uint32_t len);

uint32_t dtw_accel_get_cr(dtw_accel_t *device);
uint32_t dtw_accel_get_sr(dtw_accel_t *device);
uint32_t dtw_accel_get_ref_len(dtw_accel_t *device);
uint32_t dtw_accel_get_version(dtw_accel_t *device);
uint32_t dtw_accel_get_key(dtw_accel_t *device);

uint32_t dtw_accel_busy(dtw_accel_t *device);
uint32_t dtw_accel_ref_load_done(dtw_accel_t *device);
uint32_t dtw_accel_src_fifo_empty(dtw_accel_t *device);
uint32_t dtw_accel_src_fifo_full(dtw_accel_t *device);
uint32_t dtw_accel_sink_fifo_empty(dtw_accel_t *device);
uint32_t dtw_accel_sink_fifo_full(dtw_accel_t *device);
uint32_t dtw_accel_state(dtw_accel_t *device);
uint32_t dtw_accel_addrw_ref(dtw_accel_t *device);
uint32_t dtw_accel_addrr_ref(dtw_accel_t *device);

void dtw_accel_dbg_wren(dtw_accel_t *device, uint32_t wren);
void dtw_accel_dbg_addrW_ref(dtw_accel_t *device, uint32_t addrW_ref);
void dtw_accel_dbg_addrR_ref(dtw_accel_t *device, uint32_t addrW_ref);
void dtw_accel_dbg_din(dtw_accel_t *device, uint32_t din);
uint32_t dtw_accel_dbg_dout(dtw_accel_t *device);
uint32_t dtw_accel_dbg_cycle_cnt(dtw_accel_t *device);
uint32_t dtw_accel_dbg_nquery(dtw_accel_t *device);
uint32_t dtw_accel_dbg_curr_qid(dtw_accel_t *device);

#endif