`timescale 1 ns / 1 ps

module dtw_core #(
    parameter dtw_dwidth = 16,  // Data width
    parameter axi_dwidth = 32,  // AXI data width
    parameter SQG_SIZE = 250,   // Squiggle size
    parameter REF_SIZE = 29898  // Reference size
)(
    // Main DTW signals
    input clk, rst, start,
    input [axi_dwidth-1 : 0] ref_len,
    output [dtw_dwidth-1:0] DTW_minval,
    output [axi_dwidth-1 : 0] position,
    output running,
    output done,

    // DTW FIFO signals
    input op_mode,
    output fifo_rden,
    input fifo_empty,
    input [dtw_dwidth-1:0] fifo_data,

    // debug ports
    output [31:0] DTW_debug0,
    output [31:0] DTW_debug1
);

/* Internal signals */
wire [dtw_dwidth-1:0] dataout_squiggle;
wire [dtw_dwidth-1:0] dataout_ref;

reg [14:0] addrR_ref;
reg [7:0]  addrR_squiggle;

reg [axi_dwidth:0] debug0;
reg [axi_dwidth:0] debug1;

assign DTW_debug0 = debug0;
assign DTW_debug1 = debug1;

// Operation mode
localparam // operation mode
    MODE_NORMAL = 1'b0,
    MODE_LOAD_REF = 1'b1;

// OP state variables
reg [1:0] r_state;
reg [1:0] r_next_state;
parameter [1:0] // n states
    IDLE = 0,
    REF_LOAD = 1,
    DTW_Q_RUN = 2,
    DTW_Q_DONE = 3;


/* ===============================
 * Core FSM 
 * =============================== */

// State transition
always @(posedge clk) begin
    if (rst) begin
        r_state <= IDLE;
    end else begin
        r_state <= r_next_state;
    end
end

// State machine
always @(posedge clk) begin
    case (r_state)
        IDLE: begin
            if (start) begin
                if (op_mode == MODE_NORMAL) begin
                    r_next_state <= DTW_Q_RUN;
                end else if (op_mode == MODE_LOAD_REF) begin
                    r_next_state <= REF_LOAD
                end else begin
                    r_next_state <= IDLE;
                end
            end else begin
                r_next_state <= IDLE;
            end
        end
        REF_LOAD: begin
            // Continue load mode if write pointer is not at the reference length
            if (addrW_ref < ref_len) begin
                r_next_state <= REF_LOAD;
            end else begin
                r_next_state <= IDLE;
            end
        end
        DTW_Q_RUN: begin // TODO: not done
            if (!param_done) begin
                r_next_state <= DTW_Q_RUN;
            end else begin
                r_next_state <= DTW_Q_DONE;
            end
        end
        DTW_Q_DONE: begin
            r_next_state <= IDLE;
        end
    end
end

always @(posedge) begin
    case (r_state)
        IDLE: begin
            running <= 0;
            fifo_rden <= 0; // Don't read enable
            addrW_ref <= 0;
            param_rst <= 1;
            param_running <= 0;
            done <= 0;
        end
        REF_LOAD: begin
            running <= 1;
            fifo_rden <= 1;
            param_rst <= 1;
            param_running <= 0;
            done <= 0;
            if (!fifo_empty) begin
                addrW_ref <= addrW_ref + 1;
                wren_ref <= 1;
            end else begin
                wren_ref <= 0;
            end
        end
        DTW_Q_RUN: begin
            running <= 1;
            fifo_rden <= 1;
            param_rst <= 0;
            done <= 0;
            if (!fifo_empty) begin
                param_running <= 1;
                addrR_ref <= addrR_ref + 1;
            end else begin
                param_running <= 0;
            end
        end
        DTW_Q_DONE: begin
            running <= 1;
            fifo_rden <= 0;
            param_rst <= 0;
            done <= 1;
        end
end


/* Reference memory */
dgn_memreference #(
    .width (dtw_dwidth)
) inst_dgn_memreference (
    .clk        (clk),
    .addrR      (addrR_ref),
    .addrW      (addrW_ref),
    .wren       (wren_ref),
    .datain     (fifo_data),
    .dataout    (dataout_ref)
);

/* DTW datapath */
dgn_dtwdatapath_param #(
    .width(dtw_dwidth),
    .SQG_SIZE(SQG_SIZE),
    .REF_SIZE(REF_SIZE)
) inst_dgn_dtwdatapath (
    .clk            (clk),
    .rst            (param_rst),
    .running        (param_running),
    .Input_squiggle (fifo_data),
    .Rword          (dataout_ref),
    .DTW_minval     (DTW_minval),
    .position       (position),
    .done           (param_done)
);

endmodule