`timescale 1ns / 1ps

module dgn_dtwdatapath #(
    parameter width = 18,
    parameter SQG_SIZE = 250,
    parameter REF_SIZE = 29898
) (
    input clk, rst, running,
    input [width-1:0] Input_squiggle, Rword,
    output [width-1:0] DTW_minval,
    output [31:0] position,
    output done
);

integer k;

reg  [31:0] cycle_counter;

reg  [width-1:0] Squiggle_Buffer [1:SQG_SIZE];
reg  [width-1:0] Rword_buff;

wire [width-1:0] DTW_curr  [1:SQG_SIZE];
wire [width-1:0] p_Rword   [1:SQG_SIZE];

reg  [width-1:0] DTW_prev  [1:SQG_SIZE];
reg  [width-1:0] DTW_pprev [1:SQG_SIZE];
reg  [0:SQG_SIZE+1] running_d;

reg  [width-1:0] Minval;
reg  [31:0]      Minpos ;
reg  [width-1:0] DTW_lastrow;

//--------------------------
always @(posedge clk)
if(rst)
    DTW_lastrow <= -1;
else if(running_d[SQG_SIZE])
    DTW_lastrow <= DTW_curr[SQG_SIZE];

//--------------------------
always @(posedge clk)
if(rst) 
	for(k = 0; k <= SQG_SIZE + 1; k = k + 1)
        running_d[k] <= 0;
else begin
	running_d[0] <= running;
	for(k = 1; k <= SQG_SIZE; k = k + 1)
        running_d[k] <= running_d[k-1];
end

//--------------------------
always @(posedge clk)
if(rst)
	for(k = 1; k <= SQG_SIZE; k = k + 1) begin
		DTW_prev  [k] <= -1;
		DTW_pprev [k] <= -1;
	end
else
	for(k = 1; k <= SQG_SIZE; k = k + 1) begin
		if(running_d[k  ])
            DTW_prev  [k]<= DTW_curr[k];
		if(running_d[k+1])
            DTW_pprev [k]<= DTW_prev[k];
	end
	
//--------------------------
always @(posedge clk)
if (rst) 
	for(k = 1; k <= SQG_SIZE; k = k + 1)
		Squiggle_Buffer[k] <= 0;
else if(running_d[0])
		Squiggle_Buffer[cycle_counter]<= Input_squiggle;

always @(posedge clk)
if(rst) begin
	cycle_counter <=  1;
	Rword_buff    <=  0;
end else if(running_d[0]) begin
	cycle_counter <= cycle_counter + 1;
	Rword_buff    <= Rword;
end

// Minimum position and value
always @(posedge clk)
if (rst) begin
		Minval <= -1;
		Minpos <= 0;
end else if(DTW_lastrow < Minval) begin 
		Minval <=  DTW_curr[SQG_SIZE];
		Minpos <=  cycle_counter;
end

// output signals
assign DTW_minval = Minval;
assign position   = Minpos;
assign done       = (cycle_counter >= (SQG_SIZE + REF_SIZE) ); //48497+250


// PE array
dgn_processingelement #(
    .width(width)
) inst_dgn_pe_1 (
    .clk  (clk),
    .rst (rst),
    .x  (Squiggle_Buffer[001]),
    .y (Rword_buff),
    .W (DTW_prev[001]),
    .N (32'd0),
    .NW (32'd0),
    .DTWc (DTW_curr[001]),
    .yp (p_Rword[001])
);

genvar i;
generate
    for (i = 2; i < SQG_SIZE; i = i + 1) begin
        dgn_processingelement #(
            .width (width)
        ) inst_dgn_pe_i (
            .clk (clk),
            .rst (rst),
            .x (Squiggle_Buffer[i]),
            .y (p_Rword[i-1]),
            .W (DTW_prev[i]),
            .N (DTW_prev[i-1]),
            .NW (DTW_pprev[i-1]),
            .DTWc (DTW_curr[i]),
            .yp (p_Rword[i])
        );
    end
endgenerate

endmodule