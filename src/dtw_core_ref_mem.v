module dtw_core_ref_mem #(
    parameter width     = 16,
    parameter ptrWid    = 15,
    parameter depth     = 2**ptrWid,
    parameter initalize = 1
)(
    input   wire                clk,

    // Write
    input   wire                wren,
    input   wire [ptrWid-1:0]   addrW,
    input   wire [width-1 : 0]  datain,

    // Read
    input   wire [ptrWid-1:0]   addrR,
    output  reg  [width-1 : 0]  dataout
);

/* ===============================
 * registes/wires
 * =============================== */
(* ram_style = "block" *) 
reg [width-1:0] MEM [0:depth-1];
reg [depth-1:0] i;

/* ===============================
 * initialisation
 * =============================== */
initial begin
    if (initalize) begin
        $readmemb("data/reference.txt", MEM);
    end else begin
        for (i = 0; i < depth; i = i + 1) begin
            MEM[i] = 0;
        end
    end
end

/* ===============================
 * synchronous logic
 * =============================== */
always @(posedge clk) begin
    dataout <= MEM[addrR];
	if (wren) begin
		MEM[addrW] <= datain;
    end
end

endmodule