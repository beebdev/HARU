`timescale 1 ns / 1 ps

module stim_fifo #(
    parameter WIDTH = 16,
    parameter ptrWid = 15,
    parameter depth = 2**ptrWid,
    parameter MODE
) (
    input  wire  clk,
    input  wire  rst,
    input  wire  wren,
    input  wire  rden,
    output wire  fifo_empty,
    output wire  fifo_full,
    input  wire  [WIDTH-1:0]  data_in,
    output wire  [WIDTH-1:0]  data_out
);

// Params
localparam NUMBER_OF_INPUT_WORDS = 8;

// FIFO implementation
reg [C_S_AXIS_TDATA_WIDTH-1:0] fifo_data [0 : NUMBER_OF_INPUT_WORDS-1];
reg [ptrWid-1:0] write_pointer;
reg [ptrWid-1:0] read_pointer;
reg [ptrWid-1:0] data_count;

wire w_wren;
wire w_rden;

assign w_wren = wren & ~fifo_full;
assign w_rden = rden & ~fifo_empty;

genvar i;
initial begin
    if (MODE == 0) begin // reference
        $readmemb("D:/UNSW/Thesis/dtw_core/bram_init/reference.txt", MEM);
    end else if (MODE == 1) begin // query
        $readmemb("D:/UNSW/Thesis/dtw_core/bram_init/query.txt", MEM);
    end else begin
        for (i = 0; i < depth; i = i + 1) begin
            MEM[i] = 0;
        end
    end
end


always @(posedge clk) begin
    if (rst) begin
        write_pointer <= 0;
        read_pointer <= 0;
        data_count <= 0;
    end else begin

        // Data count
        if (w_wren && !w_rden) begin
            data_count <= data_count + 1;
        end else if (!w_wren && w_rden) begin
            data_count <= data_count - 1;
        end

        // Write pointer
        if (w_wren) begin
            fifo_data[write_pointer] <= data_in;
            write_pointer <= write_pointer + 1;
        end

        // Read pointer
        if (w_rden) begin
            data_out <= fifo_data[read_pointer];
            read_pointer <= read_pointer + 1;
        end
    end
end