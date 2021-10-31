/***********************************************************************
 * FILENAME: haru.h
 * DESCRIPTION:
 *      HARU library header file. Contains functions to initialise the HARU sDTW
 *      system and run the accelerator.
 * NOTES:
 *
 * AUTHOR: Elton Shih        START DATE: 28/09/2021
 * CHANGES:
 *
 * VERSION DATE       WHO     DETAIL
 * v0.01   29/09/2021 ES      Init Draft version
 *
 **/

#ifndef HARU_HEADER
#define HARU_HEADER

#include <stdint.h>
#include "../axi_dma/axi_dma.h"
#include "../sDTW_accel/xsubseek_dtw.h"

/* Definitions */
#define QUERY_LEN 250
#define REF_LEN 48497
#define BUFFER_PADDR 0x0e000000
#define BUFFER_SIZE 0xffff

/* Type definitions */
typedef struct haru_t {
    uint32_t *query_dma;
    int32_t* buffer_vaddr;
    XSubseek_dtw sdtw;
} haru_t;

typedef struct query_t {
    int ID;
    int position;
    int min_dist;
} query_t;

typedef int16_t value_t;

/* Function prototypes */
int haru_init(haru_t* haru);
int haru_cleanup(haru_t* haru);

int query_init(query_t* query, int ID);
void haru_process_query(haru_t* haru, value_t* query_addr, query_t* results);

#endif
