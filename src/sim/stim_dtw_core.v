module stim_code;

parameter bitwidth = 16;
parameter SQG_SIZE = 250;
parameter REF_SIZE = 29898;

reg clk = 0;  
reg rst;
reg running; 

wire [bitwidth-1:0] DTW_minval;
wire [31:0]         position ; 
wire done;

wire [31:0] debug0;
wire [31:0] debug1;

dtw_core #(
    .width(bitwidth),
    .SQG_SIZE(SQG_SIZE),
    .REF_SIZE(REF_SIZE)
) inst_dtw_core (
    .clk        (clk),
    .rst        (rst),
    .running    (running),
    .DTW_minval (DTW_minval),
    .position   (position),
    .done       (done),

    .DTW_debug0 (debug0),
    .DTW_debug1 (debug1)
);

always
#5 clk = ~clk;

initial begin
	rst = 1;
	running = 0;
# 250
	rst = 0;
	running = 1;

#1000000	
    $display("Min Value = %d", DTW_minval);
    $display("Position  = %d", position);
	$finish;
end

endmodule