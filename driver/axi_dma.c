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

#include "axi_dma.h"

/* ===============================
 * Register access macros
 * =============================== */
#ifndef _reg_set
#define _reg_set(BaseAddress, RegOffset, Data) \
    *(volatile uint32_t*)(((uint32_t *)BaseAddress) + (RegOffset >> 2)) = (uint32_t)(Data)
#endif

#ifndef _reg_get
#define _reg_get(BaseAddress, RegOffset) \
    *(volatile uint32_t*)(((uint32_t *)BaseAddress) + (RegOffset >> 2))
#endif

/**
 * @brief AXI DMA initialization
 * 
 * @param axi_dma - AXI DMA info
 * @return int - 0 on success, -1 on failure
 */
int axi_dma_init(struct axi_dma_info *axi_dma) {
    int dev_fd;

    /* Open /dev/mem to map device */
    dev_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dev_fd < 0) {
        fprintf(stderr, "zynq_dtw_init: failed to open /dev/mem (%s)\n", strerror(errno));
        return -1;
    }

    /* Init device */
    if (axi_dma->device_size == 0) {
        fprintf(stderr, "axi_dma_init: invalid device size\n");
        close(dev_fd);
        return -1;
    }

    if (axi_dma->phys_baseaddr == 0) {
        fprintf(stderr, "axi_dma_init: invalid base address\n");
        close(dev_fd);
        return -1;
    }

    if (axi_dma->phys_src_buf_addr == 0) {
        fprintf(stderr, "axi_dma_init: invalid source address\n");
        close(dev_fd);
        return -1;
    }
    
    /* Map device */
    axi_dma->virt_baseaddr = mmap(NULL, axi_dma->device_size,
            PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, axi_dma->phys_baseaddr);
    if (axi_dma->virt_baseaddr == MAP_FAILED) {
        close(dev_fd);
        return -1;
    }

    /* Currently HARU only supports direct register mode for AXI DMA */
    if (dma_mm2s_sg_active(axi_dma)) {
        munmap(axi_dma->virt_baseaddr, axi_dma->device_size);
        close(dev_fd);
        return -1;
    }

    dma_mm2s_reset(axi_dma);

    /* Initialize buffer */
    axi_dma->virt_src_buf_addr = (void *) mmap(NULL, axi_dma->src_buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, axi_dma->phys_src_buf_addr);
    if (axi_dma->virt_src_buf_addr == MAP_FAILED) {
        munmap(axi_dma->virt_baseaddr, axi_dma->device_size);
        close(dev_fd);
        return -1;
    }

    close(dev_fd);
    return 0;
}


void axi_dma_release(struct axi_dma_info *axi_dma) {
    munmap(axi_dma->virt_baseaddr, axi_dma->device_size);       // Unmap device
    munmap(axi_dma->virt_src_buf_addr, axi_dma->device_size);   // Unmap buffer
}

// TODO: add checks for timeout so that we don't hang
void axi_dma_mm2s_transfer(struct axi_dma_info *axi_dma, uint32_t size) {
    /* Clearup */
    dma_mm2s_reset(axi_dma);
    dma_mm2s_stop(axi_dma);

    /* Config and start */
    dma_mm2s_set_src_addr(axi_dma, axi_dma->phys_src_buf_addr);
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR, 0xf001);   // Enable interrupts
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_LENGTH, size); // This triggers the transfer
    
    /* Poll for completion */
    dma_mm2s_busy_wait(axi_dma);
}

// void axi_dma_haru_query_transfer(struct axi_dma_info *axi_dma, uint32_t src_len, uint32_t dst_len) {
//     dma_mm2s_reset(axi_dma);
//     dma_mm2s_stop(axi_dma);

//     dma_mm2s_set_src_addr(axi_dma, axi_dma->phys_src_buf_addr);

//     _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR, 0xf001);
//     _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_LENGTH, src_len);
    
//     dma_mm2s_busy_wait(axi_dma);
// }

/**
 * @brief Busy wait for DMA transfer to complete
 * 
 * @param axi_dma 
 */
void dma_mm2s_busy_wait(struct axi_dma_info *axi_dma) {
    volatile uint32_t mm2s_sr = _reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR);
    while (!(mm2s_sr & (1 << AXI_DMA_SR_IDLE))) {
        mm2s_sr = _reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR);
    }
}

/**
 * @brief Print DMA status
 * 
 * @param axi_dma 
 */
void dma_mm2s_status(struct axi_dma_info *axi_dma) {
    uint32_t status = _reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR);
    printf("Memory-mapped to stream status (0x%08x@0x%02x):", status, AXI_DMA_MM2S_SR);
    if (status & 0x00000001) {
        printf(" halted");
    } else {
        printf(" running");
    }

    if (status & 0x00000002) printf(" idle");
    if (status & 0x00000008) printf(" SGIncld");
    if (status & 0x00000010) printf(" DMAIntErr");
    if (status & 0x00000020) printf(" DMASlvErr");
    if (status & 0x00000040) printf(" DMADecErr");
    if (status & 0x00000100) printf(" SGIntErr");
    if (status & 0x00000200) printf(" SGSlvErr");
    if (status & 0x00000400) printf(" SGDecErr");
    if (status & 0x00001000) printf(" IOC_Irq");
    if (status & 0x00002000) printf(" Dly_Irq");
    if (status & 0x00004000) printf(" Err_Irq");
    printf("\n");
}

void axi_dma_read_data(void *address, int byte_length) {
    int *addr = (int *) address;
    int reg_offset;
    printf("\t[read_data] data at destination address 0x%p:", address); 
    for (reg_offset = 0; reg_offset < byte_length/4; reg_offset+=4) {
        if (reg_offset % 10 == 0) {
            printf("\n\t");
        }
        printf("%x ", *addr);
        addr++;
    }
    printf("\n");
}

uint32_t dma_mm2s_sr(struct axi_dma_info *axi_dma) {
    return _reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR);
}


/* ===============================
 * AXI DMA MM2S Setters
 * =============================== */
void dma_mm2s_reset(struct axi_dma_info *axi_dma) {
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR, 1 << AXI_DMA_CR_RESET);
}

void dma_mm2s_run(struct axi_dma_info *axi_dma) {
    uint32_t cr = _reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR, cr | (1 << AXI_DMA_CR_RS));
}

void dma_mm2s_stop(struct axi_dma_info *axi_dma) {
    uint32_t cr = _reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR, cr & ~(1 << AXI_DMA_CR_RS));
}

void dma_mm2s_IOC_IRQ_EN(struct axi_dma_info *axi_dma) {
    uint32_t cr = _reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR, cr | (1 << AXI_DMA_CR_IOC_IRQ_EN));
}

void dma_mm2s_IOC_IRQ_DIS(struct axi_dma_info *axi_dma) {
    uint32_t cr = _reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR, cr & ~(1 << AXI_DMA_CR_IOC_IRQ_EN));
}

void dma_mm2s_DLY_IRO_EN(struct axi_dma_info *axi_dma) {
    uint32_t cr = _reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR, cr | (1 << AXI_DMA_CR_DLY_IRQ_EN));
}

void dma_mm2s_DLY_IRO_DIS(struct axi_dma_info *axi_dma) {
    uint32_t cr = _reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR, cr & ~(1 << AXI_DMA_CR_DLY_IRQ_EN));
}

void dma_mm2s_ERR_IRQ_EN(struct axi_dma_info *axi_dma) {
    uint32_t cr = _reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR, cr | (1 << AXI_DMA_CR_ERR_IRQ_EN));
}

void dma_mm2s_ERR_IRQ_DIS(struct axi_dma_info *axi_dma) {
    uint32_t cr = _reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_CR, cr & ~(1 << AXI_DMA_CR_ERR_IRQ_EN));
}

void dma_mm2s_set_src_addr(struct axi_dma_info *axi_dma, uint32_t addr) {
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SRC_ADDR, addr);
}

void dma_mm2s_set_src_addr_msb(struct axi_dma_info *axi_dma, uint32_t addr) {
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SRC_ADDR_MSB, addr);
}

void dma_mm2s_set_length(struct axi_dma_info *axi_dma, uint32_t length) {
    _reg_set(axi_dma->virt_baseaddr, AXI_DMA_MM2S_LENGTH, length * sizeof(uint32_t));
}

/* ===============================
 * AXI DMA MM2S Getter
 * =============================== */
uint8_t dma_mms2_halted(struct axi_dma_info *axi_dma) {
    return (_reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_HALTED)) ? 1 : 0;
}

uint8_t dma_mms2_idle(struct axi_dma_info *axi_dma) {
    return (_reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_IDLE)) ? 1 : 0;
}

uint8_t dma_mm2s_busy(struct axi_dma_info *axi_dma) {
    uint32_t sr = _reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR);
    return (!(sr & (1 << AXI_DMA_SR_IOC_IRQ)) && !(sr & (1 << AXI_DMA_SR_IDLE))) ? 1 : 0;
}

uint8_t dma_mm2s_sg_active(struct axi_dma_info *axi_dma) {
    return (_reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_SG_ACT)) ? 1 : 0;
}

uint8_t dma_mm2s_dma_internal_error(struct axi_dma_info *axi_dma) {
    return (_reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_DMA_INT_ERR)) ? 1 : 0;
}

uint8_t dma_mm2s_dma_slave_error(struct axi_dma_info *axi_dma) {
    return (_reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_DMA_SLV_ERR)) ? 1 : 0;
}

uint8_t dma_mm2s_dma_decode_error(struct axi_dma_info *axi_dma) {
    return (_reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_DMA_DEC_ERR)) ? 1 : 0;
}

uint8_t dma_mm2s_IOC_IRQ(struct axi_dma_info *axi_dma) {
    return (_reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_IOC_IRQ)) ? 1 : 0;
}
uint8_t dma_mm2s_DLY_IRQ(struct axi_dma_info *axi_dma) {
    return (_reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_DLY_IRQ)) ? 1 : 0;
}

uint8_t dma_mm2s_ERR_IRQ(struct axi_dma_info *axi_dma) {
    return (_reg_get(axi_dma->virt_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_ERR_IRQ)) ? 1 : 0;
}