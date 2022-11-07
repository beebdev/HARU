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
    parameter ptrWid    = 18,
    parameter depth     = 2**ptrWid,
    parameter initalize = 0
)(
    input   wire                clk,

    input   wire                wen,
    input   wire [ptrWid-1:0]   addr,
    input   wire [width-1:0]    din,
    output  reg  [width-1:0]    dout
);

/* ===============================
 * registes/wires
 * =============================== */
(* ram_style = "block" *) 
reg [width-1:0] MEM [0:depth-1];
// reg [depth-1:0] i; // only used for simulation, comment out for synthesis
/* ===============================
 * initialisation
 * =============================== */
//initial begin
//    if (initalize) begin
//        $readmemb("data/reference.txt", MEM);
//    // end else begin
//    //     // only used for simulation, comment out for synthesis
//    //     for (i=0; i<depth; i=i+1) begin
//    //         MEM[i] = 0;
//    //     end
//    end
//end

/* ===============================
 * synchronous logic
 * =============================== */
// Write
always @(posedge clk) begin
	if (wen) begin
		MEM[addr] <= din;
	end
end

// Read
always @(posedge clk) begin
    dout <= MEM[addr];
end

endmodule