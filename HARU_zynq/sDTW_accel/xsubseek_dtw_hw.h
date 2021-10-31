// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2020.1 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// CONTROL_BUS
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
//        bit 7  - auto_restart (Read/Write)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x10 : Data signal of agg_result_ID
//        bit 0  - agg_result_ID[0] (Read)
//        others - reserved
// 0x14 : Control signal of agg_result_ID
//        bit 0  - agg_result_ID_ap_vld (Read/COR)
//        others - reserved
// 0x18 : Data signal of agg_result_pos
//        bit 31~0 - agg_result_pos[31:0] (Read)
// 0x1c : Control signal of agg_result_pos
//        bit 0  - agg_result_pos_ap_vld (Read/COR)
//        others - reserved
// 0x20 : Data signal of agg_result_dist
//        bit 31~0 - agg_result_dist[31:0] (Read)
// 0x24 : Control signal of agg_result_dist
//        bit 0  - agg_result_dist_ap_vld (Read/COR)
//        others - reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XSUBSEEK_DTW_CONTROL_BUS_ADDR_AP_CTRL              0x00
#define XSUBSEEK_DTW_CONTROL_BUS_ADDR_GIE                  0x04
#define XSUBSEEK_DTW_CONTROL_BUS_ADDR_IER                  0x08
#define XSUBSEEK_DTW_CONTROL_BUS_ADDR_ISR                  0x0c
#define XSUBSEEK_DTW_CONTROL_BUS_ADDR_AGG_RESULT_ID_DATA   0x10
#define XSUBSEEK_DTW_CONTROL_BUS_BITS_AGG_RESULT_ID_DATA   1
#define XSUBSEEK_DTW_CONTROL_BUS_ADDR_AGG_RESULT_ID_CTRL   0x14
#define XSUBSEEK_DTW_CONTROL_BUS_ADDR_AGG_RESULT_POS_DATA  0x18
#define XSUBSEEK_DTW_CONTROL_BUS_BITS_AGG_RESULT_POS_DATA  32
#define XSUBSEEK_DTW_CONTROL_BUS_ADDR_AGG_RESULT_POS_CTRL  0x1c
#define XSUBSEEK_DTW_CONTROL_BUS_ADDR_AGG_RESULT_DIST_DATA 0x20
#define XSUBSEEK_DTW_CONTROL_BUS_BITS_AGG_RESULT_DIST_DATA 32
#define XSUBSEEK_DTW_CONTROL_BUS_ADDR_AGG_RESULT_DIST_CTRL 0x24

