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

module dtw_core #(
    parameter WORD_LEN      = 16,       // Data width
    parameter AXIS_WIDTH    = 32,       // AXI data width
    parameter SQG_LEN      = 250        // Squiggle size
)(
    // Main DTW signals
    input   wire                    clk,
    input   wire                    rst,
    input   wire                    rs,                 // run/stop signal

    input   wire [31:0]             reference_len,
    output  reg                     running,            // Idle: 0, running: 1

    // Src FIFO signals
    output  wire                    src_fifo_clear,     // Src FIFO Clear signal
    output  reg                     src_fifo_rden,      // Src FIFO Read enable
    input   wire                    src_fifo_empty,     // Src FIFO Empty
    input   wire [31:0]             src_fifo_data,      // Src FIFO Data

    // Sink FIFO signals
    output  reg                     sink_fifo_wren,     // Sink FIFO Write enable
    input   wire                    sink_fifo_full,     // Sink FIFO Full
    output  reg [31:0]              sink_fifo_data,     // Sink FIFO Data
    output  reg                     sink_fifo_last      // Sink FIFO Last
);

reg dtw_core_rst;
reg dtw_core_running;
wire dtw_core_done;

reg                 dtw_load_squiggle;
reg  [WORD_LEN-1:0] dtw_core_squiggle_word;
reg  [WORD_LEN-1:0] dtw_core_reference_word;
wire [WORD_LEN-1:0] dtw_core_best_score; // TODO: is this the right width?
wire [31:0] dtw_core_best_position;

reg [31:0] n_loaded_query;

// constant for delaying FSM transition (3 cycles)
localparam [1:0] SEND_RESULT_DELAY = 2'h3;
reg     [1:0] send_result_timer;

/* ===============================
 * DTW core datapath
 * =============================== */
dtw_core_datapath #(
    .WORD_LEN       (WORD_LEN),
    .SQG_LEN        (SQG_LEN)
) inst_dtw_core_datapath (
    .clk            (clk),
    .rst            (dtw_core_rst),
    .running        (dtw_core_running),

    .squiggle_word  (dtw_core_squiggle_word),
    .reference_word (dtw_core_reference_word),
    .reference_len  (reference_len),
    .best_score     (dtw_core_best_score),
    .best_position  (dtw_core_best_position),
    .done           (dtw_core_done)
);

/* ===============================
 * FSM
 * =============================== */
/* FSM States */
localparam [1:0] // n states
    state_IDLE = 0,
    state_LOAD_QUERY = 1,
    state_DTW_RUN = 2,
    state_DTW_POST = 3;

reg [1:0] r_state;
reg [1:0] r_next_state;

always @(posedge clk, posedge rst) begin
    if (rst) begin
        r_state <= state_IDLE;
    end else begin
        r_state <= r_next_state;
    end
end

/* Mealy FSM */
always @(r_state, rs, src_fifo_empty, n_loaded_query, dtw_core_done, sink_fifo_full) begin
    // Default outputs
    dtw_core_rst <= 0;
    dtw_core_running <= 0;
    dtw_load_squiggle <= 0;
    dtw_core_squiggle_word <= 0;
    dtw_core_reference_word <= 0;
    n_loaded_query <= 0;
    src_fifo_rden <= 0;
    send_result_timer <= send_result_timer;
    sink_fifo_wren <= 0;
    sink_fifo_data <= 0;
    sink_fifo_last <= 0;

    // State transition
    case (r_state)
    state_IDLE: begin
        r_next_state = state_IDLE;
        dtw_core_rst <= 1;
        send_result_timer <= 0;

        if (rs) begin
            r_next_state = state_LOAD_QUERY;
            dtw_core_rst <= 0;
        end
    end
    state_LOAD_QUERY: begin
        r_next_state = state_LOAD_QUERY;
        dtw_core_squiggle_word <= src_fifo_data[WORD_LEN-1:0];

        // If fifo not empty, read data
        if (!src_fifo_empty) begin
            src_fifo_rden <= 1;
            n_loaded_query <= n_loaded_query + 1;
            dtw_load_squiggle <= 1;
        end

        // Transition when loaded all query
        if (n_loaded_query == SQG_LEN) begin
            r_next_state = state_DTW_RUN;
        end
    end
    state_DTW_RUN: begin
        r_next_state = state_DTW_RUN;
        dtw_core_reference_word <= src_fifo_data[WORD_LEN-1:0];

        // If fifo not empty, read data
        if (!src_fifo_empty) begin
            src_fifo_rden <= 1;
            dtw_core_running <= 1;
        end

        // Transition when done
        if (dtw_core_done) begin
            r_next_state = state_DTW_POST;
        end
    end
    state_DTW_POST: begin
        r_next_state = state_DTW_POST;

        // If sink fifo not full, write data
        if (!sink_fifo_full) begin
            // What to send
            case (send_result_timer)
            0: begin
                sink_fifo_wren <= 1;
                sink_fifo_data <= dtw_core_best_position;
            end
            1: begin
                sink_fifo_wren <= 1;
                sink_fifo_data <= {16'b0, dtw_core_best_score};
            end
            2: begin // Done! Transition to idle
                r_next_state <= state_IDLE;
                sink_fifo_wren <= 0; // Don't write
                sink_fifo_last <= 1; // Tell AXIS that this is the last word
                sink_fifo_data <= 0; // No data
            end
            endcase
            send_result_timer <= send_result_timer + 1;
        end
    end
    default: begin
        r_next_state = r_state;
    end
    endcase
end

endmodule