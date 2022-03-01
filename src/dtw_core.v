
`timescale 1 ns / 1 ps

module dtw_core #(
    parameter dtw_dwidth = 16,  // Data width
    parameter axi_dwidth = 32,  // AXI data width
    parameter SQG_SIZE = 250,   // Squiggle size
    parameter init_ref = 0
)(
    // Main DTW signals
    input wire clk, rst, start,
    input wire [axi_dwidth-1 : 0] ref_len,
    input wire op_mode,              // Mode 0: Reference, 1: Query
    output reg running,             // Idle: 0, Running: 1

    // src FIFO signals
    output reg src_fifo_rden,                   // Src FIFO Read enable
    input wire src_fifo_empty,                   // Src FIFO Empty
    input wire [31:0] src_fifo_data,             // Src FIFO Data

    // sink FIFO signals
    output reg sink_fifo_wren,                  // Sink FIFO Write enable
    input wire sink_fifo_full,                   // Sink FIFO Full
    output reg [31:0] sink_fifo_data             // Src FIFO Data
);

/* ===============================
 * Signal declarations
 * =============================== */

// Ref mem signals
reg [14:0] addrR_ref;                   // Read address for refmem 
reg [14:0] addrW_ref;                   // Write address for refmem
reg wren_ref;                           // Write enable for refmem
wire [dtw_dwidth-1:0] dataout_ref;      // Reference data

// DTW datapath signals
reg dp_rst;                             // Reset for core
reg dp_running;                         // Run enable for core
wire dp_done;                            // Core done
reg [31:0] curr_qid;                    // Current query id
wire [dtw_dwidth-1:0] curr_minval;      // Current minimum value
wire [31:0] curr_position;              // Current best position

// Operation mode
localparam // operation mode
    MODE_NORMAL = 1'b0,
    MODE_LOAD_REF = 1'b1;

// State variables
reg [2:0] r_state;
reg [2:0] r_next_state;
localparam [2:0] // n states
    IDLE = 0,
    REF_PRELOAD = 1,
    REF_LOAD = 2,
    DTW_Q_INIT = 3,
    DTW_Q_RUN = 4,
    DTW_Q_DONE = 5;

// Others
reg [1:0] stream_out_counter;

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
        IDLE: begin // 0
            if (start) begin
                if (op_mode == MODE_NORMAL) begin
                    r_next_state <= DTW_Q_INIT;
                end else if (op_mode == MODE_LOAD_REF) begin
                    r_next_state <= REF_PRELOAD;
                end else begin
                    r_next_state <= IDLE;
                end
            end else begin
                r_next_state <= IDLE;
            end
        end
        REF_PRELOAD: begin // 1
            if (!src_fifo_empty) begin
                r_next_state <= REF_LOAD;
            end else begin
                r_next_state <= REF_PRELOAD;
            end
        end
        REF_LOAD: begin // 2
            // Continue load mode if write pointer is not at the reference length
            if (addrW_ref < ref_len) begin
                r_next_state <= REF_LOAD;
            end else begin
                r_next_state <= IDLE;
            end
        end
        DTW_Q_INIT: begin // 3
            // Read in ID if FIFO is not empty
            if (!src_fifo_empty) begin
                r_next_state <= DTW_Q_RUN;
            end else begin
                r_next_state <= DTW_Q_INIT;
            end
        end
        DTW_Q_RUN: begin // 4
            if (!dp_done) begin
                r_next_state <= DTW_Q_RUN;
            end else begin
                r_next_state <= DTW_Q_DONE;
            end
        end
        DTW_Q_DONE: begin // 5
            if (sink_fifo_full || stream_out_counter < 2'h3) begin
                r_next_state <= DTW_Q_DONE;
            end else begin
                r_next_state <= IDLE;
            end
        end
    endcase
end

// state machine output
always @(posedge clk) begin
    case (r_state)
        IDLE: begin
            running <= 0;
            src_fifo_rden <= 0;
            sink_fifo_wren <= 0;
            addrR_ref <= 0;
            addrW_ref <= 0;
            dp_rst <= 1;
            dp_running <= 0;
            stream_out_counter <= 0;
            wren_ref <= 0;
        end
        REF_PRELOAD: begin
            running <= 1;
            src_fifo_rden <= 1;
            sink_fifo_wren <= 0;
            dp_rst <= 1;
            dp_running <= 0;
            stream_out_counter <= 0;
            wren_ref <= 1;
        end
        REF_LOAD: begin
            running <= 1;
            src_fifo_rden <= 1; // Read reference
            sink_fifo_wren <= 0;
            dp_rst <= 1;
            dp_running <= 0;
            stream_out_counter <= 0;
            if (!src_fifo_empty && src_fifo_rden) begin
                addrW_ref <= addrW_ref + 1; // Increment write pointer
                wren_ref <= 1;              // FIFO not full -> write enable
            end else begin
                wren_ref <= 0;              // FIFO full -> don't write
            end
        end
        DTW_Q_INIT: begin
            running <= 1;
            src_fifo_rden <= 1; // Read enable -> read id
            sink_fifo_wren <= 0;
            dp_rst <= 0;
            dp_running <= 0;
            stream_out_counter <= 0;
            if (!src_fifo_empty) begin
                curr_qid <= src_fifo_data;
            end
        end
        DTW_Q_RUN: begin
            running <= 1;
            src_fifo_rden <= 1; // Read query data
            sink_fifo_wren <= 0;
            dp_rst <= 0;
            stream_out_counter <= 0;
            if (!src_fifo_empty) begin
                dp_running <= 1;
                addrR_ref <= addrR_ref + 1;
            end else begin
                dp_running <= 0;
            end
        end
        DTW_Q_DONE: begin
            running <= 1;
            src_fifo_rden <= 0; // Don't read enable
            dp_rst <= 0;
            dp_running <= 0;

            // Serialise output
            sink_fifo_wren <= 1;
            if (!sink_fifo_full) begin
                stream_out_counter <= stream_out_counter + 1;
                if (stream_out_counter == 0) begin
                    sink_fifo_data <= curr_qid;
                end else if (stream_out_counter == 1) begin
                    sink_fifo_data <= curr_position;
                end else if (stream_out_counter == 2) begin
                    sink_fifo_data <= {16'b0, curr_minval};
                end
            end
        end
    endcase
end

/* ===============================
 * Module instantiation
 * =============================== */

/* Reference memory */
dtw_core_ref_mem #(
    .width (dtw_dwidth),
    .initalize (init_ref)
) inst_dtw_core_ref_mem (
    .clk        (clk),
    .addrR      (addrR_ref),
    .addrW      (addrW_ref),
    .wren       (wren_ref),
    .datain     (src_fifo_data[15:0]),
    .dataout    (dataout_ref)
);

/* DTW datapath */
dtw_core_datapath #(
    .width(dtw_dwidth),
    .SQG_SIZE(SQG_SIZE)
) inst_dtw_core_datapath (
    .clk            (clk),
    .rst            (dp_rst),
    .running        (dp_running),
    .Input_squiggle (src_fifo_data[15:0]),
    .Rword          (dataout_ref),
    .ref_len        (ref_len),
    .minval         (curr_minval),
    .position       (curr_position),
    .done           (dp_done)
);

endmodule