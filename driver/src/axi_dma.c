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

#include "axi_dma.h"
#include "misc.h"

#include <stdio.h> // todo: remove this once debugging is finished
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

/*
 * AXI DMA general function
 */
int32_t axi_dma_init(axi_dma_t *device, uint32_t baseaddr, uint32_t size) {
    // Open /dev/mem for DMA controller registers mapping
    int32_t dev_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dev_fd < 0) {
        fprintf(stderr, "Error: Failed to open /dev/mem\n");
        return -1;
    }

    // Init DMA controller device
    device->size = size;
    device->p_baseaddr = baseaddr;
    device->v_baseaddr = (uint32_t *) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, baseaddr);
    if (device->v_baseaddr == MAP_FAILED) {
        fprintf(stderr, "Error: Failed to map DMA controller registers\n");
        close(dev_fd);
        return -1;
    }
    close(dev_fd);

    // Open HARU DMA kernel module for CMA buffer access
    device->haru_dma_fd = open("/dev/haru-dma", O_RDWR);
    if (device->haru_dma_fd < 0) {
        fprintf(stderr, "Error: Failed to open /dev/haru-dma\n");
        munmap(device->v_baseaddr, device->size);
        return -1;
    }

    // Get physical address and size from kernel module
    if (ioctl(device->haru_dma_fd, HARU_GET_PHYS, &device->cma_phys) < 0) {
        fprintf(stderr, "Error: Failed to get CMA physical address\n");
        close(device->haru_dma_fd);
        munmap(device->v_baseaddr, device->size);
        return -1;
    }

    if (ioctl(device->haru_dma_fd, HARU_GET_SIZE, &device->cma_size) < 0) {
        fprintf(stderr, "Error: Failed to get CMA buffer size\n");
        close(device->haru_dma_fd);
        munmap(device->v_baseaddr, device->size);
        return -1;
    }

    // Map the CMA buffer to userspace
    device->cma_cpu = mmap(NULL, device->cma_size, PROT_READ | PROT_WRITE, MAP_SHARED, device->haru_dma_fd, 0);
    if (device->cma_cpu == MAP_FAILED) {
        fprintf(stderr, "Error: Failed to map CMA buffer\n");
        close(device->haru_dma_fd);
        munmap(device->v_baseaddr, device->size);
        return -1;
    }

    // Split CMA buffer into src/dst slices (4KiB-aligned)
    size_t off_dst = (device->cma_size / 2) & ~0xFFF;   // 4KiB-align
    device->v_src_addr = (uint8_t*)device->cma_cpu + 0;
    device->v_dst_addr = (uint8_t*)device->cma_cpu + off_dst;
    device->p_src_addr = device->cma_phys + 0;
    device->p_dst_addr = device->cma_phys + off_dst;

    fprintf(stderr, "CMA buffer: phys=0x%lx size=%lu bytes\n", 
           (unsigned long)device->cma_phys, (unsigned long)device->cma_size);
    fprintf(stderr, "SRC: virt=%p phys=0x%lx\n", device->v_src_addr, (unsigned long)device->p_src_addr);
    fprintf(stderr, "DST: virt=%p phys=0x%lx\n", device->v_dst_addr, (unsigned long)device->p_dst_addr);

    // Check and reset DMA controller
    if (dma_mm2s_sg_active(device) || dma_s2mm_sg_active(device)) {
        fprintf(stderr, "Error: DMA scatter/gather mode active\n");
        munmap(device->cma_cpu, device->cma_size);
        close(device->haru_dma_fd);
        munmap(device->v_baseaddr, device->size);
        return -1;
    }
    
    dma_mm2s_reset(device);
    dma_s2mm_reset(device);

    return 0;
}

void axi_dma_release(axi_dma_t *device) {
    if (device->cma_cpu != MAP_FAILED && device->cma_cpu != NULL) {
        munmap(device->cma_cpu, device->cma_size);
    }
    if (device->haru_dma_fd >= 0) {
        close(device->haru_dma_fd);
    }
    if (device->v_baseaddr != MAP_FAILED && device->v_baseaddr != NULL) {
        munmap(device->v_baseaddr, device->size);
    }
}

/*
 * AXI DMA transfers
 */
void axi_dma_mm2s_transfer(axi_dma_t *device, uint32_t size) {
    // Clearup
    dma_mm2s_reset(device);
    dma_mm2s_stop(device);

    // Config and start
    dma_mm2s_set_src_addr(device, device->p_src_addr);
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_CR, 0xf001);
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_LENGTH, size);
    
    // Wait for completion
    dma_mm2s_busy_wait(device);
    // printf("mm2s transferred %d bytes\n", size);
    // HARU_INFO("mm2s transfer done\n");
}

void axi_dma_s2mm_transfer(axi_dma_t *device, uint32_t size) {
    // Clearup
    dma_s2mm_reset(device);
    dma_s2mm_stop(device);

    // Config and start
    dma_s2mm_set_dst_addr(device, device->p_dst_addr);
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_CR, 0xf001);
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_LENGTH, size);
    
    dma_s2mm_busy_wait(device);
}

void axi_dma_haru_query_transfer(axi_dma_t *device, uint32_t src_len, uint32_t dst_len) {
    dma_mm2s_reset(device);
    dma_s2mm_reset(device);
    dma_mm2s_stop(device);
    dma_s2mm_stop(device);

    dma_mm2s_set_src_addr(device, device->p_src_addr);
    dma_s2mm_set_dst_addr(device, device->p_dst_addr);

    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_CR, 0xf001);
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_CR, 0xf001);
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_LENGTH, dst_len);
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_LENGTH, src_len);
    
    dma_mm2s_busy_wait(device);
    dma_s2mm_busy_wait(device);
}

// TODO: find a more efficient way to do this
void dma_mm2s_busy_wait(axi_dma_t *device) {
    volatile uint32_t mm2s_sr = _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR);
    while (!(mm2s_sr & (1 << AXI_DMA_SR_IDLE))) {
        mm2s_sr = _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR);
    }
}

// TODO: find a more efficient way to do this
void dma_s2mm_busy_wait(axi_dma_t *device) {
    volatile uint32_t s2mm_sr = _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR);
    while (!(s2mm_sr & (1 << AXI_DMA_SR_IDLE))) {
        s2mm_sr = _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR);
    }
}

/*
 * DMA status
 */
void dma_mm2s_status(axi_dma_t *device) {
    uint32_t status = _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR);
    printf("Memory-mapped to stream status (0x%08x@0x%02x):", status, AXI_DMA_MM2S_SR);
    if (status & 0x00000001)
        printf(" halted");
    else
        printf(" running");

    if (status & 0x00000002)
        printf(" idle");
    if (status & 0x00000008)
        printf(" SGIncld");
    if (status & 0x00000010)
        printf(" DMAIntErr");
    if (status & 0x00000020)
        printf(" DMASlvErr");
    if (status & 0x00000040)
        printf(" DMADecErr");
    if (status & 0x00000100)
        printf(" SGIntErr");
    if (status & 0x00000200)
        printf(" SGSlvErr");
    if (status & 0x00000400)
        printf(" SGDecErr");
    if (status & 0x00001000)
        printf(" IOC_Irq");
    if (status & 0x00002000)
        printf(" Dly_Irq");
    if (status & 0x00004000)
        printf(" Err_Irq");
    printf("\n");
}

void dma_s2mm_status(axi_dma_t *device) {
    uint32_t status = _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR);
    printf("Stream to Memory-mapped status (0x%08x@0x%02x):", status, AXI_DMA_S2MM_SR);
    if (status & (1 << AXI_DMA_SR_HALTED))
        printf(" halted");
    else
        printf(" running");
    if (status & (1 << AXI_DMA_SR_IDLE))
        printf(" idle");
    if (status & (1 << AXI_DMA_SR_SG_ACT))
        printf(" SGIncld");
    if (status & (1 << AXI_DMA_SR_DMA_INT_ERR))
        printf(" DMAIntErr");
    if (status & (1 << AXI_DMA_SR_DMA_SLV_ERR))
        printf(" DMASlvErr");
    if (status & (1 << AXI_DMA_SR_DMA_DEC_ERR))
        printf(" DMADecErr");
    if (status & (1 << AXI_DMA_SR_SG_INT_ERR))
        printf(" SGIntErr");
    if (status & (1 << AXI_DMA_SR_SG_SLV_ERR))
        printf(" SGSlvErr");
    if (status & (1 << AXI_DMA_SR_SG_DEC_ERR))
        printf(" SGDecErr");
    if (status & (1 << AXI_DMA_SR_IOC_IRQ))
        printf(" IOC_Irq");
    if (status & (1 << AXI_DMA_SR_DLY_IRQ))
        printf(" Dly_Irq");
    if (status & (1 << AXI_DMA_SR_ERR_IRQ))
        printf(" Err_Irq");
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

uint32_t dma_mm2s_sr(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR);
}

uint32_t dma_s2mm_sr(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR);
}

////////////////////////////////////////////////////////////////////////////////
// AXI DMA Setters
////////////////////////////////////////////////////////////////////////////////

void dma_mm2s_reset(axi_dma_t *device) {
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_CR, 1 << AXI_DMA_CR_RESET);
}

void dma_s2mm_reset(axi_dma_t *device) {
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_CR, 1 << AXI_DMA_CR_RESET);
}

void dma_mm2s_run(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_CR, cr | (1 << AXI_DMA_CR_RS));
}

void dma_mm2s_stop(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_CR, cr & ~(1 << AXI_DMA_CR_RS));
}

void dma_s2mm_run(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_S2MM_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_CR, cr | (1 << AXI_DMA_CR_RS));
}

void dma_s2mm_stop(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_S2MM_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_CR, cr & ~(1 << AXI_DMA_CR_RS));
}

void dma_mm2s_IOC_IRQ_EN(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_CR, cr | (1 << AXI_DMA_CR_IOC_IRQ_EN));
}

void dma_mm2s_IOC_IRQ_DIS(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_CR, cr & ~(1 << AXI_DMA_CR_IOC_IRQ_EN));
}

void dma_s2mm_IOC_IRQ_EN(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_S2MM_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_CR, cr | (1 << AXI_DMA_CR_IOC_IRQ_EN));
}

void dma_s2mm_IOC_IRQ_DIS(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_S2MM_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_CR, cr & ~(1 << AXI_DMA_CR_IOC_IRQ_EN));
}

void dma_mm2s_DLY_IRO_EN(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_CR, cr | (1 << AXI_DMA_CR_DLY_IRQ_EN));
}

void dma_mm2s_DLY_IRO_DIS(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_CR, cr & ~(1 << AXI_DMA_CR_DLY_IRQ_EN));
}

void dma_s2mm_DLY_IRO_EN(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_S2MM_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_CR, cr | (1 << AXI_DMA_CR_DLY_IRQ_EN));
}

void dma_s2mm_DLY_IRO_DIS(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_S2MM_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_CR, cr & ~(1 << AXI_DMA_CR_DLY_IRQ_EN));
}

void dma_mm2s_ERR_IRQ_EN(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_CR, cr | (1 << AXI_DMA_CR_ERR_IRQ_EN));
}

void dma_mm2s_ERR_IRQ_DIS(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_MM2S_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_CR, cr & ~(1 << AXI_DMA_CR_ERR_IRQ_EN));
}

void dma_s2mm_ERR_IRQ_EN(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_S2MM_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_CR, cr | (1 << AXI_DMA_CR_ERR_IRQ_EN));
}

void dma_s2mm_ERR_IRQ_DIS(axi_dma_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, AXI_DMA_S2MM_CR);
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_CR, cr & ~(1 << AXI_DMA_CR_ERR_IRQ_EN));
}

void dma_mm2s_set_src_addr(axi_dma_t *device, uint64_t addr) {
    // Set lower 32 bits
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_SRC_ADDR, (uint32_t)(addr & 0xFFFFFFFF));
    // Set upper 32 bits (if 64-bit addressing is enabled in IP)
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_SRC_ADDR_MSB, (uint32_t)((addr >> 32) & 0xFFFFFFFF));
}

void dma_mm2s_set_src_addr_msb(axi_dma_t *device, uint32_t addr) {
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_SRC_ADDR_MSB, addr);
}


// Sets the number of bytes (length) for the mm2s transfer
// Writing to this register will start the transfer
void dma_mm2s_set_length(axi_dma_t *device, uint32_t length) {
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_LENGTH, length * 4);
}

void dma_s2mm_set_dst_addr(axi_dma_t *device, uint64_t addr) {
    // Set lower 32 bits
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_DST_ADDR, (uint32_t)(addr & 0xFFFFFFFF));
    // Set upper 32 bits (if 64-bit addressing is enabled in IP)
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_DST_ADDR_MSB, (uint32_t)((addr >> 32) & 0xFFFFFFFF));
}

void dma_s2mm_set_dst_addr_msb(axi_dma_t *device, uint32_t addr) {
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_DST_ADDR_MSB, addr);
}

// Sets the number of bytes (length) for the s2mm transfer
// Writing to this register will start the transfer
void dma_s2mm_set_length(axi_dma_t *device, uint32_t length) {
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_LENGTH, length);
}


////////////////////////////////////////////////////////////////////////////////
// AXI DMA Getters
////////////////////////////////////////////////////////////////////////////////

uint8_t dma_mms2_halted(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_HALTED)) ? 1 : 0;
}

uint8_t dma_s2mm_halted(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_HALTED)) ? 1 : 0;
}

uint8_t dma_mms2_idle(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_IDLE)) ? 1 : 0;
}

uint8_t dma_s2mm_idle(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_IDLE)) ? 1 : 0;
}

uint8_t dma_mm2s_busy(axi_dma_t *device) {
    uint32_t sr = _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR);
    return (!(sr & (1 << AXI_DMA_SR_IOC_IRQ)) && !(sr & (1 << AXI_DMA_SR_IDLE))) ? 1 : 0;
}

uint8_t dma_s2mm_busy(axi_dma_t *device) {
    uint32_t sr = _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR);
    return (!(sr & (1 << AXI_DMA_SR_IOC_IRQ)) && !(sr & (1 << AXI_DMA_SR_IDLE))) ? 1 : 0;
}

uint8_t dma_mm2s_sg_active(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_SG_ACT)) ? 1 : 0;
}

uint8_t dma_s2mm_sg_active(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_SG_ACT)) ? 1 : 0;
}

uint8_t dma_mm2s_dma_internal_error(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_DMA_INT_ERR)) ? 1 : 0;
}

uint8_t dma_s2mm_dma_internal_error(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_DMA_INT_ERR)) ? 1 : 0;
}

uint8_t dma_mm2s_dma_slave_error(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_DMA_SLV_ERR)) ? 1 : 0;
}

uint8_t dma_s2mm_dma_slave_error(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_DMA_SLV_ERR)) ? 1 : 0;
}

uint8_t dma_mm2s_dma_decode_error(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_DMA_DEC_ERR)) ? 1 : 0;
}

uint8_t dma_s2mm_dma_decode_error(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_DMA_DEC_ERR)) ? 1 : 0;
}

uint8_t dma_mm2s_IOC_IRQ(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_IOC_IRQ)) ? 1 : 0;
}

uint8_t dma_s2mm_IOC_IRQ(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_IOC_IRQ)) ? 1 : 0;
}

uint8_t dma_mm2s_DLY_IRQ(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_DLY_IRQ)) ? 1 : 0;
}

uint8_t dma_s2mm_DLY_IRQ(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_DLY_IRQ)) ? 1 : 0;
}

uint8_t dma_mm2s_ERR_IRQ(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_ERR_IRQ)) ? 1 : 0;
}

uint8_t dma_s2mm_ERR_IRQ(axi_dma_t *device) {
    return (_reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_ERR_IRQ)) ? 1 : 0;
}