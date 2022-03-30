#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <stdint.h>

// TODO: add defines of error codes

/*
 * Register getter and setter
 */
#define _reg_set(BaseAddress, RegOffset, Data) \
    *(volatile uint32_t*)((BaseAddress) + (RegOffset >> 2)) = (uint32_t)(Data)
#define _reg_get(BaseAddress, RegOffset) \
    *(volatile uint32_t*)((BaseAddress) + (RegOffset >> 2))

#define HARU_INFO(msg) \
    fprintf(stderr, "INFO: %s:%d: ", __FILE__, __LINE__); \
    fprintf(stderr, "%s", msg);

// uint32_t haru_errno = 0;

#endif // MISC_H