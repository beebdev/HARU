`timescale 1 ns / 1 ps

module dtw_accel_M00_AXIS #(
	/* ===============================
	* HARU parameters
	* =============================== */

	/* ===============================
	* M AXIs paramters
	* =============================== */
	// Width of S_AXIS address bus. The slave accepts the read and write addresses of width C_M_AXIS_TDATA_WIDTH.
	parameter integer C_M_AXIS_TDATA_WIDTH = 32,
	// Start count is the number of clock cycles the master will wait before initiating/issuing any transaction.
	parameter integer C_M_START_COUNT = 32 // keep this as default
)(
	/* ===============================
	* HARU ports
	* =============================== */
	input wire dtw_fifo_wren,
	input wire [C_M_AXIS_TDATA_WIDTH-1:0] dtw_fifo_din,
	output wire dtw_fifo_full,

	/* ===============================
	* M AXIs ports
	* =============================== */
	input wire  M_AXIS_ACLK,
	// Reset
	input wire  M_AXIS_ARESETN,
	// Master Stream Ports. TVALID indicates that the master is driving a valid transfer, A transfer takes place when both TVALID and TREADY are asserted.
	output wire  M_AXIS_TVALID,
	// TDATA is the primary payload that is used to provide the data that is passing across the interface from the master.
	output wire [C_M_AXIS_TDATA_WIDTH-1 : 0] M_AXIS_TDATA,
	// TSTRB is the byte qualifier that indicates whether the content of the associated byte of TDATA is processed as a data byte or a position byte.
	output wire [(C_M_AXIS_TDATA_WIDTH/8)-1 : 0] M_AXIS_TSTRB,
	// TLAST indicates the boundary of a packet.
	output wire  M_AXIS_TLAST,
	// TREADY indicates that the slave can accept a transfer in the current cycle.
	input wire  M_AXIS_TREADY
);

/* ===============================
 * local parameters, functions
 * =============================== */

// Total number of output data
localparam NUMBER_OF_OUTPUT_WORDS = 8;

// Returns an integer which has the value of the ceiling of the log base 2.
function integer clogb2 (input integer bit_depth);
begin
	for(clogb2 = 0; bit_depth > 0; clogb2 = clogb2 + 1)
		bit_depth = bit_depth >> 1;
end
endfunction

// WAIT_COUNT_BITS is the width of the wait counter.
localparam integer WAIT_COUNT_BITS = clogb2(C_M_START_COUNT-1);

// bit_num gives the minimum number of bits needed to address 'depth' size of FIFO.
localparam bit_num = clogb2(NUMBER_OF_OUTPUT_WORDS);

// Define the states of state machine
// The control state machine oversees the writing of input streaming data to the FIFO,
// and outputs the streaming data from the FIFO
parameter [1:0] IDLE = 2'b00,        // This is the initial/idle state
				INIT_COUNTER  = 2'b01, // This state initializes the counter, once
								// the counter reaches C_M_START_COUNT count,
								// the state machine changes state to SEND_STREAM
				SEND_STREAM   = 2'b10; // In this state the
										// stream data is output through M_AXIS_TDATA

/* ===============================
 * Reg and wires
 * =============================== */
/* State variable */
reg [1:0] mst_exec_state;

/* AXI Stream internal signals */
// wait counter. The master waits for the user defined number of clock cycles before initiating a transfer.
reg [WAIT_COUNT_BITS-1 : 0] count;
// streaming data valid
wire axis_tvalid;
// streaming data valid delayed by one clock cycle
reg axis_tvalid_delay;
// Last of the streaming data
wire axis_tlast;
// Last of the streaming data delayed by one clock cycle
reg axis_tlast_delay;
// FIFO implementation signals
reg [C_M_AXIS_TDATA_WIDTH-1 : 0] stream_data_out;
// The master has issued all the streaming data stored in FIFO
reg tx_done;

/* Other signals */
wire fifo_rden;
wire fifo_wren;
reg [bit_num-1:0] fifo_data_count;
reg [bit_num-1:0] write_pointer;	// FIFO write pointer
reg [bit_num-1:0] read_pointer;		// FIFO read pointer

/* ===============================
 * I/O Connections assignments
 * =============================== */
assign M_AXIS_TVALID = axis_tvalid_delay;
assign M_AXIS_TDATA	= stream_data_out;
assign M_AXIS_TLAST	= axis_tlast_delay;
assign M_AXIS_TSTRB	= {(C_M_AXIS_TDATA_WIDTH/8){1'b1}};

/* ===============================
 * Control state machine implementation
 * =============================== */
always @(posedge M_AXIS_ACLK) begin
	if (!M_AXIS_ARESETN) begin
		mst_exec_state <= IDLE; // Synchronous reset (active low)
		count <= 0;
	end else begin
		case (mst_exec_state)
		IDLE:
			// The slave starts accepting tdata when
			// there tvalid is asserted to mark the
			// presence of valid streaming data
			mst_exec_state  <= INIT_COUNTER;

		INIT_COUNTER:
			// The slave starts accepting tdata when
			// there tvalid is asserted to mark the
			// presence of valid streaming data
			if ( count == C_M_START_COUNT - 1 ) begin
				mst_exec_state  <= SEND_STREAM;
			end else begin
				count <= count + 1;
				mst_exec_state  <= INIT_COUNTER;
			end

		SEND_STREAM:
			// The example design streaming master functionality starts
			// when the master drives output tdata from the FIFO and the slave
			// has finished storing the S_AXIS_TDATA
			if (tx_done) begin
				mst_exec_state <= IDLE;
			end else begin
				mst_exec_state <= SEND_STREAM;
			end
		endcase
	end
end

//tvalid generation
//axis_tvalid is asserted when the control state machine's state is SEND_STREAM and
//number of output streaming data is less than the NUMBER_OF_OUTPUT_WORDS.
assign axis_tvalid = ((mst_exec_state == SEND_STREAM) && (fifo_data_count > 0));

// AXI tlast generation
// axis_tlast is asserted number of output streaming data is NUMBER_OF_OUTPUT_WORDS-1
// (0 to NUMBER_OF_OUTPUT_WORDS-1)
assign axis_tlast = (read_pointer == NUMBER_OF_OUTPUT_WORDS-1);

/* ===============================
 * axis_tvalid and axis_tlast delay
 * =============================== */
// Delay the axis_tvalid and axis_tlast signal by one clock cycle
// to match the latency of M_AXIS_TDATA
always @(posedge M_AXIS_ACLK) begin
	if (!M_AXIS_ARESETN) begin
		axis_tvalid_delay <= 1'b0;
		axis_tlast_delay <= 1'b0;
	end else begin
		axis_tvalid_delay <= axis_tvalid;
		axis_tlast_delay <= axis_tlast;
	end
end


/* ===============================
 * FIFO Implementation
 * =============================== */
// FIFO read enable generation
assign fifo_rden = M_AXIS_TREADY && axis_tvalid;
assign fifo_wren = dtw_fifo_wren && (fifo_data_count < NUMBER_OF_OUTPUT_WORDS);
reg [C_M_AXIS_TDATA_WIDTH-1 : 0] fifo_data [0:NUMBER_OF_OUTPUT_WORDS-1];

// logic for FIFO
always @(posedge M_AXIS_ACLK) begin
	if (!M_AXIS_ARESETN) begin // sync active low reset
		fifo_data_count <= 0;
		write_pointer <= 0;
		read_pointer <= 0;
		tx_done <= 1'b0;
	end else begin
		// Manage fifo data count
		if (fifo_wren && !fifo_rden) begin
			fifo_data_count <= fifo_data_count + 1;
		end else if (!fifo_wren && fifo_rden) begin
			fifo_data_count <= fifo_data_count - 1;
		end

		// Manage write pointer
		if (fifo_wren) begin
			fifo_data[write_pointer] <= dtw_fifo_din;
			if (write_pointer == NUMBER_OF_OUTPUT_WORDS - 1) begin
				write_pointer <= 0;
			end else begin
				write_pointer <= write_pointer + 1;
			end
		end

		// Manage read pointer
		if (fifo_rden) begin
			stream_data_out <= fifo_data[read_pointer];
			if (read_pointer == NUMBER_OF_OUTPUT_WORDS - 1) begin
				read_pointer <= 0;
				tx_done <= 1'b1;
			end else begin
				read_pointer <= read_pointer + 1;
				tx_done <= 1'b0;
			end
		end
	end
end

endmodule
