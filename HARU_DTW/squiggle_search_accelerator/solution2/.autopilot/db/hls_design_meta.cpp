#include "hls_design_meta.h"
const Port_Property HLS_Design_Meta::port_props[]={
	Port_Property("ap_clk", 1, hls_in, -1, "", "", 1),
	Port_Property("ap_rst", 1, hls_in, -1, "", "", 1),
	Port_Property("ap_start", 1, hls_in, -1, "", "", 1),
	Port_Property("ap_done", 1, hls_out, -1, "", "", 1),
	Port_Property("ap_idle", 1, hls_out, -1, "", "", 1),
	Port_Property("ap_ready", 1, hls_out, -1, "", "", 1),
	Port_Property("agg_result_dist_i", 32, hls_in, 0, "ap_ovld", "in_data", 1),
	Port_Property("agg_result_dist_o", 32, hls_out, 0, "ap_ovld", "out_data", 1),
	Port_Property("agg_result_dist_o_ap_vld", 1, hls_out, 0, "ap_ovld", "out_vld", 1),
	Port_Property("agg_result_end_position", 32, hls_out, 1, "ap_vld", "out_data", 1),
	Port_Property("agg_result_end_position_ap_vld", 1, hls_out, 1, "ap_vld", "out_vld", 1),
	Port_Property("x_address0", 8, hls_out, 2, "ap_memory", "mem_address", 1),
	Port_Property("x_ce0", 1, hls_out, 2, "ap_memory", "mem_ce", 1),
	Port_Property("x_q0", 32, hls_in, 2, "ap_memory", "mem_dout", 1),
	Port_Property("x_address1", 8, hls_out, 2, "ap_memory", "MemPortADDR2", 1),
	Port_Property("x_ce1", 1, hls_out, 2, "ap_memory", "MemPortCE2", 1),
	Port_Property("x_q1", 32, hls_in, 2, "ap_memory", "MemPortDOUT2", 1),
	Port_Property("y_address0", 16, hls_out, 3, "ap_memory", "mem_address", 1),
	Port_Property("y_ce0", 1, hls_out, 3, "ap_memory", "mem_ce", 1),
	Port_Property("y_q0", 32, hls_in, 3, "ap_memory", "mem_dout", 1),
};
const char* HLS_Design_Meta::dut_name = "subsequence_search";
