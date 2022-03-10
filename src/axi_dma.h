// https://www.xilinx.com/support/documentation/ip_documentation/axi_dma/v7_1/pg021_axi_dma.pdf

#ifndef AXI_DMA
#define AXI_DMA

#include <stdint.h>

// AXI DMA device address
#define AXI_DMA_BASE 0xa0000000 // TODO: update this
#define AXI_DMA_SIZE 0xffff

// AXI DMA register map (Direct Register Mode)
#define AXI_DMA_MM2S_CR 0x00
#define AXI_DMA_MM2S_SR 0x04
#define AXI_DMA_MM2S_SRC_ADDR 0x18
#define AXI_DMA_MM2S_SRC_ADDR_MSB 0x1C
#define AXI_DMA_MM2S_LENGTH 0x28

#define AXI_DMA_S2MM_CR 0x30
#define AXI_DMA_S2MM_SR 0x34
#define AXI_DMA_S2MM_DST_ADDR 0x48
#define AXI_DMA_S2MM_DST_ADDR_MSB 0x4C
#define AXI_DMA_S2MM_LENGTH 0x58

// CR register bits
#define AXI_DMA_CR_RS 0x00 // Run Stop. Default = 0
#define AXI_DMA_CR_RESET 0x02 // Reset. Default = 0
#define AXI_DMA_CR_IOC_IRQ_EN 0x012 // Interrupt on Completion IRQ Enable. Default = 0
#define AXI_DMA_CR_DLY_IRQ_EN 0x013 // Delay on Timer Interrupt Enable. Default = 0
#define AXI_DMA_CR_ERR_IRQ_EN 0x014 // Error Interrupt Enable. Default = 0

// SR register bits
#define AXI_DMA_SR_HALTED 0x00 // Channel halted. 0 = Running, 1 = Halted
#define AXI_DMA_SR_IDLE 0x01 // Channel idle. 0 = Active, 1 = Idle
#define AXI_DMA_SR_SG_ACT 0x03 // Scatter/Gather Active. 0 = Inactive, 1 = Active
#define AXI_DMA_SR_DMA_INT_ERR 0x04 // DMA Internal Error. 0 = No error, 1 = Internal error
#define AXI_DMA_SR_DMA_SLV_ERR 0x05 // DMA Slave Error. 0 = No error, 1 = Slave error
#define AXI_DMA_SR_DMA_DEC_ERR 0x06 // DMA Decode Error. 0 = No error, 1 = Decode error
#define AXI_DMA_SR_SG_ERR 0x07 // DMA Scatter/Gather Error. 0 = No error, 1 = Scatter/Gather error
#define AXI_DMA_SR_SG_INT_ERR 0x08 // DMA Scatter/Gather Internal Error. 0 = No error, 1 = Error
#define AXI_DMA_SR_SG_SLV_ERR 0x09 // DMA Scatter/Gather Slave Error. 0 = No error, 1 = Slave error
#define AXI_DMA_SR_SG_DEC_ERR 0x0A // DMA Scatter/Gather Decode Error. 0 = No error, 1 = Decode error
#define AXI_DMA_SR_IOC_IRQ 0x0C // Interrupt on Completion IRQ. 0 = No IRQ, 1 = IRQ
#define AXI_DMA_SR_DLY_IRQ 0x0D // Delay on Timer IRQ. 0 = No IRQ, 1 = IRQ
#define AXI_DMA_SR_ERR_IRQ 0x0E // Error IRQ. 0 = No IRQ, 1 = IRQ

/* AXI DMA type */
typedef struct {
    volatile uint32_t control;
    volatile uint32_t status;
    volatile uint32_t source_addr;
    volatile uint32_t dest_addr;
    volatile uint32_t transfer_size;
    volatile uint32_t stride;
    volatile uint32_t next_descriptor;
} axi_dma_t;

uint32_t axi_dma_init();
void axi_dma_release();
void axi_dma_mm2s_transfer(uint32_t *axi_dma_addr, uint32_t src_addr, uint32_t transfer_size);
void axi_dma_s2mm_transfer(uint32_t *axi_dma_addr, uint32_t dest_addr, uint32_t transfer_size);
void memdump(uint32_t *axi_dma_addr, uint32_t addr, uint32_t size);

#endif