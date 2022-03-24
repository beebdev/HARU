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
 * local parameters
 * =============================== */
 
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
    .i_axi_clk        (clk),
    .i_axi_rst        (r_rst),

    .i_awvalid        (aximl_awvalid),
    .i_awaddr         (aximl_awaddr),
    .o_awready        (aximl_awready),

    .i_wvalid         (aximl_wvalid),
    .o_wready         (aximl_wready),
    .i_wdata          (aximl_wdata),

    .o_bvalid         (aximl_bvalid),
    .i_bready         (aximl_bready),
    .o_bresp          (aximl_bresp),

    .i_arvalid        (aximl_arvalid),
    .o_arready        (aximl_arready),
    .i_araddr         (aximl_araddr),

    .o_rvalid         (aximl_rvalid),
    .i_rready         (aximl_rready),
    .o_rresp          (aximl_rresp),
    .o_rdata          (aximl_rdata),

    //Input AXI Stream
    .i_axis_clk       (axis_clk),
    .i_axis_rst       (r_axis_rst),

`ifdef AXIS_IN_TUSER_EN
    .i_axis_in_tuser  (axis_in_tuser),
`else
    .i_axis_in_tuser  (1'b0),
`endif
    .i_axis_in_tvalid (axis_in_tvalid),
    .o_axis_in_tready (axis_in_tready),
    .i_axis_in_tlast  (axis_in_tlast),
    .i_axis_in_tdata  (axis_in_tdata),

`ifdef AXIS_IN_TUSER_EN
    .o_axis_out_tuser (axis_out_tuser),
`else
    .i_axis_out_tuser (1'b0),
`endif
    .o_axis_out_tvalid(axis_out_tvalid),
    .i_axis_out_tready(axis_out_tready),
    .o_axis_out_tlast (axis_out_tlast),
    .o_axis_out_tdata (axis_out_tdata)
);

/* ===============================
 * asynchronus logic
 * =============================== */

/* ===============================
 * synchronous logic
 * =============================== */

endmodule
