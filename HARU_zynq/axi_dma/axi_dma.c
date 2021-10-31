
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "axi_dma.h"

uint32_t* axi_dma_init(uint32_t hw_addr) {
    /* Open dev mem */
    int dev_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dev_fd == -1) {
        printf("Error\n");
        return NULL;
    }

    /* Memory map to axi dma device */
    uint32_t* dma_addr =
        mmap(NULL, AXI_DMA_RANGE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, hw_addr);
    dma_set(dma_addr, MM2S_CONTROL_REGISTER, 4);  // reset

    /* /dev/mem/ fd no longer needed */
    close(dev_fd);
    return dma_addr;
}

void axi_dma_release(uint32_t *hw_addr) {
    munmap(hw_addr, AXI_DMA_RANGE);
}

void axi_mm2s_transfer(uint32_t* dma_addr, uint32_t address, uint32_t length) {
    dma_set(dma_addr, MM2S_CONTROL_REGISTER, 4);  // reset
    dma_set(dma_addr, MM2S_CONTROL_REGISTER, 0);  // halt
    dma_mm2s_status(dma_addr);
    dma_set(dma_addr, MM2S_START_ADDRESS, address);    // set address
    dma_set(dma_addr, MM2S_CONTROL_REGISTER, 0xf001);  // interrupt masking and set run
    dma_set(dma_addr, MM2S_LENGTH, length);
    dma_mm2s_status(dma_addr);
}

void dma_set(uint32_t* dma_addr, int offset, unsigned int value) {
    dma_addr[offset >> 2] = value;
}

uint32_t dma_get(uint32_t* dma_addr, int offset) {
    return dma_addr[offset >> 2];
}

void dma_mm2s_status(uint32_t* dma_addr) {
    unsigned int status = dma_get(dma_addr, MM2S_STATUS_REGISTER);
    printf("Memory-mapped to stream status (0x%08x@0x%02x):", status, MM2S_STATUS_REGISTER);
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

int dma_mm2s_busy(uint32_t* dma_addr) {
    uint32_t mm2s_status = dma_get(dma_addr, MM2S_STATUS_REGISTER);
    return (!(mm2s_status & 1 << 12) || !(mm2s_status & 1 << 1));
}

void memdump(void* addr, int byte_count) {
    char* p = addr;
    int offset;
    for (offset = 0; offset < byte_count; offset++) {
        printf("%02x", p[offset]);
        if (offset % 4 == 3) {
            printf(" ");
        }
    }
    printf("\n");
}
