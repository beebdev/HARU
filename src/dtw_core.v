`timescale 1 ns / 1 ps

module dtw_core #(
    parameter WIDTH         = 16,   // Data width
    parameter AXIS_WIDTH    = 32,   // AXI data width
    parameter SQG_SIZE      = 250,  // Squiggle size
    parameter REF_INIT      = 0
)(
    // Main DTW signals
    input   wire                    clk,
    input   wire                    rst,
    input   wire                    rs,

    input   wire [AXIS_WIDTH-1 : 0] ref_len,
    input   wire                    op_mode,            // Reference mode: 0, query mode: 1
    output  reg                     busy,               // Idle: 0, busy: 1
    output  wire                    load_done,

    // Src FIFO signals
    output  wire                    src_fifo_clear,     // Src FIFO Clear signal
    output  reg                     src_fifo_rden,      // Src FIFO Read enable
    input   wire                    src_fifo_empty,     // Src FIFO Empty
    input   wire [31:0]             src_fifo_data,      // Src FIFO Data

    // Sink FIFO signals
    output  reg                     sink_fifo_wren,     // Sink FIFO Write enable
    input   wire                    sink_fifo_full,     // Sink FIFO Full
    output  reg [31:0]              sink_fifo_data,     // Sink FIFO Data
    output  reg                     sink_fifo_last      // Sink FIFO Last
);

/* ===============================
 * local parameters
 * =============================== */
// Operation mode
localparam
    MODE_NORMAL = 1'b0,
    MODE_LOAD_REF = 1'b1;

// FSM states
localparam [2:0] // n states
    IDLE = 0,
    REF_LOAD = 1,
    DTW_Q_INIT = 2,
    DTW_Q_RUN = 3,
    DTW_Q_DONE = 4;

/* ===============================
 * registers/wires
 * =============================== */
reg r_load_done;
reg r_src_fifo_clear;

// Ref mem signals
reg  [14:0]         addrR_ref;          // Read address for refmem 
reg  [14:0]         addrW_ref;          // Write address for refmem
reg                 wren_ref;           // Write enable for refmem
wire [WIDTH-1:0]    dataout_ref;        // Reference data

// DTW datapath signals
reg                 dp_rst;             // dp core reset
reg                 dp_running;         // dp core run enable
wire                dp_done;            // dp core done
reg  [31:0]         curr_qid;           // Current query id
wire [WIDTH-1:0]    curr_minval;        // Current minimum value
wire [31:0]         curr_position;      // Current best match position

// FSM state
reg [2:0] r_state;

// Others
reg [1:0] stall_counter;

/* ===============================
 * submodules
 * =============================== */
// Reference memory
dtw_core_ref_mem #(
    .width      (WIDTH),
    .initalize  (REF_INIT)
) inst_dtw_core_ref_mem (
    .clk            (clk),
    .addrR          (addrR_ref),
    .addrW          (addrW_ref),
    .wren           (wren_ref),
    .datain         (src_fifo_data[15:0]),
    .dataout        (dataout_ref)
);

// DTW datapath
dtw_core_datapath #(
    .width      (WIDTH),
    .SQG_SIZE   (SQG_SIZE)
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

/* ===============================
 * asynchronous logic
 * =============================== */
assign load_done = r_load_done;
assign src_fifo_clear = r_src_fifo_clear;

/* ===============================
 * synchronous logic
 * =============================== */
// FSM State change
always @(posedge clk) begin
    if (rst) begin
        r_state <= IDLE;
        r_load_done <= 0;
    end else begin
        case (r_state)
        IDLE: begin
            if (rs) begin
                if (op_mode == MODE_NORMAL && r_load_done == 1) begin
                    r_state <= DTW_Q_INIT;
                end else if (op_mode == MODE_LOAD_REF && r_load_done == 0) begin
                    r_state <= REF_LOAD;
                end else begin
                    r_state <= IDLE;
                end
            end else begin
                r_state <= IDLE;
            end
        end
        REF_LOAD: begin
            if (addrW_ref < ref_len) begin
                r_state <= REF_LOAD;
            end else begin
                r_load_done <= 1;
                r_state <= IDLE;
            end
        end
        DTW_Q_INIT: begin
            if (!src_fifo_empty) begin
                r_state <= DTW_Q_RUN;
            end else begin
                r_state <= DTW_Q_INIT;
            end
        end
        DTW_Q_RUN: begin
            if (!dp_done) begin
                r_state <= DTW_Q_RUN;
            end else begin
                r_state <= DTW_Q_DONE;
            end
        end
        DTW_Q_DONE: begin
            if (sink_fifo_full || stall_counter < 2'h3) begin
                r_state <= DTW_Q_DONE;
            end else begin
                r_state <= IDLE;
            end
        end
        endcase
    end
end

// FSM output
always @(posedge clk) begin
    case (r_state)
    IDLE: begin
        busy                <= 0;
        src_fifo_rden       <= 0;
        sink_fifo_wren      <= 0;
        addrR_ref           <= 0;
        addrW_ref           <= 0;
        dp_rst              <= 1;
        dp_running          <= 0;
        stall_counter       <= 0;
        wren_ref            <= 0;
        r_src_fifo_clear    <= 1;
        sink_fifo_last      <= 0;
    end
    REF_LOAD: begin
        busy                <= 1;
        src_fifo_rden       <= 1;
        sink_fifo_wren      <= 0;
        dp_rst              <= 1;
        dp_running          <= 0;
        stall_counter       <= 0;
        r_src_fifo_clear    <= 0;

        if (!src_fifo_empty && src_fifo_rden) begin
            addrW_ref       <= addrW_ref + 1;
            wren_ref        <= 1;
        end else begin
            wren_ref        <= 0;
        end
    end
    DTW_Q_INIT: begin
        busy                <= 1;
        src_fifo_rden       <= 1;
        sink_fifo_wren      <= 0;
        dp_rst              <= 0;
        dp_running          <= 0;
        stall_counter       <= 0;
        r_src_fifo_clear    <= 0;

        if (!src_fifo_empty) begin
            curr_qid        <= src_fifo_data;
            dp_running      <= 1;
        end else begin
            dp_running      <= 0;
        end
    end
    DTW_Q_RUN: begin
        busy                <= 1;
        sink_fifo_wren      <= 0;
        dp_rst              <= 0;
        stall_counter       <= 0;
        r_src_fifo_clear    <= 0;
            
        if (!src_fifo_empty) begin
            if (addrR_ref < SQG_SIZE) begin
                src_fifo_rden <= 1;
            end else begin
                src_fifo_rden <= 0;
            end
            dp_running <= 1;
        end

        if (addrR_ref >= SQG_SIZE || !src_fifo_empty) begin
            addrR_ref <= addrR_ref + 1;
        end
    end
    DTW_Q_DONE: begin
        busy                <= 1;
        src_fifo_rden       <= 0;
        dp_rst              <= 0;
        dp_running          <= 0;

        // Serialize output
        if (!sink_fifo_full) begin
            stall_counter   <= stall_counter + 1;
            if (stall_counter == 0) begin
                sink_fifo_last  <= 0;
                sink_fifo_wren  <= 1;
                sink_fifo_data  <= curr_qid;
            end else if (stall_counter == 1) begin
                sink_fifo_last  <= 0;
                sink_fifo_wren  <= 1;
                sink_fifo_data  <= curr_position;
            end else if (stall_counter == 2) begin
                sink_fifo_last  <= 0;
                sink_fifo_wren  <= 1;
                sink_fifo_data  <= {16'b0, curr_minval};
            end else begin
                sink_fifo_last  <= 1;
                sink_fifo_wren  <= 0;
                sink_fifo_data  <= 0;
            end
        end 
    end
    endcase
end

endmodule