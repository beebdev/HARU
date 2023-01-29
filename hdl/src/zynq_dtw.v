/*
Distributed under the MIT license.
Copyright (c) 2022 Elton Shih (beebdev@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
 * Author: Elton Shih (beebdev@gmail.com)
 * Description: Core implementation for pipelined subsequence DTW algorithm
 *
 * Changes:     Author         Description
 *  03/31/2022  Elton Shih     Initial Commit
 */

`timescale 1ns / 1ps

/* Version Information */
// version 0.0.2
`define MAJOR_VERSION       0
`define MINOR_VERSION       0
`define REVISION            2

`define MAJOR_RANGE         31:28
`define MINOR_RANGE         27:20
`define REVISION_RANGE      19:16
`define VERSION_PAD_RANGE   15:0

module zynq_dtw #(
    parameter S_AXI_ADDR_WIDTH = 16,
    parameter S_AXI_DATA_WIDTH = 32,
    parameter S_AXI_RST_INVERT = 1,
    parameter S_AXIS_DATA_WIDTH = 32,
    parameter S_AXIS_RST_INVERT = 1,
    parameter SRC_FIFO_DEPTH    = 4
) (
    /* ===============================
     * Control interface (AXI4-Lite Slave)
     * =============================== */
    input wire                              S_AXI_clk,
    input wire                              S_AXI_rst,

    // Write Address Channel
    input  wire                             S_AXI_awvalid,
    input  wire [S_AXI_ADDR_WIDTH - 1: 0]   S_AXI_awaddr,
    output wire                             S_AXI_awready,

    // Write Data Channel
    input  wire                             S_AXI_wvalid,
    output wire                             S_AXI_wready,
    input  wire [S_AXI_DATA_WIDTH - 1: 0]   S_AXI_wdata,

    // Write Response Channel
    output wire                             S_AXI_bvalid,
    input  wire                             S_AXI_bready,
    output wire [1:0]                       S_AXI_bresp,
    
    // Read Address Channel
    input  wire                             S_AXI_arvalid,
    output wire                             S_AXI_arready,
    input  wire [S_AXI_ADDR_WIDTH - 1: 0]   S_AXI_araddr,
    
    // Read Data Channel
    output wire                             S_AXI_rvalid,
    input  wire                             S_AXI_rready,
    output wire [1:0]                       S_AXI_rresp,
    output wire [S_AXI_DATA_WIDTH - 1: 0]   S_AXI_rdata,

    /* ===============================
     * Data-in interface (AXI4-Stream Master)
     * =============================== */
    input  wire                             S_AXIS_clk,
    input  wire                             S_AXIS_rst,
    input  wire                             S_AXIS_tuser,
    input  wire                             S_AXIS_tvalid,
    output wire                             S_AXIS_tready,
    input  wire                             S_AXIS_tlast,
    input  wire [S_AXIS_DATA_WIDTH - 1:0]   S_AXIS_tdata
);

/* ===============================
 * Control interface (AXI4-Lite Slave)
 * =============================== */

/* Register map */
localparam REG_0_CONTROL = 0;
localparam REG_1_STATUS = 1;
localparam REG_2_VERSION = 2;
localparam REG_3_KEY = 3;
localparam REG_4_REF_LEN = 4;
localparam REG_5_BEST_SCORE = 5;
localparam REG_6_BEST_POSITION = 6;
// TODO: add a query count

reg [S_AXI_DATA_WIDTH - 1: 0]   reg_0_control;
reg [S_AXI_DATA_WIDTH - 1: 0]   reg_1_status;
reg [S_AXI_DATA_WIDTH - 1: 0]   reg_2_version;
reg [S_AXI_DATA_WIDTH - 1: 0]   reg_3_key;
reg [S_AXI_DATA_WIDTH - 1: 0]   reg_4_ref_len;
reg [S_AXI_DATA_WIDTH - 1: 0]   reg_5_best_score;
reg [S_AXI_DATA_WIDTH - 1: 0]   reg_6_best_position;

localparam REG_0_CONTROL_RST_BIT = 0;
localparam REG_0_CONTROL_RS_BIT = 1;
localparam REG_1_STATUS_RUNNING_BIT = 0;
localparam REG_1_STATUS_DONE_BIT = 1;

/* Address related */
localparam  integer ADDR_LSB = (DATA_WIDTH / 32) + 1;
localparam  integer ADDR_BITS = 3; // 2^3 = 8 registers
// localparam  MAX_ADDR = REG_6_BEST_POSITION;

wire                            s_axi_rst;
wire [S_AXI_ADDR_WIDTH - 1: 0]  s_axi_reg_address;
reg                             s_axi_reg_invalid_addr;
wire                            s_axi_reg_in_rdy;
reg                             s_axi_reg_in_ack_stb;
wire [S_AXI_DATA_WIDTH - 1: 0]  s_axi_reg_in_data;
wire                            s_axi_reg_out_req;
reg                             s_axi_reg_out_rdy_stb;
reg [S_AXI_DATA_WIDTH - 1: 0]   s_axi_reg_out_data;


/* ===============================
 * Data-in interface (AXI4-Stream Master)
 * =============================== */
wire                            s_axis_rst;
wire [S_AXIS_DATA_WIDTH - 1: 0] s_axis_fifo_w_data;
wire                            s_axis_fifo_w_stb;
wire                            s_axis_fifo_full;
wire                            s_axis_fifo_not_full;
wire                            s_axis_fifo_not_empty

/* ===============================
 * DTW core
 * =============================== */
wire                            dtw_core_rst;
wire                            dtw_core_rs;
wire                            dtw_core_running;
wire                            dtw_core_fifo_clear;
wire                            dtw_core_fifo_rden;
wire                            dtw_core_fifo_empty;
wire [S_AXIS_DATA_WIDTH - 1: 0] dtw_core_fifo_data;
wire [S_AXI_DATA_WIDTH - 1: 0]  dtw_core_reference_len;
wire [S_AXI_DATA_WIDTH - 1: 0]  dtw_core_best_score;
wire [S_AXI_DATA_WIDTH - 1: 0]  dtw_core_best_position;
wire                            dtw_core_done;


/* ===============================
 * Modules
 * =============================== */
 
/* Data-in interface and FIFO */
axis_2_fifo_adapter #(
    .AXIS_DATA_WIDTH    (S_AXIS_DATA_WIDTH)
) a2fa (
    .i_axis_tuser       (S_AXIS_tuser),
    .i_axis_tvalid      (S_AXIS_tvalid),
    .o_axis_tready      (S_AXIS_tready),
    .i_axis_tlast       (S_AXIS_tlast),
    .i_axis_tdata       (S_AXIS_tdata),

    .o_fifo_data        (s_axis_fifo_w_data),
    .o_fifo_w_stb       (s_axis_fifo_w_stb),
    .i_fifo_not_full    (s_axis_fifo_not_full)
);

assign s_axis_rst = S_AXIS_RST_INVERT ? ~S_AXIS_rst : S_AXIS_rst;
fifo #(
    .DEPTH              (SRC_FIFO_DEPTH),
    .WIDTH              (S_AXIS_DATA_WIDTH)
) src_fifo (
    .clk                (SRC_AXIS_clk),
    .rst                (s_axis_rst | dtw_core_fifo_clear),

    .i_fifo_w_stb       (s_axis_fifo_w_stb),
    .i_fifo_w_data      (s_axis_fifo_w_data),
    .o_fifo_full        (s_axis_fifo_full), // TODO: used?
    .o_fifo_not_full    (s_axis_fifo_not_full),

    .i_fifo_r_stb       (dtw_core_fifo_rden),
    .o_fifo_r_data      (dtw_core_fifo_data),
    .o_fifo_empty       (dtw_core_fifo_empty),
    .o_fifo_not_empty   (s_axis_fifo_not_empty)
);

/* DTW core */
assign dtw_core_rst = reg_0_control[REG_0_CONTROL_RST_BIT];
assign dtw_core_rs = reg_0_control[REG_0_CONTROL_RS_BIT];
assign dtw_core_reference_len = reg_2_reference_len;
dtw_core #(
    .WORD_LEN (WORD_LEN),
    .SQG_LEN (SQG_LEN)
) inst_dtw_core (
    .clk                (S_AXI_clk),
    .rst                (dtw_core_rst),
    .rs                 (dtw_core_rs),
    .running            (dtw_core_running),

    .src_fifo_clear     (dtw_core_fifo_clear),
    .src_fifo_rden      (dtw_core_fifo_rden),
    .src_fifo_empty     (dtw_core_fifo_empty),
    .src_fifo_data      (dtw_core_fifo_data),

    .reference_len      (dtw_core_reference_len),
    .best_score         (dtw_core_best_score),
    .best_position      (dtw_core_best_position),
    .done               (dtw_core_done)
);

/* AXI4-Lite Slave */
assign s_axi_rst = S_AXI_RST_INVERT ? ~S_AXI_rst : S_AXI_rst;
axi_lite_slave #(
    .ADDR_WIDTH(S_AXI_ADDR_WIDTH),
    .DATA_WIDTH(S_AXI_DATA_WIDTH)
) reg_interface (
    .clk                (S_AXI_clk),
    .rst                (s_axi_rst),

    .i_awvalid          (S_AXI_awvalid),
    .i_awaddr           (S_AXI_awaddr),
    .o_awready          (S_AXI_awready),

    .i_wvalid           (S_AXI_wvalid),
    .o_wready           (S_AXI_wready),
    .i_wdata            (S_AXI_wdata),

    .o_bvalid           (S_AXI_bvalid),
    .i_bready           (S_AXI_bready),
    .o_bresp            (S_AXI_bresp),

    .i_arvalid          (S_AXI_arvalid),
    .o_arready          (S_AXI_arready),
    .i_araddr           (S_AXI_araddr),

    .o_rvalid           (S_AXI_rvalid),
    .i_rready           (S_AXI_rready),
    .o_rresp            (S_AXI_rresp),
    .o_rdata            (S_AXI_rdata),

    // Register Interface
    .o_reg_address      (s_axi_reg_address),        // Handled below
    .i_reg_invalid_addr (s_axi_reg_invalid_addr),   // Handled below

    // From Master
    .o_reg_in_rdy       (s_axi_reg_in_rdy),         // Handled below
    .i_reg_in_ack_stb   (s_axi_reg_in_ack_stb),     // Handled below
    .o_reg_in_data      (s_axi_reg_in_data),        // Handled below

    // To Master
    .o_reg_out_req      (s_axi_reg_out_req),        // Handled below
    .i_reg_out_rdy_stb  (s_axi_reg_out_rdy_stb),    // Handled below
    .i_reg_out_data     (s_axi_reg_out_data)        // Handled below
);

/* Register Interface */
// TODO: check if read only register assignments are correct

// reg_0_control is written by master
assign reg_1_status = {30'h0, dtw_core_done, dtw_core_running};
assign reg_2_version[MAJOR_RANGE] = MAJOR_VERSION;
assign reg_2_version[MINOR_RANGE] = MINOR_VERSION;
assign reg_2_version[REVISION_RANGE] = REVISION;
assign reg_2_version[VERSION_PAD_RANGE] = 0;
assign reg_3_key = 32'h0ca7cafe;
// reg_4_ref_len is written by master
assign reg_5_best_score = dtw_core_best_score;
assign reg_6_best_position = dtw_core_best_position;

always @ (posedge S_AXI_clk) begin
    s_axi_reg_in_ack_stb    <=  0;
    s_axi_reg_out_rdy_stb   <=  0;
    s_axi_reg_invalid_addr  <=  0;

    if (s_axi_rst) begin
        s_axi_reg_out_data  <=  0;

        // Reset registers
        reg_0_control         <=  0;
        reg_4_ref_len         <=  0;

    end else begin
        if (s_axi_reg_in_rdy) begin
            // M_AXI to here
            case (s_axi_reg_address[ADDR_LSB + ADDR_BITS:ADDR_LSB])
            REG_0_CONTROL: begin
                reg_0_control <= s_axi_reg_in_data;
            end
            REG_1_STATUS: begin
                // Read only, we don't write to it here
            end
            REG_2_VERSION: begin
                // Read only, we don't write to it here
            end
            REG_3_KEY: begin
                // Read only, we don't write to it here
            end
            REG_4_REF_LEN: begin
                reg_4_ref_len <= s_axi_reg_in_data;
            end
            REG_5_BEST_SCORE: begin
                // Read only, we don't write to it here
            end
            REG_6_BEST_POSITION: begin
                // Read only, we don't write to it here
            end
            default: begin // unknown address
                s_axi_reg_invalid_addr <= 1;
            end
            endcase
            // Tell AXI slave we are done with the data
            s_axi_reg_in_ack_stb <= 1;
        
        end else if (s_axi_reg_out_req) begin
            // here to M_AXI
            case (s_axi_reg_address[ADDR_LSB + ADDR_BITS:ADDR_LSB])
            REG_0_CONTROL: begin
                s_axi_reg_out_data <= reg_0_control;
            end
            REG_1_STATUS: begin
                s_axi_reg_out_data <= reg_1_status;
            end
            REG_2_VERSION: begin
                s_axi_reg_out_data <= reg_2_version;
            end
            REG_3_KEY: begin
                s_axi_reg_out_data <= reg_3_key;
            end
            REG_4_REF_LEN: begin
                s_axi_reg_out_data <= reg_4_ref_len;
            end
            REG_5_BEST_SCORE: begin
                s_axi_reg_out_data <= reg_5_best_score;
            end
            REG_6_BEST_POSITION: begin
                s_axi_reg_out_data <= reg_6_best_position;
            end
            default: begin // unknown address
                s_axi_reg_invalid_addr <= 1;
            end
            endcase
            // Tell AXI slave to send back this packet
            s_axi_reg_out_rdy_stb <= 1;
        end
    end

end

endmodule
