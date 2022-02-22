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

wire sink_fifo_wren;
wire sink_fifo_full;
wire [31:0] sink_fifo_data;

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
stim_fifo #(
    .SQG_SIZE(SQG_SIZE),
    .REF_SIZE(REF_SIZE)
) ref_fifo (
    .clk(clk),
    .rst(rst),
    .wren(ref_fifo_wren),
    .rden(ref_fifo_rden),
    .fifo_empty(ref_fifo_empty),
    .fifo_full(ref_fifo_full),
    .data_in(ref_fifo_din),
    .data_out(ref_fifo_dout)
);

// Query FIFO
stim_fifo #(
    .SQG_SIZE(SQG_SIZE),
    .REF_SIZE(REF_SIZE)
) query_fifo (
    .clk(clk),
    .rst(rst),
    .wren(query_fifo_wren),
    .rden(query_fifo_rden),
    .fifo_empty(query_fifo_empty),
    .fifo_full(query_fifo_full),
    .data_in(query_fifo_din),
    .data_out(query_fifo_dout)
);

// Output FIFO
stim_fifo #(
    .SQG_SIZE(SQG_SIZE),
    .REF_SIZE(REF_SIZE)
) result_fifo (
    .clk(clk),
    .rst(rst),
    .wren(result_fifo_wren),
    .rden(result_fifo_rden),
    .fifo_empty(result_fifo_empty),
    .fifo_full(result_fifo_full),
    .data_in(result_fifo_din),
    .data_out(result_fifo_dout)
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


initial begin
	rst = 1;
	start = 0;
#250
	rst = 0;
	start = 1;
#255
    start = 0;

#1000000
	$finish;
end

endmodule