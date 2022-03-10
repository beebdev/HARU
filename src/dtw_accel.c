#include "dtw_accel.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

void _reg_set(uint32_t baseaddr, uint32_t offset, uint32_t data);
uint32_t _reg_get(uint32_t baseaddr, uint32_t offset);


// Initialise the device, return 0 on success, non-0 on failure
// This function will map the device into memory, and set the base address
int32_t dtw_accel_init(dtw_accel_t *device, uint32_t baseaddr, uint32_t size) {
    int dev_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dev_fd < 0) {
        return -1;
    }

    device->size = size;
    device->p_baseaddr = baseaddr;
    device->v_baseaddr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, baseaddr);

    if (device->v_baseaddr == MAP_FAILED) {
        close(dev_fd);
        return -1;
    }

    dtw_accel_reset_EN(device);
    dtw_accel_reset_DIS(device);
    close(dev_fd);
    return 0;
}

// Release the device, return 0 on success, -1 on failure
// This function will unmap the device from memory
int32_t dtw_accel_release(dtw_accel_t *device) {
    return (int32_t) munmap(device->v_baseaddr, device->size);
}

// Reset the device
// Sets CR to reset
void dtw_accel_reset_EN(dtw_accel_t *device) {
    _reg_set(device->v_baseaddr, DTW_ACCEL_CR_ADDR,  (1 << DTW_ACCEL_CR_OFFSET_RESET));
}

void dtw_accel_reset_DIS(dtw_accel_t *device) {
    _reg_set(device->v_baseaddr, DTW_ACCEL_CR_ADDR,  (0 << DTW_ACCEL_CR_OFFSET_RESET));
}

// Start the device
void dtw_accel_run(dtw_accel_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, DTW_ACCEL_CR_ADDR);
    _reg_set(device->v_baseaddr, DTW_ACCEL_CR_ADDR, cr | (1 << DTW_ACCEL_CR_OFFSET_RS));
}

// Stop the device
void dtw_accel_stop(dtw_accel_t *device) {
    uint32_t cr = _reg_get(device->v_baseaddr, DTW_ACCEL_CR_ADDR);
    _reg_set(device->v_baseaddr, DTW_ACCEL_CR_ADDR, cr & ~(1 << DTW_ACCEL_CR_OFFSET_RS));
}

// Set the mode of the device
void dtw_accel_set_mode(dtw_accel_t *device, uint8_t mode) {
    uint32_t cr = _reg_get(device->v_baseaddr, DTW_ACCEL_CR_ADDR);
    if (mode == 0) { // turn off
        _reg_set(device->v_baseaddr, DTW_ACCEL_CR_ADDR, cr & ~(1 << DTW_ACCEL_CR_OFFSET_MODE));
    } else if (mode == 1) { // turn on
        _reg_set(device->v_baseaddr, DTW_ACCEL_CR_ADDR, cr | (1 << DTW_ACCEL_CR_OFFSET_MODE));
    }
}

// Get the busy status of the device
uint32_t dtw_accel_busy(dtw_accel_t *device) {
    return _reg_get(device->v_baseaddr, DTW_ACCEL_SR_ADDR) & (1 << DTW_ACCEL_SR_OFFSET_BUSY);
}

// Get the done status of the device
uint32_t dtw_accel_done(dtw_accel_t *device) {
    return _reg_get(device->v_baseaddr, DTW_ACCEL_SR_ADDR) & (1 << DTW_ACCEL_SR_OFFSET_REF_LOAD_DONE);
}

// Set register
void _reg_set(uint32_t baseaddr, uint32_t offset, uint32_t data) {
    *(volatile uint32_t *)(baseaddr + offset) = data;
    // baseaddr[offset>>2] = data;
}

// Get register
uint32_t _reg_get(uint32_t baseaddr, uint32_t offset) {
    return *(uint32_t *)(baseaddr + offset);
    // return baseaddr[offset>>2];
}