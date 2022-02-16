`timescale 1ns / 1ps 

module dgn_processingelement #(
    parameter width = 16  // data width
) (
    input clk, rst,
    input [width-1:0] x ,y, N, W, NW,
    output [width-1:0] DTWc,
    output reg [width-1:0] yp
);

wire [width-1:0] diff = (x - y);                        // distance of two sequences
wire [width-1:0] cost = diff[width - 1] ? -diff : diff; // absolute value of distance

wire [width-1:0] min2 = (N > W) ? W : N;                // minimum of two
wire [width-1:0] min3 = (min2 > NW)? NW : min2;         // minimam of three

assign DTWc = cost + min3;                              // PE cell accumulated cost

always @(posedge clk) begin
    if (rst) begin
        yp <= 0;
    end else begin
        yp <= y;    // pass on previous y
    end
end

endmodule
