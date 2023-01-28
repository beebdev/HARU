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

`timescale 1ns / 1ps

module dtw_core_datapath #(
    parameter WORD_LEN = 16,
    parameter SQG_LEN  = 250
)(
    input   wire                clk,
    input   wire                rst,
    input   wire                running,        // Run enable

    input   wire                load_squiggle,  // Load squiggle
    input   wire [WORD_LEN-1:0] squiggle_word,  // Squiggle sample
    input   wire [WORD_LEN-1:0] reference_word, // Reference sample
    input   wire [31:0]         reference_len,  // Reference length
    output  wire [WORD_LEN-1:0] best_score,     // Minimum value
    output  wire [31:0]         best_position,  // best_position of minimum value
    output  wire                done            // Query search done

);

/* ===============================
 * registers/wires
 * =============================== */
integer k;

reg     [31:0]              cycle_counter;
reg     [7:0]               sqb_address;    // Squiggle buffer address

/* Buffer for squiggles and reference */
reg     [WORD_LEN-1:0]      squiggle_buffer     [1:SQG_LEN];
reg     [WORD_LEN-1:0]      ref_word_buffer;

wire    [WORD_LEN-1:0]      scores              [1:SQG_LEN]; // Stores the scores of each PE
wire    [WORD_LEN-1:0]      reference_buffer    [1:SQG_LEN]; // Stores the reference values of each PE (shifted by 1)

reg     [WORD_LEN-1:0]      l1_score            [1:SQG_LEN]; // Dependency for PE chain
reg     [WORD_LEN-1:0]      l2_score            [1:SQG_LEN]; // Dependency for PE chain
// reg     [0:SQG_LEN+1]       running_d; // Status of each PE (1 bit per PE)
reg     [1:SQG_LEN+1]         running_d; // Status of each PE (1 bit per PE)

reg     [WORD_LEN-1:0]      curr_best_score;        // Current best score
reg     [31:0]              curr_best_position;     // Current best position
reg     [WORD_LEN-1:0]      dtw_score;              // Last row's score; candidate for best score

/* ===============================
 * PE Chain
 * =============================== */
/* First PE */
dtw_core_pe #(
    .WORD_LEN(WORD_LEN)
) inst_dtw_core_pe_001 (
    .clk    (clk),
    .rst    (rst),
    .running (running_d[1]),
    
    .x      (squiggle_buffer[001]),
    .y      (ref_word_buffer),
    .W      (l1_score[001]),
    .N      (16'd0),
    .NW     (16'd0),
    .score  (scores[001]),
    .yp     (reference_buffer[001])
);

/* Rest of PE chain */
genvar m;
generate
for (m = 2; m <= SQG_LEN; m = m + 1) begin
	dtw_core_pe #(
        .WORD_LEN(WORD_LEN)
    ) inst_dtw_core_pe_n (
        .clk    (clk),
        .rst    (rst),
        .running (running_d[m]),

        .x      (squiggle_buffer[m]),
        .y      (reference_buffer[m-1]),
        .W      (l1_score[m]),
        .N      (l1_score[m-1]),
        .NW     (l2_score[m-1]),
        .score   (scores[m]),
        .yp     (reference_buffer[m])
    );
end
endgenerate

/* ===============================
 * asynchronous logic
 * =============================== */
assign best_score       = curr_best_score;
assign best_position    = curr_best_position;
assign done             = (cycle_counter >= reference_len);

/* ===============================
 * synchronous logic
 * =============================== */

/**
 * When dtw_core's running is asserted, we assert the running_d of bit 0 and
 * shift the running_d to the right. This inserts a 1 (run enable) at the first
 * PE.
 * On reset, we set all running_d to 0.
 */
always @(posedge clk) begin
    if(rst) begin
        for (k = 0; k <= SQG_LEN + 1; k = k + 1) begin
            running_d[k] <= 0;
        end
    end else if (running) begin
        // running_d[0] <= 1;
        // for (k = 1; k <= SQG_LEN + 1; k = k + 1) begin
        //     running_d[k] <= running_d[k-1];
        // end
        for (k = SQG_LEN + 1; k > 1; k = k - 1) begin
            running_d[k] <= running_d[k-1];
        end
        running_d[1] <= 1;
    end
end

/** 
 * Update l1_score and l2_score:
 * On each clock cycle when the global dtw_core running is asserted, we update
 * the l1_score and l2_score. The l1_score receives the current score from the
 * PE chain while l2_score receives the l1_score from the previous clock cycle.
 */
always @(posedge clk) begin
    if (rst) begin
        for(k = 1; k <= SQG_LEN; k = k + 1) begin
            l1_score [k] <= -1;
            l2_score[k] <= -1;
        end
    end else if (running) begin
        for(k = 1; k <= SQG_LEN; k = k + 1) begin
            if(running_d[k+1]) begin
                l1_score[k] <= scores[k];
            end
            if(running_d[k+2]) begin
                l2_score[k] <= l1_score[k];
            end
        end
    end
end


/**  Load squiggle sample value
 * On each clock cycle if the global dtw_core running is asserted, when
 * the first PE will be running in the next clock cycle, we load the squiggle
 * sample value into the squiggle buffer. As the address to load into is 
 * managed by sqb_address, once squiggle buffer is fully loaded the address
 * won't be incremented anymore and the squiggle buffer's extra cell will
 * contibue being loaded with the last squiggle_word.
 */
always @(posedge clk) begin
    if (rst) begin
        for(k = 1; k <= SQG_LEN; k = k + 1) begin
            squiggle_buffer[k] <= 0;
        end
    end else if (load_squiggle) begin
        squiggle_buffer[sqb_address] <= squiggle_word;
    end
end

always @(posedge clk) begin
    if (rst) begin
        sqb_address <= 1;
    end else if (load_squiggle) begin
        if(sqb_address <= SQG_LEN) begin
            sqb_address <= sqb_address + 1;
        end
    end
end

/** reference sample loading
 * Load the reference word into a temporary buffer.
 */
always @(posedge clk) begin
    if (rst) begin
        ref_word_buffer <= 0;
    end else if (running) begin
        if(running_d[1]) begin
            ref_word_buffer <= reference_word;
        end
    end
end

/**
 * This cycle counter only increments when the gloabl dtw_core running is
 * asserted and the last PE is running in the next clock cycle.
 * This is used to determine if the search is done.
 */
always @(posedge clk) begin
    if (rst) begin
        cycle_counter <=  0;
    end else if (running) begin
        if(running_d[SQG_LEN]) begin
            cycle_counter <= cycle_counter + 1;
        end
    end
end

/**
 * Updates the dtw_score with the score of the last PE.
 */
always @(posedge clk) begin
    if (rst) begin
        dtw_score <= -1;
    end else if (running) begin
        if(running_d[SQG_LEN+1]) begin
            dtw_score <= scores[SQG_LEN];
        end
    end
end

/**
 * Compare current best with the current dtw_score and update the best score
 * and best position if the current dtw_score is better.
 */
always @(posedge clk) begin
    if (rst) begin
        curr_best_score <= -1;
        curr_best_position <= 0;
    end else if (dtw_score < curr_best_score) begin
        curr_best_score <= dtw_score;
        curr_best_position <= cycle_counter - 32'h2;
    end
end

endmodule