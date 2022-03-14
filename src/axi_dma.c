#include "axi_dma.h"

#include <stdio.h> // todo: remove this once debugging is finished
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

// Set register
inline void _reg_set(uint32_t *baseaddr, int32_t offset, uint32_t data) {
    // *(volatile uint32_t *)(baseaddr + offset) = data;
    baseaddr[offset>>2] = data;
}

// Get register
inline uint32_t _reg_get(uint32_t *baseaddr, int32_t offset) {
    // return *(uint32_t *)(baseaddr + offset);
    return baseaddr[offset>>2];
}
uint32_t axi_dma_init(axi_dma_t *device, uint32_t baseaddr, uint32_t size) {
    int32_t dev_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dev_fd < 0) {
        return -1;
    }

    device->size = size;
    device->p_baseaddr = baseaddr;
    device->v_baseaddr = (uint32_t *) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, baseaddr);
    if (device->v_baseaddr == MAP_FAILED) {
        close(dev_fd);
        return -1;
    }

    dma_mm2s_reset(device);
    dma_s2mm_reset(device);
    close(dev_fd);
    return 0;
}

void axi_dma_release(axi_dma_t *device) {
    munmap(device->v_baseaddr, device->size);
}

// Setters
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

// Getters
uint32_t dma_mms2_halted(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_HALTED);
}

uint32_t dma_s2mm_halted(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_HALTED);
}

uint32_t dma_mms2_idle(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_IDLE);
}

uint32_t dma_s2mm_idle(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_IDLE);
}

uint32_t dma_mm2s_busy(axi_dma_t *device) {
    uint32_t sr = _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR);
    return !(sr & (1 << AXI_DMA_SR_IOC_IRQ)) && !(sr & (1 << AXI_DMA_SR_IDLE));
}

uint32_t dma_s2mm_busy(axi_dma_t *device) {
    uint32_t sr = _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR);
    return !(sr & (1 << AXI_DMA_SR_IOC_IRQ)) && !(sr & (1 << AXI_DMA_SR_IDLE));
}

uint32_t dma_mm2s_sg_active(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_SG_ACT);
}

uint32_t dma_s2mm_sg_active(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_SG_ACT);
}

uint32_t dma_mm2s_dma_internal_error(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_DMA_INT_ERR);
}

uint32_t dma_s2mm_dma_internal_error(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_DMA_INT_ERR);
}

uint32_t dma_mm2s_dma_slave_error(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_DMA_SLV_ERR);
}

uint32_t dma_s2mm_dma_slave_error(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_DMA_SLV_ERR);
}

uint32_t dma_mm2s_dma_decode_error(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_DMA_DEC_ERR);
}

uint32_t dma_s2mm_dma_decode_error(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_DMA_DEC_ERR);
}

uint32_t dma_mm2s_IOC_IRQ(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_IOC_IRQ);
}

uint32_t dma_s2mm_IOC_IRQ(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_IOC_IRQ);
}

uint32_t dma_mm2s_DLY_IRQ(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_DLY_IRQ);
}

uint32_t dma_s2mm_DLY_IRQ(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_DLY_IRQ);
}

uint32_t dma_mm2s_ERR_IRQ(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_MM2S_SR) & (1 << AXI_DMA_SR_ERR_IRQ);
}

uint32_t dma_s2mm_ERR_IRQ(axi_dma_t *device) {
    return _reg_get(device->v_baseaddr, AXI_DMA_S2MM_SR) & (1 << AXI_DMA_SR_ERR_IRQ);
}

void dma_mm2s_set_src_addr(axi_dma_t *device, uint32_t addr) {
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_SRC_ADDR, addr);
}

void dma_mm2s_set_src_addr_msb(axi_dma_t *device, uint32_t addr) {
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_SRC_ADDR_MSB, addr);
}

void dma_mm2s_set_length(axi_dma_t *device, uint32_t length) {
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_LENGTH, length);
}

void dma_s2mm_set_dst_addr(axi_dma_t *device, uint32_t addr) {
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_DST_ADDR, addr);
}

void dma_s2mm_set_dst_addr_msb(axi_dma_t *device, uint32_t addr) {
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_DST_ADDR_MSB, addr);
}

void dma_s2mm_set_length(axi_dma_t *device, uint32_t length) {
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_LENGTH, length);
}

void axi_dma_mm2s_transfer(axi_dma_t *device, void *src_addr, uint32_t size) {
    uint32_t src_addr_lsb = (uint32_t)((uint64_t) src_addr);
    uint32_t src_addr_msb = (uint32_t)((uint64_t) src_addr >> 32);

    dma_mm2s_reset(device);
    dma_mm2s_stop(device);
    dma_mm2s_set_src_addr(device, src_addr_lsb); // set lsb byte of src addr
    dma_mm2s_set_src_addr_msb(device, src_addr_msb); // set msb byte of src addr
    dma_mm2s_IOC_IRQ_EN(device);
    dma_mm2s_DLY_IRO_EN(device);
    dma_mm2s_ERR_IRQ_EN(device);
    dma_mm2s_run(device);
    dma_mm2s_set_length(device, size);

    // TODO: change this to a more efficient polling method
    while (!dma_mm2s_busy(device));
}

void axi_dma_s2mm_transfer(axi_dma_t *device, void *dst_addr, uint32_t size) {
    uint32_t dst_addr_lsb = (uint32_t)((uint64_t) dst_addr);
    uint32_t dst_addr_msb = (uint32_t)((uint64_t) dst_addr >> 32);

    dma_s2mm_reset(device);
    dma_s2mm_stop(device);
    dma_s2mm_set_dst_addr(device, dst_addr_lsb);
    dma_s2mm_set_dst_addr_msb(device, dst_addr_msb);
    dma_s2mm_IOC_IRQ_EN(device);
    dma_s2mm_DLY_IRO_EN(device);
    dma_s2mm_ERR_IRQ_EN(device);
    dma_s2mm_run(device);
    dma_s2mm_set_length(device, size);

    // TODO: change this to a more efficient polling method
    while (!dma_s2mm_busy(device));
}