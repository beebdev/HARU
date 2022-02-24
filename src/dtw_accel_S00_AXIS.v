
`timescale 1 ns / 1 ps

module dtw_accel_S00_AXIS #(
	/* HARU parameters */

	/* AXI4Stream sink: Data Width */
	parameter integer C_S_AXIS_TDATA_WIDTH	= 32
)(
	/* HARU ports */
	input wire dtw_fifo_rden,
	output reg [(C_S_AXIS_TDATA_WIDTH/4)-1:0] dtw_fifo_dout,
	output wire dtw_fifo_empty,

	/* AXI stream ports */
	input wire S_AXIS_ACLK,										// AXI4Stream sink: Clock
	input wire S_AXIS_ARESETN,									// AXI4Stream sink: Reset
	output wire S_AXIS_TREADY,									// Ready to accept data in
	input wire [C_S_AXIS_TDATA_WIDTH-1 : 0] S_AXIS_TDATA,		// Data in
	input wire [(C_S_AXIS_TDATA_WIDTH/8)-1 : 0] S_AXIS_TSTRB,	// Byte qualifier -> 1-data byte, 0-position byte
	input wire S_AXIS_TLAST,									// Indicates boundary of last packet
	input wire S_AXIS_TVALID									// Data is in valid
);

// Returns an integer which has the value of the ceiling of the log base 2.
function integer clogb2 (input integer bit_depth);
begin
	for (clogb2 = 0; bit_depth > 0; clogb2 = clogb2 + 1) begin 
		bit_depth = bit_depth >> 1;
	end
end
endfunction

// Total number of input data.
localparam NUMBER_OF_INPUT_WORDS = 8;
// bit_num gives the minimum number of bits needed to address 'NUMBER_OF_INPUT_WORDS' size of FIFO.
localparam bit_num  = clogb2(NUMBER_OF_INPUT_WORDS-1);
// Define the states of state machine
// The control state machine oversees the writing of input streaming data to the FIFO,
// and outputs the streaming data from the FIFO
parameter [1:0] IDLE = 1'b0,        // This is the initial/idle state 
				WRITE_FIFO  = 1'b1; // In this state FIFO is written with the
									// input stream data S_AXIS_TDATA

wire axis_tready;
reg mst_exec_state;					// State variable
genvar byte_index;     				// FIFO implementation signals

/* FIFO */
reg [(C_S_AXIS_TDATA_WIDTH/4)-1:0] fifo_data [0 : NUMBER_OF_INPUT_WORDS-1];
wire fifo_wren;						// FIFO write enable
wire fifo_rden;						// FIFO read enable
reg [bit_num-1:0] fifo_data_count;				// FIFO data count
reg fifo_full_flag;					// FIFO full flag
reg fifo_empty_flag;				// FIFO empty flag
reg [bit_num-1:0] write_pointer;	// FIFO write pointer
reg [bit_num-1:0] read_pointer;		// FIFO read pointer
reg writes_done;					// sink has accepted all the streaming data and stored in FIFO

// I/O Connections assignments
assign S_AXIS_TREADY = axis_tready;

/* Control state machine implementation */
always @(posedge S_AXIS_ACLK) begin  
	if (!S_AXIS_ARESETN) begin // Synchronous reset (active low)
		mst_exec_state <= IDLE;
	end else begin
		case (mst_exec_state)
		IDLE: 
			// The sink starts accepting tdata when 
			// there tvalid is asserted to mark the
			// presence of valid streaming data 
			if (S_AXIS_TVALID) begin
				mst_exec_state <= WRITE_FIFO;
			end else begin
				mst_exec_state <= IDLE;
			end
		WRITE_FIFO: 
			// When the sink has accepted all the streaming input data,
			// the interface swiches functionality to a streaming master
			if (writes_done) begin
				mst_exec_state <= IDLE;
			end else begin
				// The sink accepts and stores tdata into FIFO
				mst_exec_state <= WRITE_FIFO;
			end
		endcase
	end
end

// AXI Streaming Sink 
// 
// The example design sink is always ready to accept the S_AXIS_TDATA  until
// the FIFO is not filled with NUMBER_OF_INPUT_WORDS number of input words.
assign axis_tready = ((mst_exec_state == WRITE_FIFO) && !fifo_full_flag);

always@(posedge S_AXIS_ACLK) begin
	if(!S_AXIS_ARESETN) begin
		write_pointer <= 0;
		writes_done <= 1'b0;
	end else begin
		if (write_pointer <= NUMBER_OF_INPUT_WORDS-1) begin
			if (fifo_wren) begin
				// write pointer is incremented after every write to the FIFO
				// when FIFO write signal is enabled.
				write_pointer <= write_pointer + 1;
				writes_done <= 1'b0;
			end if ((write_pointer == NUMBER_OF_INPUT_WORDS - 1) || S_AXIS_TLAST) begin
				// reads_done is asserted when NUMBER_OF_INPUT_WORDS numbers of streaming data 
				// has been written to the FIFO which is also marked by S_AXIS_TLAST(kept for optional usage).
				writes_done <= 1'b1;
			end
		end
	end
end

/* FIFO */

// port assignment
assign dtw_fifo_empty = fifo_empty_flag;

// inner assignment
always @(fifo_data_count) begin
    fifo_full_flag = (fifo_data_count == NUMBER_OF_INPUT_WORDS);
    fifo_empty_flag = (fifo_data_count == 0);
end
 
assign fifo_wren = S_AXIS_TVALID && axis_tready;			// src -> axis
assign fifo_rden = dtw_fifo_rden  && !fifo_empty_flag;		// sink -> dtw

always @(posedge S_AXIS_ACLK) begin
	if (!S_AXIS_ARESETN) begin
		fifo_data_count <= 0;
		write_pointer <= 0;
		read_pointer <= 0;
	end else begin
		// Manage fifo data count
		if (fifo_wren && !fifo_rden) begin
			fifo_data_count <= fifo_data_count + 1;
		end else if (!fifo_wren && fifo_rden) begin
			fifo_data_count <= fifo_data_count - 1;
		end

		// Write index
		if (fifo_wren) begin
			fifo_data[write_pointer] <= S_AXIS_TDATA[(byte_index*8+7) -: 8];
			if (write_pointer == NUMBER_OF_INPUT_WORDS - 1) begin
				write_pointer <= 0;
			end else begin
				write_pointer <= write_pointer + 1;
			end
		end

		// Read index
		if (fifo_rden) begin
            dtw_fifo_dout = fifo_data[read_pointer];
			if (read_pointer == NUMBER_OF_INPUT_WORDS - 1) begin
				read_pointer <= 0;
			end else begin
				read_pointer <= read_pointer + 1;
			end
		end
	end
end

endmodule