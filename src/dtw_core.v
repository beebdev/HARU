
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
    input op_mode,              // Mode 0: Reference, 1: Query
    output running,             // Idle: 0, Running: 1

    // src FIFO signals
    output src_fifo_rden,                   // Src FIFO Read enable
    input src_fifo_empty,                   // Src FIFO Empty
    input [31:0] src_fifo_data,             // Src FIFO Data

    // sink FIFO signals
    output sink_fifo_wren,                  // Sink FIFO Write enable
    input sink_fifo_full,                   // Sink FIFO Full
    output [dtw_dwidth-1:0] sink_minval,
    output [31:0] sink_position,
    output [31:0] sink_qid
);

/* Internal signals */
wire [dtw_dwidth-1:0] dataout_ref;      // Reference data

reg [14:0] addrR_ref;                   // Read address for refmem 
reg [14:0] addrW_ref;                   // Write address for refmem

reg [31:0] curr_qid;                    // Current query id
reg param_rst;                          // Reset for core
reg param_running;                      // Run enable for core
reg param_done;

wire wren_ref;                          // Write enable for refmem

// Operation mode
localparam // operation mode
    MODE_NORMAL = 1'b0,
    MODE_LOAD_REF = 1'b1;

// OP state variables
reg [1:0] r_state;
reg [1:0] r_next_state;
localparam [1:0] // n states
    IDLE = 0,
    REF_LOAD = 1,
    DTW_Q_RUN = 2,
    DTW_Q_DONE = 3;

assign sink_qid = curr_qid;

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

// State change
always @(posedge clk) begin
    case (r_state)
        IDLE: begin
            if (start) begin
                if (op_mode == MODE_NORMAL) begin
                    r_next_state <= DTW_Q_INIT;
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
        DTW_Q_INIT: begin
            // Read in ID if FIFO is not empty
            if (!src_fifo_empty) begin
                r_next_state <= DTW_Q_RUN;
            end else begin
                r_next_state <= DTW_Q_INIT;
            end
        end
        DTW_Q_RUN: begin
            if (!param_done) begin
                r_next_state <= DTW_Q_RUN;
            end else begin
                r_next_state <= DTW_Q_DONE;
            end
        end
        DTW_Q_DONE: begin
            if (sink_fifo_full) begin
                r_next_state <= DTW_Q_DONE;
            end else begin
                r_next_state <= IDLE;
            end
        end
    endcase
end

// state machine output
always @(posedge) begin
    case (r_state) begin
        IDLE: begin
            running <= 0;
            src_fifo_rden <= 0; // Don't read enable
            addrR_ref <= 0;
            addrW_ref <= 0;
            param_rst <= 1;
            param_running <= 0;
        end
        REF_LOAD: begin
            running <= 1;
            src_fifo_rden <= 1; // Read enable -> read reference
            param_rst <= 1;
            param_running <= 0;
            if (!src_fifo_empty) begin
                addrW_ref <= addrW_ref + 1; // Increment write pointer
                wren_ref <= 1;              // FIFO not full -> write enable
            end else begin
                wren_ref <= 0;              // FIFO full -> don't write
            end
        end
        DTW_Q_INIT: begin
            running <= 1;
            src_fifo_rden <= 1; // Read enable -> read id
            param_rst <= 1;
            param_running <= 0;
            if (!src_fifo_empty) begin
                curr_qid <= src_fifo_data;
            end
        end
        DTW_Q_RUN: begin
            running <= 1;
            src_fifo_rden <= 1; // Read enable -> read data
            param_rst <= 0;
            if (!src_fifo_empty) begin
                param_running <= 1;
                addrR_ref <= addrR_ref + 1;
            end else begin
                param_running <= 0;
            end
        end
        DTW_Q_DONE: begin
            running <= 1;
            src_fifo_rden <= 0; // Don't read enable
            param_rst <= 0;
            param_running <= 0;
        end
    endcase
end

/* ===============================
 * Module instantiation
 * =============================== */

/* Reference memory */
dtw_core_ref_mem #(
    .width (dtw_dwidth)
) inst_dtw_core_ref_mem (
    .clk        (clk),
    .addrR      (addrR_ref),
    .addrW      (addrW_ref),
    .wren       (wren_ref),
    .datain     (src_fifo_data),
    .dataout    (dataout_ref)
);

/* DTW datapath */
dtw_core_datapath #(
    .width(dtw_dwidth),
    .SQG_SIZE(SQG_SIZE),
    .REF_SIZE(REF_SIZE)
) inst_dtw_core_datapath (
    .clk            (clk),
    .rst            (param_rst),
    .running        (param_running),
    .Input_squiggle (src_fifo_data),
    .Rword          (dataout_ref),
    .ref_len        (ref_len),
    .DTW_minval     (sink_minval),
    .position       (sink_position),
    .done           (param_done)
);

endmodule