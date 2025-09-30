// test_dma.c
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define HARU_IOC_MAGIC   'h'
#define HARU_GET_PHYS    _IOR(HARU_IOC_MAGIC, 10, uint64_t)
#define HARU_GET_SIZE    _IOR(HARU_IOC_MAGIC, 11, uint64_t)

int main(void) {
    int fd = open("/dev/haru-dma", O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    uint64_t phys=0, size=0;
    if (ioctl(fd, HARU_GET_PHYS, &phys) < 0) { perror("ioctl phys"); return 1; }
    if (ioctl(fd, HARU_GET_SIZE, &size) < 0) { perror("ioctl size"); return 1; }

    printf("DMA phys=0x%lx size=%lu bytes\n", (unsigned long)phys, (unsigned long)size);

    void *cpu = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (cpu == MAP_FAILED) { perror("mmap"); return 1; }

    /* Touch the first page */
    ((volatile uint32_t*)cpu)[0] = 0xA5A5A5A5;
    printf("wrote first word via CPU mapping\n");

    munmap(cpu, size);
    close(fd);
    return 0;
}