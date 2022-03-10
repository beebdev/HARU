#ifndef DTW_ACCEL_H
#define DTW_ACCEL_H

#include <stdint.h>

#define DTW_ACCEL_BASE 0xa0010000
#define DTW_ACCEL_SIZE 0xffff

#define DTW_ACCEL_CR_ADDR 0x00
#define DTW_ACCEL_SR_ADDR 0x04

#define DTW_ACCEL_CR_OFFSET_RESET  0x00
#define DTW_ACCEL_CR_OFFSET_START  0x01
#define DTW_ACCEL_CR_OFFSET_MODE   0x02

#define DTW_ACCEL_SR_OFFSET_BUSY   0x00
#define DTW_ACCEL_SR_OFFSET_REF_LOAD_DONE   0x01

typedef struct {
    uint32_t baseaddr;
} dtw_accel_t;


int32_t dtw_accel_initialise(dtw_accel_t *device);
int32_t dtw_accel_release(dtw_accel_t *device);

void dtw_accel_reset(dtw_accel_t *device);
void dtw_accel_start(dtw_accel_t *device);
void dtw_accel_set_mode(dtw_accel_t *device, uint16_t mode);

uint32_t dtw_accel_busy(dtw_accel_t *device);
uint32_t dtw_accel_done(dtw_accel_t *device);

#endif