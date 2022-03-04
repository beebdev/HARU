module stim_code;

// Parameters
parameter MODE = 1; // 0->query; 1->ref
parameter bitwidth = 16;
parameter SQG_SIZE = 250;
parameter REF_SIZE = 29898;

// Module IO
reg clk = 0;  
reg rst;
reg running;
reg [31:0] ref_len = REF_SIZE;
reg op_mode;
wire busy;
wire load_done;

wire src_fifo_rden;
reg src_fifo_empty;
wire [15:0] src_fifo_data;
reg [14:0] src_fifo_addr;

wire sink_fifo_wren;
reg sink_fifo_full;
wire [31:0] sink_fifo_data;

reg started;

/* IO assignment */

// Reference FIFO
stim_mem #(
    .init_mode(MODE)
) inst_stim_mem (
    .clk(clk),
    .addrR(src_fifo_addr),
    .addrW(15'b0),
    .wren(1'b0),
    .datain(16'b0),
    .dataout(src_fifo_data)
);

dtw_core #(
    .dtw_dwidth(bitwidth),
    .axi_dwidth(32),
    .SQG_SIZE(SQG_SIZE),
    .init_ref(0)
) inst_dtw_core (
    .clk        (clk),
    .rst        (rst),
    .running    (running),
    .ref_len    (ref_len),
    .op_mode    (op_mode),
    .busy       (busy),
    .load_done  (load_done),

    .src_fifo_rden    (src_fifo_rden),
    .src_fifo_empty   (src_fifo_empty),
    .src_fifo_data    ({16'b0, src_fifo_data}),

    .sink_fifo_wren   (sink_fifo_wren),
    .sink_fifo_full   (sink_fifo_full),
    .sink_fifo_data   (sink_fifo_data)
);

// Clock gen
always
#5 clk = ~clk;

//assign src_fifo_empty = 1'b0;
always begin
    src_fifo_empty <= 1'b0;
     #65 src_fifo_empty <= 1'b1;
     #10 src_fifo_empty <= 1'b0;
     #5 src_fifo_empty <= 1'b0;
end

always @(posedge clk) begin
    if (rst) begin
        src_fifo_addr <= 0;
    end else if (!src_fifo_empty && started && src_fifo_rden) begin
        src_fifo_addr <= src_fifo_addr + 1;
    end
end

initial begin
	rst = 1;
	running = 0;
    started = 0;
    op_mode = MODE;
    sink_fifo_full = 1'b0;
#20
	rst = 0;
	running = 1;
    started = 1;
#1000000
	$finish;
end

endmodule