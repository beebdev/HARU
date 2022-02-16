`timescale 1ns / 1ps 

module dgn_processingelement #(
    parameter width = 18
) (
    input clk, rst,
    input [width-1:0] x ,y, N, W, NW,
    output [width-1:0] DTWc,
    output reg [width-1:0] yp
);

wire [width-1:0] diff = (x - y);                                  // width LUT
//wire [width-1:0] cost = diff[width-1]? -diff: diff;   //(abs diff)    // width/2 + ( width) LUT  

wire [width-1:0] min2 = (N > W) ? W : N   ;           // width LUT
wire [width-1:0] min3 = (min2 > NW) ? NW : min2;           // width LUT

assign DTWc = min3 + {{width{diff[width - 1]}} ^ diff} + diff[width - 1];                            // width

always@(posedge clk)
if(rst)
    yp <= 0;
else 
    yp <= y;

endmodule
