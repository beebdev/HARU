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

#ifndef ZYNQ_DTW_H
#define ZYNQ_DTW_H

#include <stdint.h>

/* Zynq DTW IP core register map offsets */
#define ZYNQ_DTW_CR             0x00
#define ZYNQ_DTW_SR             0x04
#define ZYNQ_DTW_VERSION        0x08
#define ZYNQ_DTW_KEY            0x0C
#define ZYNQ_DTW_REF_LEN        0x10
#define ZYNQ_DTW_BEST_SCORE     0x14
#define ZYNQ_DTW_BEST_POSITION  0x18

/* Control register bit offsets */
#define ZYNQ_DTW_CR_RST         0x00 // Reset bit; Driver should assert and deassert
#define ZYNQ_DTW_CR_RS          0x01 // Run/Stop bit

/* Status register bit offsets */
#define ZYNQ_DTW_SR_RUNNING        0x00 // Running bit
#define ZYNQ_DTW_SR_DONE           0x01 // Done bit

/* Zynq DTW IP core info type */
struct zynq_dtw_info {
    void *virt_baseaddr;
    uint32_t phys_baseaddr;
    uint32_t device_size;
};

int zynq_dtw_init(struct zynq_dtw_info *zynq_dtw);
void zynq_dtw_release(struct zynq_dtw_info *zynq_dtw);

void zynq_dtw_reset(struct zynq_dtw_info *zynq_dtw);
void zynq_dtw_run(struct zynq_dtw_info *zynq_dtw);
uint32_t zynq_dtw_is_running(struct zynq_dtw_info *zynq_dtw);
uint32_t zynq_dtw_is_done(struct zynq_dtw_info *zynq_dtw);

void zynq_dtw_set_control(struct zynq_dtw_info *zynq_dtw, uint32_t value);
uint32_t zynq_dtw_get_control(struct zynq_dtw_info *zynq_dtw);
uint32_t zynq_dtw_get_status(struct zynq_dtw_info *zynq_dtw);
uint32_t zynq_dtw_get_version(struct zynq_dtw_info *zynq_dtw);
uint32_t zynq_dtw_get_key(struct zynq_dtw_info *zynq_dtw);
void zynq_dtw_set_ref_len(struct zynq_dtw_info *zynq_dtw, uint32_t ref_len);
uint32_t zynq_dtw_get_ref_len(struct zynq_dtw_info *zynq_dtw);
uint32_t zynq_dtw_get_best_score(struct zynq_dtw_info *zynq_dtw);
uint32_t zynq_dtw_get_best_position(struct zynq_dtw_info *zynq_dtw);

#endif // ZYNQ_DTW_H