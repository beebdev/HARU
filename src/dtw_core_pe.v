`timescale 1ns / 1ps 

module dtw_core_pe #(
    parameter width = 18
)(
    input clk, rst,
    input [width-1:0] x ,y, N, W, NW,
    output [width-1:0] DTWc,
    output reg [width-1:0] yp
);

wire [width-1:0] diff = (x - y);
wire [width-1:0] min2 = (N > W) ? W : N;
wire [width-1:0] min3 = (min2 > NW) ? NW : min2;

assign DTWc = min3 + {{width{diff[width - 1]}} ^ diff} + diff[width - 1];

always@(posedge clk) begin
    if(rst) begin
        yp <= 0;
    end else begin
        yp <= y;
    end
end

endmodule
