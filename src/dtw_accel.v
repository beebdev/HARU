`timescale 1 ns / 1 ps

module dtw_accel #(
    /* HARU-PL parameters */
    parameter width = 16,
    parameter SQG_SIZE = 250,

    /* Parameters of Axi Slave Bus Interface S00_AXI */
    parameter integer C_S00_AXI_DATA_WIDTH	= 32,
    parameter integer C_S00_AXI_ADDR_WIDTH	= 5,

    /* Parameters of Axi stream */
    parameter integer C_AXIS_TDATA_WIDTH = 32
)(
    /* Common ports */
    input wire clk,
    input wire aresetn,

    /* AXI stream */
    input wire axis_clk,
    input wire axis_aresetn,

    /* S00_AXI ports */
    input wire  [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_awaddr,
    input wire  [2 : 0] s00_axi_awprot,
    input wire  s00_axi_awvalid,
    output wire s00_axi_awready,
    input wire  [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_wdata,
    input wire  [(C_S00_AXI_DATA_WIDTH/8)-1 : 0] s00_axi_wstrb,
    input wire  s00_axi_wvalid,
    output wire s00_axi_wready,
    output wire [1 : 0] s00_axi_bresp,
    output wire s00_axi_bvalid,
    input wire  s00_axi_bready,
    input wire  [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_araddr,
    input wire  [2 : 0] s00_axi_arprot,
    input wire  s00_axi_arvalid,
    output wire s00_axi_arready,
    output wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_rdata,
    output wire [1 : 0] s00_axi_rresp,
    output wire s00_axi_rvalid,
    input wire  s00_axi_rready,

    /* S00_AXIS ports */
    output wire  s00_axis_tready,
    input wire [C_AXIS_TDATA_WIDTH-1 : 0] s00_axis_tdata,
    input wire [(C_AXIS_TDATA_WIDTH/8)-1 : 0] s00_axis_tstrb,
    input wire  s00_axis_tlast,
    input wire  s00_axis_tvalid,

    /* M00_AXIS ports */
    output wire m00_axis_tvalid,
	output wire [C_AXIS_TDATA_WIDTH-1 : 0] m00_axis_tdata,
	output wire [(C_AXIS_TDATA_WIDTH/8)-1 : 0] m00_axis_tstrb,
	output wire  m00_axis_tlast,
    input wire m00_axis_tready
);

/* =========================
 * Signal declaration
 * ========================= */

// DTW signals within S AXI
wire [C_AXIS_TDATA_WIDTH-1:0] s00_dtw_cr;             // DTW Core control register
wire [C_AXIS_TDATA_WIDTH-1:0] s00_dtw_sr;             // DTW Core status register
wire [C_AXIS_TDATA_WIDTH-1:0] s00_dtw_ref_len;        // DTW Core reference length

// Control wires
wire s00_dtw_reset;
wire s00_dtw_rs;
wire s00_dtw_mode;
wire s00_dtw_busy;
wire s00_dtw_load_done;

// Src AXIS FIFO signals
wire s00_axis_fifo_rden;
wire [C_AXIS_TDATA_WIDTH-1:0] s00_axis_fifo_dout;
wire s00_axis_fifo_empty;

// Sink AXIS FIFO signals
wire m00_axis_dtw_fifo_wren;
wire [C_AXIS_TDATA_WIDTH-1:0] m00_axis_dtw_fifo_din;
wire m00_axis_dtw_fifo_full;

// debug
wire [31:0] w_dbg_addrW_ref;
wire [31:0] w_dbg_ref_dout;
wire [2:0] w_dbg_curr_state;

/* =========================
 * IO assignments
 * ========================= */
 
// DTW CR
assign s00_dtw_reset = s00_dtw_cr[0];        // CR Offset 0: reset
assign s00_dtw_rs = s00_dtw_cr[1];           // CR Offset 1: run/stop
assign s00_dtw_mode = s00_dtw_cr[2];         // CR Offset 2: mode -> 0: q mode, 1: ref load

// DTW SR
// SR offset 0: busy
// SR offset 1: reference loading done
assign s00_dtw_sr = {26'b0, w_dbg_curr_state, s00_dtw_load_done, s00_dtw_busy};

/* =========================
 * Module instantiation
 * ========================= */

// AXI Bus S00_AXI
dtw_accel_S00_AXI # (
    .C_S_AXI_DATA_WIDTH (C_S00_AXI_DATA_WIDTH),
    .C_S_AXI_ADDR_WIDTH (C_S00_AXI_ADDR_WIDTH)
) dtw_accel_S00_AXI_inst (
    // dtw
    .dtw_cr         (s00_dtw_cr),
    .dtw_sr         (s00_dtw_sr),
    .dtw_ref_len    (s00_dtw_ref_len),
    .dtw_dbg_addrW_ref (w_dbg_addrW_ref),
    .dtw_dbg_rd_ref_dout (w_dbg_ref_dout),

    // axi
    .S_AXI_ACLK     (clk),
    .S_AXI_ARESETN  (aresetn),
    .S_AXI_AWADDR   (s00_axi_awaddr),
    .S_AXI_AWPROT   (s00_axi_awprot),
    .S_AXI_AWVALID  (s00_axi_awvalid),
    .S_AXI_AWREADY  (s00_axi_awready),
    .S_AXI_WDATA    (s00_axi_wdata),
    .S_AXI_WSTRB    (s00_axi_wstrb),
    .S_AXI_WVALID   (s00_axi_wvalid),
    .S_AXI_WREADY   (s00_axi_wready),
    .S_AXI_BRESP    (s00_axi_bresp),
    .S_AXI_BVALID   (s00_axi_bvalid),
    .S_AXI_BREADY   (s00_axi_bready),
    .S_AXI_ARADDR   (s00_axi_araddr),
    .S_AXI_ARPROT   (s00_axi_arprot),
    .S_AXI_ARVALID  (s00_axi_arvalid),
    .S_AXI_ARREADY  (s00_axi_arready),
    .S_AXI_RDATA    (s00_axi_rdata),
    .S_AXI_RRESP    (s00_axi_rresp),
    .S_AXI_RVALID   (s00_axi_rvalid),
    .S_AXI_RREADY   (s00_axi_rready)
);

// AXI Stream Bus S00_AXIS
dtw_accel_S00_AXIS # ( 
   .C_S_AXIS_TDATA_WIDTH(C_AXIS_TDATA_WIDTH)
) dtw_accel_S00_AXIS_inst (
    // dtw
    .dtw_fifo_rden  (s00_axis_fifo_rden),
    .dtw_fifo_dout  (s00_axis_fifo_dout),
    .dtw_fifo_empty (s00_axis_fifo_empty),

    // axis
   .S_AXIS_ACLK     (axis_clk),
   .S_AXIS_ARESETN  (axis_aresetn),
   .S_AXIS_TREADY   (s00_axis_tready),
   .S_AXIS_TDATA    (s00_axis_tdata),
   .S_AXIS_TSTRB    (s00_axis_tstrb),
   .S_AXIS_TLAST    (s00_axis_tlast),
   .S_AXIS_TVALID   (s00_axis_tvalid)
);

// AXI Stream Bus M00_AXIS
dtw_accel_M00_AXIS #(
    .C_M_AXIS_TDATA_WIDTH (C_AXIS_TDATA_WIDTH)
) dtw_accel_M00_AXIS_inst (
    // dtw
    .dtw_fifo_wren  (m00_axis_dtw_fifo_wren),
    .dtw_fifo_din   (m00_axis_dtw_fifo_din),
    .dtw_fifo_full  (m00_axis_dtw_fifo_full),

    // axis
    .M_AXIS_ACLK    (axis_clk),
    .M_AXIS_ARESETN (axis_aresetn),
    .M_AXIS_TVALID  (m00_axis_tvalid),
    .M_AXIS_TDATA   (m00_axis_tdata),
    .M_AXIS_TSTRB   (m00_axis_tstrb),
    .M_AXIS_TLAST   (m00_axis_tlast),
    .M_AXIS_TREADY  (m00_axis_tready)
);

// DTW core
dtw_core #(
    .dtw_dwidth (width),
    .axi_dwidth (C_AXIS_TDATA_WIDTH),
    .SQG_SIZE (SQG_SIZE)
) inst_dtw_core (
    // Main DTW signals
    .clk            (axis_clk),
    .rst            (s00_dtw_reset),
    .rs             (s00_dtw_rs),
    .ref_len        (s00_dtw_ref_len),
    .op_mode        (s00_dtw_mode),
    .busy           (s00_dtw_busy),
    .load_done      (s00_dtw_load_done),

    // DTW FIFO signals -> to the inside world! (S00_AXIS)
    .src_fifo_rden  (s00_axis_fifo_rden),
    .src_fifo_empty (s00_axis_fifo_empty),
    .src_fifo_data  (s00_axis_fifo_dout),

    // DTW FIFO signals -> to the outside world! (M00_AXIS)
    .sink_fifo_wren (m00_axis_dtw_fifo_wren),
    .sink_fifo_full (m00_axis_dtw_fifo_full),
    .sink_fifo_data (m00_axis_dtw_fifo_din),
    
    // Debug
    .dbg_addrW_ref (w_dbg_addrW_ref),
    .dbg_ref_dout   (w_dbg_ref_dout),
    .dbg_curr_state (w_dbg_curr_state)
);

endmodule
