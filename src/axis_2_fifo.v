`timescale 1ps / 1ps

module axis_2_fifo_adapter #(
    parameter AXIS_DATA_WIDTH = 32,
    parameter FIFO_DATA_WIDTH = AXIS_DATA_WIDTH
)(
    input  wire                             i_axis_tuser,
    input  wire                             i_axis_tvalid,
    output wire                             o_axis_tready,
    input  wire                             i_axis_tlast,
    input  wire [AXIS_DATA_WIDTH - 1:0]     i_axis_tdata,

    output wire [FIFO_DATA_WIDTH - 1:0]     o_fifo_data,
    output wire                             o_fifo_w_stb,
    input  wire                             i_fifo_not_full
);

/* ===============================
 * asynchronous logic
 * =============================== */
assign o_fifo_data      = i_axis_tdata;
assign o_axis_tready    = i_fifo_not_full;
assign o_fifo_w_stb     = i_axis_tvalid & o_axis_tready & i_fifo_not_full;

endmodule