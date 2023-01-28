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

module dtw_core_pe #(
    parameter WORD_LEN = 16  // data WORD_LEN
)(
    input   wire                clk,
    input   wire                rst,
    input   wire                running,    // PE run enable

    input   wire [WORD_LEN-1:0]    x,          // squiggle sample
    input   wire [WORD_LEN-1:0]    y,          // reference sample
    input   wire [WORD_LEN-1:0]    N,          // north dependancy
    input   wire [WORD_LEN-1:0]    W,          // west dependancy
    input   wire [WORD_LEN-1:0]    NW,         // northwest dependancy
    output  wire [WORD_LEN-1:0]    score,      // DTW cost
    output  reg  [WORD_LEN-1:0]    yp          // previous y sample
);

/* Raw sample distance */
wire [WORD_LEN-1:0] diff = (x - y);
wire [WORD_LEN-1:0] cost = diff[WORD_LEN - 1] ? -diff : diff;

/* Find minimum value of dependancies */
wire [WORD_LEN-1:0] min2 = (N > W) ? W : N;
wire [WORD_LEN-1:0] min3 = (min2 > NW)? NW : min2;

/* DTW score */
assign score = cost + min3;

/* Pass on the previous y sample */
always@(posedge clk) begin
    if(rst) begin
        yp <= 0;
    end else if (running) begin
        yp <= y;
    end
end

endmodule

