`timescale 1ps / 1ps

module fifo_2_axis_adapter #(
    parameter AXIS_DATA_WIDTH     = 32,
    parameter FIFO_DATA_WIDTH     = AXIS_DATA_WIDTH
)(
    input  wire [FIFO_DATA_WIDTH - 1: 0]    i_fifo_data,
    output wire                             o_fifo_r_stb,
    input  wire                             i_fifo_not_empty,
    input  wire                             i_fifo_last,

    output wire                             o_axis_tuser,
    output wire [AXIS_DATA_WIDTH - 1: 0]    o_axis_tdata,
    output wire                             o_axis_tvalid,
    input  wire                             i_axis_tready,
    output wire                             o_axis_tlast
);

/* ===============================
 * asynchronous logic
 * =============================== */
assign o_axis_tuser     = 0;
assign o_axis_tlast     = i_fifo_last;
assign o_axis_tdata     = i_fifo_data;
assign o_axis_tvalid    = i_fifo_not_empty;
assign o_fifo_r_stb     = i_axis_tready & o_axis_tvalid;

endmodule
