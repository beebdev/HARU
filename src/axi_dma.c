#include "axi_dma.h"
#include "misc.h"

#include <stdio.h> // todo: remove this once debugging is finished
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

/*
 * AXI DMA general function
 */
int32_t axi_dma_init(axi_dma_t *device, uint32_t baseaddr, uint32_t src_addr, uint32_t dst_addr, uint32_t size) {
    printf("[%s] Initializing AXI DMA\n", __func__);

    // Open /dev/mem for memory mapping
    int32_t dev_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dev_fd < 0) {
        return -1;
    }

    // Init device
    device->size = size;
    device->p_baseaddr = baseaddr;
    device->v_baseaddr = (uint32_t *) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, baseaddr);
    if (device->v_baseaddr == MAP_FAILED) {
        printf("[axi_dma] Initialize failed\n");
        close(dev_fd);
        return -1;
    }

    if (dma_mm2s_sg_active(device) || dma_s2mm_sg_active(device)) {
        printf("[axi_dma] DMA SG is active, aborting..\n");
        close(dev_fd);
        return -1;
    }
    dma_mm2s_reset(device);
    dma_s2mm_reset(device);

    // Init buffers
    device->p_src_addr = src_addr;
    device->v_src_addr = (void *) mmap(NULL, 0xffff, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, src_addr);
    if (device->v_src_addr == MAP_FAILED) {
        printf("[axi_dma] Initialize failed\n");
        munmap(device->v_baseaddr, device->size);
        close(dev_fd);
        return -1;
    }
    
    device->p_dst_addr = dst_addr;
    device->v_dst_addr = (void *) mmap(NULL, 0xffff, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, dst_addr);
    if (device->v_dst_addr == MAP_FAILED) {
        printf("[axi_dma] Initialize failed\n");
        munmap(device->v_baseaddr, device->size);
        munmap(device->v_src_addr, device->size);
        close(dev_fd);
        return -1;
    }

    close(dev_fd);
    return 0;
}

void axi_dma_release(axi_dma_t *device) {
    munmap(device->v_baseaddr, device->size);
    munmap(device->v_src_addr, device->size);
    munmap(device->v_dst_addr, device->size);
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
    printf("mm2s transferred %d bytes\n", size);
    HARU_INFO("mm2s transfer done\n");
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
    printf("s2mm transferred %d bytes\n", size);
    HARU_INFO("s2mm transfer done\n");
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
    printf("mm2s transferred %d bytes\n", src_len);
    printf("s2mm transferred %d bytes\n", dst_len);
    HARU_INFO("s2mm and mm2s transfer done\n");
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

void dma_mm2s_set_src_addr(axi_dma_t *device, uint32_t addr) {
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_SRC_ADDR, addr);
}

void dma_mm2s_set_src_addr_msb(axi_dma_t *device, uint32_t addr) {
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_SRC_ADDR_MSB, addr);
}


// Sets the number of bytes (length) for the mm2s transfer
// Writing to this register will start the transfer
void dma_mm2s_set_length(axi_dma_t *device, uint32_t length) {
    _reg_set(device->v_baseaddr, AXI_DMA_MM2S_LENGTH, length * 4);
}

void dma_s2mm_set_dst_addr(axi_dma_t *device, uint32_t addr) {
    _reg_set(device->v_baseaddr, AXI_DMA_S2MM_DST_ADDR, addr);
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