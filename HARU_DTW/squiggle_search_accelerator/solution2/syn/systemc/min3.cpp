// ==============================================================
// RTL generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and OpenCL
// Version: 2020.1
// Copyright (C) 1986-2020 Xilinx, Inc. All Rights Reserved.
// 
// ===========================================================

#include "min3.h"
#include "AESL_pkg.h"

using namespace std;

namespace ap_rtl {

const sc_logic min3::ap_const_logic_1 = sc_dt::Log_1;
const bool min3::ap_const_boolean_1 = true;
const sc_logic min3::ap_const_logic_0 = sc_dt::Log_0;

min3::min3(sc_module_name name) : sc_module(name), mVcdFile(0) {

    SC_METHOD(thread_ap_ready);

    SC_METHOD(thread_ap_return);
    sensitive << ( c );
    sensitive << ( select_ln7_fu_32_p3 );
    sensitive << ( icmp_ln11_fu_40_p2 );

    SC_METHOD(thread_icmp_ln11_fu_40_p2);
    sensitive << ( c );
    sensitive << ( select_ln7_fu_32_p3 );

    SC_METHOD(thread_icmp_ln7_fu_26_p2);
    sensitive << ( a );
    sensitive << ( b );

    SC_METHOD(thread_select_ln7_fu_32_p3);
    sensitive << ( a );
    sensitive << ( b );
    sensitive << ( icmp_ln7_fu_26_p2 );

    static int apTFileNum = 0;
    stringstream apTFilenSS;
    apTFilenSS << "min3_sc_trace_" << apTFileNum ++;
    string apTFn = apTFilenSS.str();
    mVcdFile = sc_create_vcd_trace_file(apTFn.c_str());
    mVcdFile->set_time_unit(1, SC_PS);
    if (1) {
#ifdef __HLS_TRACE_LEVEL_PORT_HIER__
    sc_trace(mVcdFile, ap_ready, "(port)ap_ready");
    sc_trace(mVcdFile, a, "(port)a");
    sc_trace(mVcdFile, b, "(port)b");
    sc_trace(mVcdFile, c, "(port)c");
    sc_trace(mVcdFile, ap_return, "(port)ap_return");
#endif
#ifdef __HLS_TRACE_LEVEL_INT__
    sc_trace(mVcdFile, icmp_ln7_fu_26_p2, "icmp_ln7_fu_26_p2");
    sc_trace(mVcdFile, select_ln7_fu_32_p3, "select_ln7_fu_32_p3");
    sc_trace(mVcdFile, icmp_ln11_fu_40_p2, "icmp_ln11_fu_40_p2");
#endif

    }
}

min3::~min3() {
    if (mVcdFile) 
        sc_close_vcd_trace_file(mVcdFile);

}

void min3::thread_ap_ready() {
    ap_ready = ap_const_logic_1;
}

void min3::thread_ap_return() {
    ap_return = (!icmp_ln11_fu_40_p2.read()[0].is_01())? sc_lv<32>(): ((icmp_ln11_fu_40_p2.read()[0].to_bool())? c.read(): select_ln7_fu_32_p3.read());
}

void min3::thread_icmp_ln11_fu_40_p2() {
    icmp_ln11_fu_40_p2 = (!select_ln7_fu_32_p3.read().is_01() || !c.read().is_01())? sc_lv<1>(): (sc_bigint<32>(select_ln7_fu_32_p3.read()) > sc_bigint<32>(c.read()));
}

void min3::thread_icmp_ln7_fu_26_p2() {
    icmp_ln7_fu_26_p2 = (!b.read().is_01() || !a.read().is_01())? sc_lv<1>(): (sc_bigint<32>(b.read()) < sc_bigint<32>(a.read()));
}

void min3::thread_select_ln7_fu_32_p3() {
    select_ln7_fu_32_p3 = (!icmp_ln7_fu_26_p2.read()[0].is_01())? sc_lv<32>(): ((icmp_ln7_fu_26_p2.read()[0].to_bool())? b.read(): a.read());
}

}
