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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#include "zynq_dtw.h"

/* ===============================
 * Register access macros
 * =============================== */
#ifndef _reg_set
#define _reg_set(BaseAddress, RegOffset, Data) \
    *(volatile uint32_t*)(((uint32_t *) BaseAddress) + (RegOffset >> 2)) = (uint32_t)(Data)
#endif

#ifndef _reg_get
#define _reg_get(BaseAddress, RegOffset) \
    *(volatile uint32_t*)(((uint32_t *) BaseAddress) + (RegOffset >> 2))
#endif


int zynq_dtw_init(struct zynq_dtw_info *zynq_dtw) {
    int dev_fd;

    /* Open /dev/mem to map device */
    dev_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dev_fd < 0) {
        fprintf(stderr, "%s: failed to open /dev/mem (%s)\n", __func__, strerror(errno));
        return -1;
    }

    /* Init device */
    if (zynq_dtw->phys_baseaddr == 0) {
        fprintf(stderr, "%s: invalid source address\n", __func__);
        close(dev_fd);
        return -1;
    }

    if (zynq_dtw->device_size == 0) {
        fprintf(stderr, "%s: invalid device size\n", __func__);
        close(dev_fd);
        return -1;
    }

    /* Map device */
    zynq_dtw->virt_baseaddr = mmap(NULL, zynq_dtw->device_size,
            PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, zynq_dtw->phys_baseaddr);
    if (zynq_dtw->virt_baseaddr == MAP_FAILED) {
        fprintf(stderr, "%s: failed to map device (%s)\n", __func__, strerror(errno));
        close(dev_fd);
        return -1;
    }

    close(dev_fd);
    return 0;
}

void zynq_dtw_release(struct zynq_dtw_info *zynq_dtw) {
    munmap(zynq_dtw->virt_baseaddr, zynq_dtw->device_size);
}


void zynq_dtw_reset(struct zynq_dtw_info *zynq_dtw) {
    _reg_set(zynq_dtw->virt_baseaddr, ZYNQ_DTW_CR, ZYNQ_DTW_CR_RST);
    _reg_set(zynq_dtw->virt_baseaddr, ZYNQ_DTW_CR, 0);
}

void zynq_dtw_run(struct zynq_dtw_info *zynq_dtw) {
    _reg_set(zynq_dtw->virt_baseaddr, ZYNQ_DTW_CR, ZYNQ_DTW_CR_RS);
}

uint32_t zynq_dtw_is_running(struct zynq_dtw_info *zynq_dtw) {
    return _reg_get(zynq_dtw->virt_baseaddr, ZYNQ_DTW_SR) & ZYNQ_DTW_SR_RUNNING;
}

uint32_t zynq_dtw_is_done(struct zynq_dtw_info *zynq_dtw) {
    return _reg_get(zynq_dtw->virt_baseaddr, ZYNQ_DTW_SR) & ZYNQ_DTW_SR_DONE;
}

void zynq_dtw_set_control(struct zynq_dtw_info *zynq_dtw, uint32_t value) {
    _reg_set(zynq_dtw->virt_baseaddr, ZYNQ_DTW_CR, value);
}

uint32_t zynq_dtw_get_control(struct zynq_dtw_info *zynq_dtw) {
    return _reg_get(zynq_dtw->virt_baseaddr, ZYNQ_DTW_CR);
}

uint32_t zynq_dtw_get_status(struct zynq_dtw_info *zynq_dtw) {
    return _reg_get(zynq_dtw->virt_baseaddr, ZYNQ_DTW_SR);
}

uint32_t zynq_dtw_get_version(struct zynq_dtw_info *zynq_dtw) {
    return _reg_get(zynq_dtw->virt_baseaddr, ZYNQ_DTW_VERSION);
}

uint32_t zynq_dtw_get_key(struct zynq_dtw_info *zynq_dtw) {
    return _reg_get(zynq_dtw->virt_baseaddr, ZYNQ_DTW_KEY);
}

void zynq_dtw_set_ref_len(struct zynq_dtw_info *zynq_dtw, uint32_t ref_len) {
    _reg_set(zynq_dtw->virt_baseaddr, ZYNQ_DTW_REF_LEN, ref_len);
}

uint32_t zynq_dtw_get_ref_len(struct zynq_dtw_info *zynq_dtw) {
    return _reg_get(zynq_dtw->virt_baseaddr, ZYNQ_DTW_REF_LEN);
}

uint32_t zynq_dtw_get_best_score(struct zynq_dtw_info *zynq_dtw) {
    return _reg_get(zynq_dtw->virt_baseaddr, ZYNQ_DTW_BEST_SCORE);
}

uint32_t zynq_dtw_get_best_position(struct zynq_dtw_info *zynq_dtw) {
    return _reg_get(zynq_dtw->virt_baseaddr, ZYNQ_DTW_BEST_POSITION);
}
