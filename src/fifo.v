`timescale 1ns / 1ps

module fifo #(
    parameter WIDTH = 4,
    parameter DEPTH = 4
)(
    input  wire                 clk,
    input  wire                 rst,

    input  wire                 i_fifo_w_stb,
    input  wire [WIDTH-1:0]     i_fifo_w_data,
    output wire                 o_fifo_full,
    output wire                 o_fifo_not_full,

    input  wire                 i_fifo_r_stb,
    output wire [WIDTH-1:0]     o_fifo_r_data,
    output wire                 o_fifo_empty,
    output wire                 o_fifo_not_empty
);

/* ===============================
 * registers/wires
 * =============================== */
// FIFO
reg [WIDTH-1:0] MEM [0:DEPTH-1];

// FIFO pointers
reg [$clog2(DEPTH)-1:0] write_ptr;
reg [$clog2(DEPTH)-1:0] read_ptr;
reg [$clog2(DEPTH)-1:0] r_read_ptr;

/* ===============================
 * asynchronous logic
 * =============================== */
assign o_fifo_empty     = (write_ptr == read_ptr);
assign o_fifo_full      = (write_ptr == r_read_ptr);
assign o_fifo_not_empty = ~o_fifo_empty;
assign o_fifo_not_full  = ~o_fifo_full;
assign o_fifo_r_data = MEM[read_ptr];

/* ===============================
 * initialization
 * =============================== */
initial begin
    write_ptr   = 0;
    read_ptr    = 0;
    r_read_ptr  = (DEPTH - 1);

    if ( WIDTH <= 0 ) begin
        $error("%m ** Illegal condition **, you used %d WIDTH", WIDTH);
    end

    if ( DEPTH <= 0) begin
        $error("%m ** Illegal condition **, you used %d DEPTH", DEPTH);
    end
end

/* ===============================
 * synchronous logic
 * =============================== */
// Write data
always @ (posedge clk) begin
    if (rst) begin
    end else begin
        if (i_fifo_w_stb && o_fifo_not_full) begin
            MEM[write_ptr] <= i_fifo_w_data;
        end
    end
end

always @ ( posedge clk ) begin
    if (rst) begin
        write_ptr       <=  0;
        read_ptr        <=  0;
        r_read_ptr      <=  (DEPTH - 1);
    end else begin
        // Write pointer
        if (i_fifo_w_stb && o_fifo_not_full) begin
            write_ptr   <= write_ptr + 1;
        end

        // Read pointer
        if (i_fifo_r_stb && o_fifo_not_empty) begin
            r_read_ptr  <= read_ptr;
            read_ptr    <= read_ptr + 1;
        end
    end
end

endmodule
