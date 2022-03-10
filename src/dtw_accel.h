#ifndef DTW_ACCEL_H
#define DTW_ACCEL_H

#include <stdint.h>

#define DTW_ACCEL_CR_ADDR                   0x00
#define DTW_ACCEL_SR_ADDR                   0x04

#define DTW_ACCEL_CR_OFFSET_RESET           0x00
#define DTW_ACCEL_CR_OFFSET_RS              0x01
#define DTW_ACCEL_CR_OFFSET_MODE            0x02

#define DTW_ACCEL_SR_OFFSET_BUSY            0x00
#define DTW_ACCEL_SR_OFFSET_REF_LOAD_DONE   0x01

#define DTW_ACCEL_MODE_QUERY                0x00
#define DTW_ACCEL_MODE_REF_LOAD             0x01

typedef struct {
    uint32_t v_baseaddr;    // Memory mapped virtual base address
    uint32_t p_baseaddr;    // Physical base address
    uint32_t size;          // Size of device
} dtw_accel_t;

int32_t dtw_accel_init(dtw_accel_t *device, uint32_t baseaddr, uint32_t size);
int32_t dtw_accel_release(dtw_accel_t *device);

void dtw_accel_reset_EN(dtw_accel_t *device);
void dtw_accel_reset_DIS(dtw_accel_t *device);
void dtw_accel_run(dtw_accel_t *device);
void dtw_accel_stop(dtw_accel_t *device);
void dtw_accel_set_mode(dtw_accel_t *device, uint8_t mode);

uint32_t dtw_accel_busy(dtw_accel_t *device);
uint32_t dtw_accel_done(dtw_accel_t *device);

#endif