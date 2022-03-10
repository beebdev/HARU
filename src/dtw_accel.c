#include "dtw_accel.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

void _reg_set(uint32_t baseaddr, uint32_t offset, uint32_t data);
uint32_t _reg_get(uint32_t baseaddr, uint32_t offset);

int32_t dtw_accel_initialise(dtw_accel_t *device) {
    int dev_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dev_fd < 0) {
        return -1;
    }

    device->baseaddr = mmap(NULL, DTW_ACCEL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, DTW_ACCEL_BASE);
    dtw_accel_reset(device);
    close(dev_fd);
    return 0;
}

int32_t dtw_accel_release(dtw_accel_t *device) {
    return (int32_t) munmap(device->baseaddr, DTW_ACCEL_SIZE);
}

void dtw_accel_reset(dtw_accel_t *device) {
    uint32_t ctrl_reg = _reg_get(device->baseaddr, DTW_ACCEL_CR_ADDR);
    _reg_set(device->baseaddr, DTW_ACCEL_CR_ADDR, ctrl_reg & (1 << DTW_ACCEL_CR_OFFSET_RESET));
}

void dtw_accel_start(dtw_accel_t *device) {
    _reg_set(device->baseaddr, DTW_ACCEL_CR_ADDR, DTW_ACCEL_CR_START);
}

void dtw_accel_set_mode(dtw_accel_t *device, uint16_t mode) {
    _reg_set(device->baseaddr, DTW_ACCEL_CR_ADDR, mode << 2 && DTW_ACCEL_CR_MODE);
}

uint32_t dtw_accel_busy(dtw_accel_t *device) {
    return _reg_get(device->baseaddr, DTW_ACCEL_SR_ADDR) & DTW_ACCEL_SR_BUSY;
}

uint32_t dtw_accel_done(dtw_accel_t *device) {

}