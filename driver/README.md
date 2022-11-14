# Userspace Device Driver for haru-core-dtw

The device driver in this directory is a userspace driver that exposes simple API calls for prompt sDTW mapping. By running `make` in this directory, a basic example test of the sDTW mapping will be built to run with the accelerator.

## API documentation

### Initialization
```c
int32_t haru_init(haru_t *haru);
```
Initializes the driver for the AXI DMA module and sDTW accelerator haru controller. In the underlying implementation, the AXI DMA initialization memory maps the physical address of the AXI DMA device, the physical address of two unused section on the DDR memory for AXI-stream TX/RX transfer buffers. The `dtw_accel` initialization memory maps the physical address of the dtw_accel device to the program address space.

### Release
```c
int32_t haru_release(haru_t *haru);
```
This call cleans up the initialized constructs for HARU, which includes the memory mapped AXI DMA and DTW Accel device and corresponding transfer buffers.

### Key check
```c
void haru_check_key(haru_t *haru);
```
To make sure the provided physical address of the AXI Lite devices are correct and mapped successfully, the `haru_check_key` tries to access the key register on the DTW Accel device and compares with the expexted value which is `0x0catca7e`. If this fails, it may be caused by incorrect physical address of devices and you should double check the address provided in the header files and the address asigned to the devices in the Vivado project (Open block design -> address editor tab)

### Get version
```c
uint32_t haru_get_version(haru_t *haru);
```
This call obtains the version registor on the device and prints it out to standard error.

### Reference loading check
```c
void haru_get_load_done(haru_t *haru);
```
At the moment, the reference signal needs to be loaded onto the device before query searches. This call checks whether the reference signal has been loaded.

### Load reference
```c
int32_t haru_load_reference(haru_t *haru, int32_t *ref, uint32_t size);
```
This call take in the `haru_t` construct, reference signal, and the length of the reference and loads it to the BRAM of the accelerator through AXI Stream. This has to be called before the query calls.

### Process Query
```c
void haru_process_query(haru_t *haru, int32_t *query, uint32_t size, search_result_t *results);
```
This call takes in the `haru_t` construct, query signal, size of query size, and pointer to the results struct. It transfers the query signal to the accelerator which processes the mapping and returns the results (ID, position, and accumulated score) for post-processing.

## Example
See [src/main](https://github.com/beebdev/HARU/tree/main/driver/src/main.c) for a basic example usage of the API. You can run `make` in this directory to build the example to run with the accelerator.

For the POC selective seqeuncing system using this core, see [sigfish-haru](https://github.com/beebdev/sigfish-haru). 
