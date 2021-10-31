// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2020.1 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xsubseek_dtw.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XSubseek_dtw_CfgInitialize(XSubseek_dtw *InstancePtr, XSubseek_dtw_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_bus_BaseAddress = ConfigPtr->Control_bus_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XSubseek_dtw_Start(XSubseek_dtw *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_AP_CTRL) & 0x80;
    XSubseek_dtw_WriteReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_AP_CTRL, Data | 0x01);
}

u32 XSubseek_dtw_IsDone(XSubseek_dtw *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XSubseek_dtw_IsIdle(XSubseek_dtw *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XSubseek_dtw_IsReady(XSubseek_dtw *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XSubseek_dtw_EnableAutoRestart(XSubseek_dtw *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSubseek_dtw_WriteReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_AP_CTRL, 0x80);
}

void XSubseek_dtw_DisableAutoRestart(XSubseek_dtw *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSubseek_dtw_WriteReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_AP_CTRL, 0);
}

u32 XSubseek_dtw_Get_agg_result_ID(XSubseek_dtw *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_AGG_RESULT_ID_DATA);
    return Data;
}

u32 XSubseek_dtw_Get_agg_result_ID_vld(XSubseek_dtw *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_AGG_RESULT_ID_CTRL);
    return Data & 0x1;
}

u32 XSubseek_dtw_Get_agg_result_pos(XSubseek_dtw *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_AGG_RESULT_POS_DATA);
    return Data;
}

u32 XSubseek_dtw_Get_agg_result_pos_vld(XSubseek_dtw *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_AGG_RESULT_POS_CTRL);
    return Data & 0x1;
}

u32 XSubseek_dtw_Get_agg_result_dist(XSubseek_dtw *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_AGG_RESULT_DIST_DATA);
    return Data;
}

u32 XSubseek_dtw_Get_agg_result_dist_vld(XSubseek_dtw *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_AGG_RESULT_DIST_CTRL);
    return Data & 0x1;
}

void XSubseek_dtw_InterruptGlobalEnable(XSubseek_dtw *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSubseek_dtw_WriteReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_GIE, 1);
}

void XSubseek_dtw_InterruptGlobalDisable(XSubseek_dtw *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSubseek_dtw_WriteReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_GIE, 0);
}

void XSubseek_dtw_InterruptEnable(XSubseek_dtw *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_IER);
    XSubseek_dtw_WriteReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_IER, Register | Mask);
}

void XSubseek_dtw_InterruptDisable(XSubseek_dtw *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_IER);
    XSubseek_dtw_WriteReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_IER, Register & (~Mask));
}

void XSubseek_dtw_InterruptClear(XSubseek_dtw *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSubseek_dtw_WriteReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_ISR, Mask);
}

u32 XSubseek_dtw_InterruptGetEnabled(XSubseek_dtw *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_IER);
}

u32 XSubseek_dtw_InterruptGetStatus(XSubseek_dtw *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XSubseek_dtw_ReadReg(InstancePtr->Control_bus_BaseAddress, XSUBSEEK_DTW_CONTROL_BUS_ADDR_ISR);
}

