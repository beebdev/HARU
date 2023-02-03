
`timescale 1ns/1ps

module tb_zynq_dtw #(
    parameter ADDR_WIDTH          = 32,
    parameter DATA_WIDTH          = 32,
    parameter AXIS_DATA_WIDTH     = 32,
    parameter STROBE_WIDTH        = (DATA_WIDTH / 8)
) (
    input                               S_AXI_clk,
    input                               S_AXI_rst,

    //Write Address Channel
    input                               S_AXI_awvalid,
    input       [ADDR_WIDTH - 1: 0]     S_AXI_awaddr,
    output                              S_AXI_awready,

    //Write Data Channel
    input                               S_AXI_wvalid,
    output                              S_AXI_wready,
    input       [STROBE_WIDTH - 1:0]    S_AXI_wstrb,
    input       [DATA_WIDTH - 1: 0]     S_AXI_wdata,

    //Write Response Channel
    output                              S_AXI_bvalid,
    input                               S_AXI_bready,
    output      [1:0]                   S_AXI_bresp,

    //Read Address Channel
    input                               S_AXI_arvalid,
    output                              S_AXI_arready,
    input       [ADDR_WIDTH - 1: 0]     S_AXI_araddr,

    //Read Data Channel
    output                              S_AXI_rvalid,
    input                               S_AXI_rready,
    output      [1:0]                   S_AXI_rresp,
    output      [DATA_WIDTH - 1: 0]     S_AXI_rdata,

    //AXIS IN Stream
    input                               S_AXIS_clk,
    input                               S_AXIS_rst,

    input                               S_AXIS_tuser,
    input                               S_AXIS_tvalid,
    output                              S_AXIS_tready,
    input                               S_AXIS_tlast,
    input       [AXIS_DATA_WIDTH - 1:0] S_AXIS_tdata
);


/* ===============================
 * Registers
 * =============================== */
reg               r_rst;
reg               r_axis_rst;
reg [7:0] 	      test_id       = 0;

// Workaround for weird icarus simulator bug
always @ (*)      r_rst         = S_AXI_rst;
always @ (*)      r_axis_rst    = S_AXIS_rst;

zynq_dtw #(
    .S_AXI_ADDR_WIDTH(ADDR_WIDTH),
    .S_AXI_DATA_WIDTH(DATA_WIDTH),
    .S_AXI_RST_INVERT(0),
    .S_AXIS_DATA_WIDTH(AXIS_DATA_WIDTH),
    .S_AXIS_RST_INVERT(0)
    // .SRC_FIFO_DEPTH    = 4
) dut (
    .S_AXI_clk(S_AXI_clk),
    .S_AXI_rst(r_rst),

    // Write Address Channel
    .S_AXI_awvalid(S_AXI_awvalid),
    .S_AXI_awaddr(S_AXI_awaddr),
    .S_AXI_awready(S_AXI_awready),

    // Write Data Channel
    .S_AXI_wvalid(S_AXI_wvalid),
    .S_AXI_wready(S_AXI_wready),
    .S_AXI_wdata(S_AXI_wdata),

    // Write Response Channel
    .S_AXI_bvalid(S_AXI_bvalid),
    .S_AXI_bready(S_AXI_bready),
    .S_AXI_bresp(S_AXI_bresp),

    // Read Address Channel
    .S_AXI_arvalid(S_AXI_arvalid),
    .S_AXI_arready(S_AXI_arready),
    .S_AXI_araddr(S_AXI_araddr),

    // Read Data Channel
    .S_AXI_rvalid(S_AXI_rvalid),
    .S_AXI_rready(S_AXI_rready),
    .S_AXI_rresp(S_AXI_rresp),
    .S_AXI_rdata(S_AXI_rdata),

    // AXIS IN Stream
    .S_AXIS_clk(S_AXIS_clk),
    .S_AXIS_rst(r_axis_rst),
    .S_AXIS_tuser(S_AXIS_tuser),
    .S_AXIS_tvalid(S_AXIS_tvalid),
    .S_AXIS_tready(S_AXIS_tready),
    .S_AXIS_tlast(S_AXIS_tlast),
    .S_AXIS_tdata(S_AXIS_tdata)
);

endmodule