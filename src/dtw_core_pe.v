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
    output  reg [width-1:0]     yp          // previous y sample
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

