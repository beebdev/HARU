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
    parameter SQG_LEN       = 250        // Squiggle size
)(
    // Main DTW signals
    input   wire                    clk,
    input   wire                    rst,
    input   wire                    rs,                 // run/stop signal
    output  reg                     running,            // Idle: 0, running: 1

    // Src FIFO signals
    output  reg                     src_fifo_clear,     // Src FIFO Clear signal
    output  reg                     src_fifo_rden,      // Src FIFO Read enable
    input   wire                    src_fifo_empty,     // Src FIFO Empty
    input   wire [31:0]             src_fifo_data,      // Src FIFO Data

    // DTW IO
    input   wire [31:0]             reference_len,
    output  reg  [WORD_LEN-1:0]     best_score,
    output  reg  [31:0]             best_position,
    output  reg                     done
);

reg [31:0] n_loaded_query;

/* ===============================
 * dtw_core signals
 * =============================== */
reg                 dtw_core_rst;
reg                 dtw_core_running;

reg                 dtw_core_load_squiggle;
reg  [WORD_LEN-1:0] dtw_core_squiggle_word;
reg  [WORD_LEN-1:0] dtw_core_reference_word;
wire [31:0]         dtw_core_reference_len;
wire [WORD_LEN-1:0] dtw_core_best_score; // TODO: is this the right width?
wire [31:0]         dtw_core_best_position;
wire                dtw_core_done;

assign dtw_core_reference_len = reference_len;

/* ===============================
 * dtw_core datapath
 * =============================== */
dtw_core_datapath #(
    .WORD_LEN       (WORD_LEN),
    .SQG_LEN        (SQG_LEN)
) inst_dtw_core_datapath (
    .clk            (clk),
    .rst            (dtw_core_rst),
    .running        (dtw_core_running),

    .load_squiggle  (dtw_core_load_squiggle),
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
localparam [1:0]
    state_IDLE = 0,
    state_LOAD_QUERY = 1,
    state_DTW_RUN = 2,
    state_DTW_POST = 3;

reg [1:0] r_state;

always @(posedge clk) begin
    if (rst) begin
        r_state <= state_IDLE;
    end else begin
        case (r_state)
        state_IDLE : begin
        /* This state idles and waits for the rs signal to be asserted
         * to start the search. Once the rs signal is asserted, the
         * state machine will load the query and start the search.
         */

            // Once this transition happens, the rs signal provider is
            // responsible for deasserting the rs signal so that when
            // the search is done and the state is back to IDLE the FSM
            // does not start again directly (gives the user a chance to
            // receive the results)
            if (rs) begin
                r_state <= state_LOAD_QUERY;
            end else begin
                r_state <= state_IDLE;
            end
        end

        state_LOAD_QUERY : begin
        /* This state loads the query into the DTW core. The query samples
         * are provided by the AXIS source FIFO. At the moment we assume there
         * is a fixed 250 samples in the query. Once the query is loaded, the
         * state machine will start the search. After the query samples the
         * rest of the stream is expected to be reference samples.
         */
            if (n_loaded_query == SQG_LEN) begin // TODO: move n_loaded_query to datapath
                r_state <= state_DTW_RUN;
            end else begin
                r_state <= state_LOAD_QUERY;
            end
        end

        state_DTW_RUN : begin
        /* This state runs the DTW core. The reference samples are provided
         * by the AXIS source FIFO. The results include the best score and
         * the best position. When the done signal is asserted it indicates
         * the DTW search is complete. The state machine will then transition
         * to the post state.
         */
            if (dtw_core_done) begin
                r_state <= state_DTW_POST;
            end else begin
                r_state <= state_DTW_RUN;
            end
        end

        state_DTW_POST : begin
            // Write results to output registers
            // Note: As the rs is expected to be deasserted once the IDLE state
            // transitions to the LOAD_QUERY state, the best_score, best_position,
            // and done signal will be kept until the rs signal is asserted again.
            // This allows the user to read the results before the state machine
            // starts again.
            r_state <= state_IDLE;
        end

        default : begin  // Fault Recovery
            r_state <= state_IDLE;
        end
        endcase
    end
end

always @(posedge clk) begin
    case (r_state)
    state_IDLE : begin
        dtw_core_rst <= 1;              // Reset the DTW core
        dtw_core_running <= 0;          // Stop the DTW core
        dtw_core_load_squiggle <= 0;    // Don't load the query
        dtw_core_squiggle_word <= 0;    // Clear the query word
        dtw_core_reference_word <= 0;   // Clear the reference word
        n_loaded_query <= 0;            // Reset the query counter
        src_fifo_rden <= 0;             // Don't read from the source FIFO
        src_fifo_clear <= 0;            // Don't clear the source FIFO
        running <= 0;                   // Indicate the search is not running
    end

    state_LOAD_QUERY : begin
        dtw_core_rst <= 0;              // Deassert the DTW core reset
        dtw_core_running <= 0;          // DTW core should not be running
        dtw_core_squiggle_word <= src_fifo_data;    // Load fifo data to squiggle word
        dtw_core_reference_word <= 0;   // Clear the reference word
        src_fifo_clear <= 0;
        n_loaded_query <= n_loaded_query;
        dtw_core_load_squiggle <= 0;
        src_fifo_rden <= 0;
        
        if (!src_fifo_empty) begin
            n_loaded_query <= n_loaded_query + 1;
            dtw_core_load_squiggle <= 1;    // Load the query
            src_fifo_rden <= 1;
        end
              
        running <= 1;
        best_score <= 0;
        best_position <= 0;
        done <= 0;
    end

    state_DTW_RUN : begin
        dtw_core_rst <= 0;
        dtw_core_load_squiggle <= 0;
        dtw_core_squiggle_word <= 0;
        dtw_core_reference_word <= src_fifo_data;
        n_loaded_query <= 0;
        src_fifo_clear <= 0;

        if (!src_fifo_empty) begin
            src_fifo_rden <= 1;
            dtw_core_running <= 1;
        end else begin
            src_fifo_rden <= 0;
            dtw_core_running <= 0;
        end
        running <= 1;
        best_score <= 0;
        best_position <= 0;
        done <= 0;
    end

    state_DTW_POST : begin
        dtw_core_rst <= 0;
        dtw_core_running <= 0;
        dtw_core_load_squiggle <= 0;
        dtw_core_squiggle_word <= 0;
        dtw_core_reference_word <= 0;
        n_loaded_query <= 0;
        src_fifo_rden <= 0;
        src_fifo_clear <= 1;
        
        running <= 1;
        best_score <= dtw_core_best_score;
        best_position <= dtw_core_best_position;
        done <= dtw_core_done;
    end
    endcase
end

endmodule