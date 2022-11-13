/* MIT License

Copyright (c) 2022 Po Jui Shih
Copyright (c) 2022 Hassaan Saadat
Copyright (c) 2022 Sri Parameswaran
Copyright (c) 2022 Hasindu Gamaarachchi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

`timescale 1ns/1ps

`define AXIS_IN_TUSER_EN
`define AXIS_OUT_TUSER_EN

module tb_dtw_accel #(
    parameter ADDR_WIDTH          = 32,
    parameter DATA_WIDTH          = 32,
    parameter AXIS_DATA_WIDTH     = 32,
    parameter STROBE_WIDTH        = (DATA_WIDTH / 8)
)(
    input                               clk,
    input                               rst,

    //Write Address Channel
    input                               aximl_awvalid,
    input       [ADDR_WIDTH - 1: 0]     aximl_awaddr,
    output                              aximl_awready,

    //Write Data Channel
    input                               aximl_wvalid,
    output                              aximl_wready,
    input       [STROBE_WIDTH - 1:0]    aximl_wstrb,
    input       [DATA_WIDTH - 1: 0]     aximl_wdata,

    //Write Response Channel
    output                              aximl_bvalid,
    input                               aximl_bready,
    output      [1:0]                   aximl_bresp,

    //Read Address Channel
    input                               aximl_arvalid,
    output                              aximl_arready,
    input       [ADDR_WIDTH - 1: 0]     aximl_araddr,

    //Read Data Channel
    output                              aximl_rvalid,
    input                               aximl_rready,
    output      [1:0]                   aximl_rresp,
    output      [DATA_WIDTH - 1: 0]     aximl_rdata,

    //AXIS IN Stream
    input                               axis_clk,
    input                               axis_rst,

    `ifdef AXIS_IN_TUSER_EN
    input                               axis_in_tuser,
    `endif
    input                               axis_in_tvalid,
    output                              axis_in_tready,
    input                               axis_in_tlast,
    input       [AXIS_DATA_WIDTH - 1:0] axis_in_tdata,

    `ifdef AXIS_OUT_TUSER_EN
    input                               axis_out_tuser,
    `endif
    input                               axis_out_tvalid,
    output                              axis_out_tready,
    input                               axis_out_tlast,
    input       [AXIS_DATA_WIDTH - 1:0] axis_out_tdata
);

/* ===============================
 * Registers
 * =============================== */
reg               r_rst;
reg               r_axis_rst;
reg [7:0] 	      test_id       = 0;

// Workaround for weird icarus simulator bug
always @ (*)      r_rst         = rst;
always @ (*)      r_axis_rst    = axis_rst;

/* ===============================
 * submodules
 * =============================== */
dtw_accel #(
    .ADDR_WIDTH       (ADDR_WIDTH),
    .DATA_WIDTH       (DATA_WIDTH),
    .AXIS_DATA_WIDTH  (AXIS_DATA_WIDTH),
    .INVERT_AXI_RESET (0),
    .INVERT_AXIS_RESET(0)
) dut (
    .S_AXI_clk        (clk),
    .S_AXI_rst        (r_rst),

    .S_AXI_awvalid        (aximl_awvalid),
    .S_AXI_awaddr         (aximl_awaddr),
    .S_AXI_awready        (aximl_awready),

    .S_AXI_wvalid         (aximl_wvalid),
    .S_AXI_wready         (aximl_wready),
    .S_AXI_wdata          (aximl_wdata),

    .S_AXI_bvalid         (aximl_bvalid),
    .S_AXI_bready         (aximl_bready),
    .S_AXI_bresp          (aximl_bresp),

    .S_AXI_arvalid        (aximl_arvalid),
    .S_AXI_arready        (aximl_arready),
    .S_AXI_araddr         (aximl_araddr),

    .S_AXI_rvalid         (aximl_rvalid),
    .S_AXI_rready         (aximl_rready),
    .S_AXI_rresp          (aximl_rresp),
    .S_AXI_rdata          (aximl_rdata),

    // Input AXI Stream
    .SRC_AXIS_clk       (axis_clk),
    .SRC_AXIS_rst       (r_axis_rst),

`ifdef AXIS_IN_TUSER_EN
    .SRC_AXIS_tuser  (axis_in_tuser),
`else
    .SRC_AXIS_tuser  (1'b0),
`endif
    .SRC_AXIS_tvalid (axis_in_tvalid),
    .SRC_AXIS_tready (axis_in_tready),
    .SRC_AXIS_tlast  (axis_in_tlast),
    .SRC_AXIS_tdata  (axis_in_tdata),

    // Output AXI Stream
    .SINK_AXIS_clk   (axis_clk),
    .SINK_AXIS_rst   (r_axis_rst),
`ifdef AXIS_IN_TUSER_EN
    .SINK_AXIS_tuser (axis_out_tuser),
`else
    .SINK_AXIS_tuser (1'b0),
`endif
    .SINK_AXIS_tvalid(axis_out_tvalid),
    .SINK_AXIS_tready(axis_out_tready),
    .SINK_AXIS_tlast (axis_out_tlast),
    .SINK_AXIS_tdata (axis_out_tdata)
);

endmodule
