#ifndef AXI_DMA
#define AXI_DMA

#include <stdint.h>

#define AXI_DMA0_ADDR 0x40400000
#define AXI_DMA1_ADDR 0x40410000
#define AXI_DMA_RANGE 0xffff
#define MM2S_CONTROL_REGISTER 0x00
#define MM2S_STATUS_REGISTER 0x04
#define MM2S_START_ADDRESS 0x18
#define MM2S_LENGTH 0x28

uint32_t *axi_dma_init(uint32_t hw_addr);
void axi_dma_release(uint32_t *dma_addr);

void axi_mm2s_transfer(uint32_t *dma_addr, uint32_t address, uint32_t length);

void dma_set(uint32_t *dma_addr, int offset, unsigned int value);
uint32_t dma_get(uint32_t *dma_addr, int offset);

int dma_mm2s_busy(uint32_t *dma_addr);
void dma_mm2s_status(uint32_t *dma_addr);

void memdump(void *addr, int byte_count);


#endif
