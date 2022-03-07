#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "axi_dma.h"

void dma_set(uint32_t* hw_addr, int offset, unsigned int value) {
    hw_addr[offset >> 2] = value;
}

uint32_t dma_get(uint32_t* hw_addr, int offset) {
    return hw_addr[offset >> 2];
}


uint32_t axi_dma_init(uint32_t hw_addr) {
    // Open dev mem
    int dev_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dev_fd == -1) {
        printf("Error\n");
        return NULL;
    }

    // Map AXI DMA memory
    uint32_t *dma_addr = mmap(NULL, AXI_DMA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, hw_addr);
    if (dma_addr == MAP_FAILED) {
        printf("Error\n");
        return NULL;
    }

    dma_set(dma_addr, AXI_DMA_MM2S_CR, AXI_DMA_CR_RESET);
    close(dev_fd);

    return dma_addr;
}

void axi_dma_release(uint32_t hw_addr) {
    munmap(hw_addr, AXI_DMA_SIZE);
}

int dma_mm2s_busy(uint32_t* hw_addr) {
    uint32_t mm2s_status = dma_get(hw_addr, AXI_DMA_MM2S_SR);
    return (!(mm2s_status & 1 << 12) || !(mm2s_status & 1 << 1));
}

void memdump(uint32_t *axi_dma_addr, uint32_t addr, uint32_t size) {
    char* p = addr;
    for (int i = 0; i < size; i++) {
        printf("%02x", p[i]);
        if (i % 4 == 3) {
            printf(" ");
        }
    }
    printf("\n");
}