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
 * Description: Dual port BRAM for storing reference sequence for the DTW core
 *
 * Changes:     Author         Description
 *  03/31/2022  Elton Shih     Initial Commit
 */

`timescale 1ns / 1ps

module dtw_core_ref_mem #(
    parameter width     = 16,
    parameter ptrWid    = 15,
    parameter depth     = 2**ptrWid,
    parameter initalize = 1
)(
    input   wire                clk,

    // Port A
    input   wire                a_wen,
    input   wire [ptrWid-1:0]   a_addrW,
    input   wire [ptrWid-1:0]   a_addrR,
    input   wire [width-1:0]    a_din,
    output  reg  [width-1:0]    a_dout,

    // Port B
    input   wire                b_wen,
    input   wire [ptrWid-1:0]   b_addrW,
    input   wire [ptrWid-1:0]   b_addrR,
    input   wire [width-1:0]    b_din,
    output  reg  [width-1:0]    b_dout
);

/* ===============================
 * registes/wires
 * =============================== */
(* ram_style = "block" *) 
reg [width-1:0] MEM [0:depth-1];
reg [depth-1:0] i;

/* ===============================
 * initialisation
 * =============================== */
initial begin
    if (initalize) begin
        $readmemb("data/reference.txt", MEM);
    end else begin
        for (i = 0; i < depth; i = i + 1) begin
            MEM[i] = 0;
        end
    end
end

/* ===============================
 * synchronous logic
 * =============================== */
// Write
always @(posedge clk) begin
	if (a_wen) begin
		MEM[a_addrW] <= a_din;
    end else if (b_wen) begin
        MEM[b_addrW] <= b_din;
    end
end

// Read
always @(posedge clk) begin
    a_dout <= MEM[a_addrR];
    b_dout <= MEM[b_addrR];
end

endmodule