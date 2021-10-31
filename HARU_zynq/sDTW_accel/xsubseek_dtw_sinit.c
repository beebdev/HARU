// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2020.1 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xsubseek_dtw.h"

extern XSubseek_dtw_Config XSubseek_dtw_ConfigTable[];

XSubseek_dtw_Config *XSubseek_dtw_LookupConfig(u16 DeviceId) {
	XSubseek_dtw_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XSUBSEEK_DTW_NUM_INSTANCES; Index++) {
		if (XSubseek_dtw_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XSubseek_dtw_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XSubseek_dtw_Initialize(XSubseek_dtw *InstancePtr, u16 DeviceId) {
	XSubseek_dtw_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XSubseek_dtw_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XSubseek_dtw_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif

