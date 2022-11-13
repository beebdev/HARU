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

// https://www.xilinx.com/support/documentation/ip_documentation/axi_dma/v7_1/pg021_axi_dma.pdf

#ifndef AXI_DMA
#define AXI_DMA

#include <stdint.h>

// AXI DMA register map offsets (Direct Register Mode)
#define AXI_DMA_MM2S_CR             0x00
#define AXI_DMA_MM2S_SR             0x04
#define AXI_DMA_MM2S_SRC_ADDR       0x18
#define AXI_DMA_MM2S_SRC_ADDR_MSB   0x1C
#define AXI_DMA_MM2S_LENGTH         0x28

#define AXI_DMA_S2MM_CR             0x30
#define AXI_DMA_S2MM_SR             0x34
#define AXI_DMA_S2MM_DST_ADDR       0x48
#define AXI_DMA_S2MM_DST_ADDR_MSB   0x4C
#define AXI_DMA_S2MM_LENGTH         0x58

// CR register bits
#define AXI_DMA_CR_RS               0x00 // Run Stop. Default = 0
#define AXI_DMA_CR_RESET            0x02 // Reset. Default = 0
#define AXI_DMA_CR_IOC_IRQ_EN       0x0c // Interrupt on Completion IRQ Enable. Default = 0
#define AXI_DMA_CR_DLY_IRQ_EN       0x0d // Delay on Timer Interrupt Enable. Default = 0
#define AXI_DMA_CR_ERR_IRQ_EN       0x0e // Error Interrupt Enable. Default = 0

// SR register bits
#define AXI_DMA_SR_HALTED           0x00 // Channel halted. 0 = Running, 1 = Halted
#define AXI_DMA_SR_IDLE             0x01 // Channel idle. 0 = Active, 1 = Idle
#define AXI_DMA_SR_SG_ACT           0x03 // Scatter/Gather Active. 0 = Inactive, 1 = Active
#define AXI_DMA_SR_DMA_INT_ERR      0x04 // DMA Internal Error. 0 = No error, 1 = Internal error
#define AXI_DMA_SR_DMA_SLV_ERR      0x05 // DMA Slave Error. 0 = No error, 1 = Slave error
#define AXI_DMA_SR_DMA_DEC_ERR      0x06 // DMA Decode Error. 0 = No error, 1 = Decode error
#define AXI_DMA_SR_SG_ERR           0x07 // DMA Scatter/Gather Error. 0 = No error, 1 = Scatter/Gather error
#define AXI_DMA_SR_SG_INT_ERR       0x08 // DMA Scatter/Gather Internal Error. 0 = No error, 1 = Error
#define AXI_DMA_SR_SG_SLV_ERR       0x09 // DMA Scatter/Gather Slave Error. 0 = No error, 1 = Slave error
#define AXI_DMA_SR_SG_DEC_ERR       0x0A // DMA Scatter/Gather Decode Error. 0 = No error, 1 = Decode error
#define AXI_DMA_SR_IOC_IRQ          0x0C // Interrupt on Completion IRQ. 0 = No IRQ, 1 = IRQ
#define AXI_DMA_SR_DLY_IRQ          0x0D // Delay on Timer IRQ. 0 = No IRQ, 1 = IRQ
#define AXI_DMA_SR_ERR_IRQ          0x0E // Error IRQ. 0 = No IRQ, 1 = IRQ

/* AXI DMA type */
typedef struct {
    // device info
    uint32_t *v_baseaddr;    // Memory mapped virtual base address
    uint32_t p_baseaddr;    // Physical base address
    uint32_t size;          // Size of device

    // src and dst buffers
    void *v_src_addr;
    void *v_dst_addr;
    uint32_t p_src_addr;
    uint32_t p_dst_addr;
} axi_dma_t;

int32_t axi_dma_init(axi_dma_t *device, uint32_t baseaddr, uint32_t src_addr, uint32_t dst_addr, uint32_t size);
void axi_dma_release(axi_dma_t *device);

void dma_mm2s_reset(axi_dma_t *device);
void dma_s2mm_reset(axi_dma_t *device);
void dma_mm2s_run(axi_dma_t *device);
void dma_mm2s_stop(axi_dma_t *device);
void dma_s2mm_run(axi_dma_t *device);
void dma_s2mm_stop(axi_dma_t *device);
void dma_mm2s_IOC_IRQ_EN(axi_dma_t *device);
void dma_mm2s_IOC_IRQ_DIS(axi_dma_t *device);
void dma_s2mm_IOC_IRQ_EN(axi_dma_t *device);
void dma_s2mm_IOC_IRQ_DIS(axi_dma_t *device);
void dma_mm2s_DLY_IRO_EN(axi_dma_t *device);
void dma_mm2s_DLY_IRO_DIS(axi_dma_t *device);
void dma_s2mm_DLY_IRO_EN(axi_dma_t *device);
void dma_s2mm_DLY_IRO_DIS(axi_dma_t *device);
void dma_mm2s_ERR_IRQ_EN(axi_dma_t *device);
void dma_mm2s_ERR_IRQ_DIS(axi_dma_t *device);
void dma_s2mm_ERR_IRQ_EN(axi_dma_t *device);
void dma_s2mm_ERR_IRQ_DIS(axi_dma_t *device);

uint8_t dma_mms2_halted(axi_dma_t *device);
uint8_t dma_s2mm_halted(axi_dma_t *device);
uint8_t dma_mms2_idle(axi_dma_t *device);
uint8_t dma_s2mm_idle(axi_dma_t *device);
uint8_t dma_mm2s_busy(axi_dma_t *device);
uint8_t dma_s2mm_busy(axi_dma_t *device);
uint8_t dma_mm2s_sg_active(axi_dma_t *device);
uint8_t dma_s2mm_sg_active(axi_dma_t *device);
uint8_t dma_mm2s_dma_internal_error(axi_dma_t *device);
uint8_t dma_s2mm_dma_internal_error(axi_dma_t *device);
uint8_t dma_mm2s_dma_slave_error(axi_dma_t *device);
uint8_t dma_s2mm_dma_slave_error(axi_dma_t *device);
uint8_t dma_mm2s_dma_decode_error(axi_dma_t *device);
uint8_t dma_s2mm_dma_decode_error(axi_dma_t *device);
uint8_t dma_mm2s_IOC_IRQ(axi_dma_t *device);
uint8_t dma_s2mm_IOC_IRQ(axi_dma_t *device);
uint8_t dma_mm2s_DLY_IRQ(axi_dma_t *device);
uint8_t dma_s2mm_DLY_IRQ(axi_dma_t *device);
uint8_t dma_mm2s_ERR_IRQ(axi_dma_t *device);
uint8_t dma_s2mm_ERR_IRQ(axi_dma_t *device);

void dma_mm2s_set_src_addr(axi_dma_t *device, uint32_t addr);
void dma_mm2s_set_src_addr_msb(axi_dma_t *device, uint32_t addr);
void dma_mm2s_set_length(axi_dma_t *device, uint32_t length);
void dma_s2mm_set_dst_addr(axi_dma_t *device, uint32_t addr);
void dma_s2mm_set_dst_addr_msb(axi_dma_t *device, uint32_t addr);
void dma_s2mm_set_length(axi_dma_t *device, uint32_t length);

void axi_dma_mm2s_transfer(axi_dma_t *device, uint32_t size);
void axi_dma_s2mm_transfer(axi_dma_t *device, uint32_t size);
void axi_dma_haru_query_transfer(axi_dma_t *device, uint32_t src_len, uint32_t dst_len);

void dma_mm2s_busy_wait(axi_dma_t *device);
void dma_s2mm_busy_wait(axi_dma_t *device);

void dma_mm2s_status(axi_dma_t *device);
void dma_s2mm_status(axi_dma_t *device);

uint32_t dma_mm2s_sr(axi_dma_t *device);
uint32_t dma_s2mm_sr(axi_dma_t *device);

#endif