// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2020.1 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XSUBSEEK_DTW_H
#define XSUBSEEK_DTW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xsubseek_dtw_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
typedef struct {
    u16 DeviceId;
    u32 Control_bus_BaseAddress;
} XSubseek_dtw_Config;
#endif

typedef struct {
    u32 Control_bus_BaseAddress;
    u32 IsReady;
} XSubseek_dtw;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XSubseek_dtw_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XSubseek_dtw_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XSubseek_dtw_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XSubseek_dtw_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XSubseek_dtw_Initialize(XSubseek_dtw *InstancePtr, u16 DeviceId);
XSubseek_dtw_Config* XSubseek_dtw_LookupConfig(u16 DeviceId);
int XSubseek_dtw_CfgInitialize(XSubseek_dtw *InstancePtr, XSubseek_dtw_Config *ConfigPtr);
#else
int XSubseek_dtw_Initialize(XSubseek_dtw *InstancePtr, const char* InstanceName);
int XSubseek_dtw_Release(XSubseek_dtw *InstancePtr);
#endif

void XSubseek_dtw_Start(XSubseek_dtw *InstancePtr);
u32 XSubseek_dtw_IsDone(XSubseek_dtw *InstancePtr);
u32 XSubseek_dtw_IsIdle(XSubseek_dtw *InstancePtr);
u32 XSubseek_dtw_IsReady(XSubseek_dtw *InstancePtr);
void XSubseek_dtw_EnableAutoRestart(XSubseek_dtw *InstancePtr);
void XSubseek_dtw_DisableAutoRestart(XSubseek_dtw *InstancePtr);

u32 XSubseek_dtw_Get_agg_result_ID(XSubseek_dtw *InstancePtr);
u32 XSubseek_dtw_Get_agg_result_ID_vld(XSubseek_dtw *InstancePtr);
u32 XSubseek_dtw_Get_agg_result_pos(XSubseek_dtw *InstancePtr);
u32 XSubseek_dtw_Get_agg_result_pos_vld(XSubseek_dtw *InstancePtr);
u32 XSubseek_dtw_Get_agg_result_dist(XSubseek_dtw *InstancePtr);
u32 XSubseek_dtw_Get_agg_result_dist_vld(XSubseek_dtw *InstancePtr);

void XSubseek_dtw_InterruptGlobalEnable(XSubseek_dtw *InstancePtr);
void XSubseek_dtw_InterruptGlobalDisable(XSubseek_dtw *InstancePtr);
void XSubseek_dtw_InterruptEnable(XSubseek_dtw *InstancePtr, u32 Mask);
void XSubseek_dtw_InterruptDisable(XSubseek_dtw *InstancePtr, u32 Mask);
void XSubseek_dtw_InterruptClear(XSubseek_dtw *InstancePtr, u32 Mask);
u32 XSubseek_dtw_InterruptGetEnabled(XSubseek_dtw *InstancePtr);
u32 XSubseek_dtw_InterruptGetStatus(XSubseek_dtw *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
