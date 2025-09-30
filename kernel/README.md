# HARU Kernel Module

This directory contains kernel module code for allocating memory from the CMA (Contiguous Memory Allocator) pool. The allocated memory is used as transfer buffers for AXI DMA operations.

## Purpose

- Efficiently allocate contiguous memory for high-performance DMA transfers.
- Support AXI DMA hardware integration.

## Files

- Source code for kernel module implementation.
- Build scripts and configuration files.
- Smoke test functions in the `tests/` directory.

## Usage

The recommended way to build and test the module is directly on the target system.

```sh
make
sudo insmod haru_dma.ko
# or if you want to alloc a particular size
sudo insmod haru_dma.ko buf_size_kib=2048 # 2MiB

dmesg | tail
ls -l /dev/haru-dma
```

For the test_dma.c tester, build with:
```sh
gcc -O2 -Wall -o test_dma test_dma.c
```

Refer to the module source code and comments for integration and usage instructions.