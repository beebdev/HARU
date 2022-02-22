`timescale 1ns / 1ps

module dtw_core_datapath #(
    parameter width = 16,
    parameter SQG_SIZE =250
)(
    input clk, rst, running,
    input [width-1:0] Input_squiggle, Rword,
    input [31:0] ref_len,
    output [width-1:0] minval,
    output [31:0] position,
    output done
);

integer k;

reg [31:0] cycle_counter;
reg [7:0]  squiggle_buffaddress;

reg [width-1:0] Squiggle_Buffer [1:SQG_SIZE];
reg [width-1:0] Rword_buff;

wire [width-1:0] DTW_curr [1:SQG_SIZE];
wire [width-1:0] p_Rword  [1:SQG_SIZE];

reg [width-1:0] DTW_prev  [1:SQG_SIZE];
reg [width-1:0] DTW_pprev [1:SQG_SIZE];
reg [0:SQG_SIZE+1] running_d;

reg [width-1:0] Minval;
reg [31:0]      Minpos ;
reg [width-1:0] DTW_lastrow;


/* ===============================
 * PE running status
 * =============================== */
always @(posedge clk) begin
    if(rst) begin
        for (k = 0; k <= SQG_SIZE + 1; k = k + 1) begin
            running_d[k] <= 0;
        end
    end else if (running) begin
        running_d[0] <= 1;
        for (k = 1; k <= SQG_SIZE + 1; k = k + 1) begin
            running_d[k] <= running_d[k-1];
        end
    end
end

/* ===============================
 * DTW prev and pprev value
 * =============================== */
always @(posedge clk) begin
    if (rst) begin
        for(k = 1; k <= SQG_SIZE; k = k + 1) begin
            DTW_prev [k] <= -1;
            DTW_pprev[k] <= -1;
        end
    end else if (running) begin
        for(k = 1; k <= SQG_SIZE; k = k + 1) begin
            if(running_d[k]) begin
                DTW_prev[k] <= DTW_curr[k];
            end
            if(running_d[k+1]) begin
                DTW_pprev[k] <= DTW_prev[k];
            end
        end
    end
end
	
/* ===============================
 * Load squiggle value
 * =============================== */
always @(posedge clk) begin
    if (rst) begin
        for(k = 1; k <= SQG_SIZE; k = k + 1) begin
            Squiggle_Buffer[k] <= 0;
        end
    end else if (running) begin
        if (running_d[0]) begin
            Squiggle_Buffer[squiggle_buffaddress] <= Input_squiggle;
        end
    end
end

/* ===============================
 * Handle Squiggle buffer address
 * =============================== */
always @(posedge clk) begin
    if (rst) begin
        squiggle_buffaddress <= 1;
    end else if (running) begin
        if(running_d[0] && (squiggle_buffaddress <= SQG_SIZE)) begin
            squiggle_buffaddress <= squiggle_buffaddress + 1;
        end
    end
end

/* ===============================
 * Load Rword value
 * =============================== */
always @(posedge clk) begin
    if (rst) begin
        Rword_buff <= 0;
    end else if (running) begin
        if(running_d[0]) begin
            Rword_buff <= Rword;
        end
    end
end

/* ===============================
 * Cycle counter
 * =============================== */
always @(posedge clk) begin
    if (rst) begin
        cycle_counter <=  0;
    end else if (running) begin
        if(running_d[SQG_SIZE]) begin
            cycle_counter <= cycle_counter + 1;
        end
    end
end

/* ===============================
 * Last row value
 * =============================== */
always @(posedge clk) begin
    if (rst) begin
        DTW_lastrow <= -1;
    end else if (running) begin
        if(running_d[SQG_SIZE]) begin
            DTW_lastrow <= DTW_curr[SQG_SIZE];
        end
    end
end

/* ===============================
 * Min value and position update
 * =============================== */
always @(posedge clk) begin
    if (rst) begin
        Minval <= -1;
        Minpos <= 0;
    end else if (running) begin
        if(DTW_lastrow < Minval) begin 
            Minval <= DTW_lastrow;
            Minpos <= cycle_counter;
        end
    end
end

/* ===============================
 * Assignments
 * =============================== */
assign DTW_minval = Minval;
assign position   = Minpos;
assign done       = (cycle_counter >= ref_len); // REF_LEN + SQG_LEN

/* ===============================
 * PE DTW calculation
 * =============================== */
dtw_core_pe #(
    .width(width)
) inst_dtw_core_pe_001 (
    .clk  (clk),
    .rst  (rst),
    .x    (Squiggle_Buffer[001]),
    .y    (Rword_buff),
    .W    (DTW_prev[001]),
    .N    (32'd0),
    .NW   (32'd0),
    .DTWc (DTW_curr[001]),
    .yp   (p_Rword[001])
);

genvar m;
generate
for (m = 2; m <= SQG_SIZE; m = m + 1) begin
	dtw_core_pe #(
        .width(width)
    ) inst_dtw_core_pe_n (
        .clk    (clk),
        .rst    (rst),
        .x      (Squiggle_Buffer[m]),
        .y      (p_Rword[m-1]),
        .W      (DTW_prev[m]),
        .N      (DTW_prev[m-1]),
        .NW     (DTW_pprev[m-1]),
        .DTWc   (DTW_curr[m]),
        .yp     (p_Rword[m])
    );
end
endgenerate

endmodule