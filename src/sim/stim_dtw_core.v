module stim_code;

// Parameters
parameter TEST = 0;
parameter bitwidth = 16;
parameter SQG_SIZE = 250;
parameter REF_SIZE = 29898;

// Module IO
reg clk = 0;  
reg rst;
reg start;
reg ref_len = REF_SIZE;
reg op_mode;
wire running;

wire src_fifo_rden;
wire src_fifo_empty;
wire [31:0] src_fifo_data;
reg [14:0] src_fifo_addr;

wire sink_fifo_wren;
wire sink_fifo_full;
wire [31:0] sink_fifo_data;

reg started;

/* IO assignment */
assign result_fifo_wren = sink_fifo_wren;
assign result_fifo_full = sink_fifo_full;
assign result_fifo_data = sink_fifo_data;

always begin
    if (MODE == 0) begin // Reference test
        src_fifo_rden <= ref_fifo_rden;
        src_fifo_empty <= ref_fifo_empty;
        src_fifo_data <= ref_fifo_dout;
    end else begin       // Query test
        src_fifo_rden <= query_fifo_rden;
        src_fifo_empty <= query_fifo_empty;
        src_fifo_data <= query_fifo_dout;
    end
end

// Reference FIFO
dtw_core_ref_mem #(
    .MODE(1)
) inst_dtw_core_ref_mem (
    .clk(clk),
    .addrR(src_fifo_addr),
    .addrW(15'b0),
    .wren(1'b0),
    .datain(31'b0),
    .dataout(src_fifo_data)
);


dtw_core #(
    .width(bitwidth),
    .SQG_SIZE(SQG_SIZE),
    .REF_SIZE(REF_SIZE)
) inst_dtw_core (
    .clk        (clk),
    .rst        (rst),
    .start      (start),
    .ref_len    (ref_len),
    .op_mode    (op_mode),
    .running    (running),

    .src_fifo_rden    (src_fifo_rden),
    .src_fifo_empty   (src_fifo_empty),
    .src_fifo_data    (src_fifo_data),

    .sink_fifo_wren   (sink_fifo_wren),
    .sink_fifo_full   (sink_fifo_full),
    .sink_fifo_data   (sink_fifo_data)
);

// Clock gen
always
#5 clk = ~clk;

always
src_fifo_empty <= 1'b0;
#50 src_fifo_empty <= 1'b1;
#60 src_fifo_empty <= 1'b0;

always @(posedge clk) begin
    if (!src_fifo_empty && started) begin
        src_fifo_addr <= src_fifo_addr + 1;
    end
end

initial begin
	rst = 1;
	start = 0;
    started = 0;
#250
	rst = 0;
	start = 1;
    started = 1;
#255
    start = 0;

#1000000
	$finish;
end

endmodule