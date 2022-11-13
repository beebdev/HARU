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
    parameter width = 16  // data width
)(
    input   wire                clk,
    input   wire                rst,
    input   wire                running,    // PE run enable

    input   wire [width-1:0]    x,          // squiggle sample
    input   wire [width-1:0]    y,          // reference sample
    input   wire [width-1:0]    N,          // north dependancy
    input   wire [width-1:0]    W,          // west dependancy
    input   wire [width-1:0]    NW,         // northwest dependancy
    output  wire [width-1:0]    DTWc,       // DTW cost
    output  reg  [width-1:0]    yp          // previous y sample
);

/* ===============================
 * registes/wires
 * =============================== */
// Sample distances
wire [width-1:0] diff = (x - y);
wire [width-1:0] cost = diff[width - 1] ? -diff : diff;

// min
wire [width-1:0] min2 = (N > W) ? W : N;
wire [width-1:0] min3 = (min2 > NW)? NW : min2;

/* ===============================
 * asynchronous logic
 * =============================== */
assign DTWc = cost + min3;

/* ===============================
 * synchronous logic
 * =============================== */
always@(posedge clk) begin
    if(rst) begin
        yp <= 0;
    end else if (running) begin
        yp <= y;
    end
end

endmodule

