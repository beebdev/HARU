module dgn_memreference #(
    parameter width = 16,
    parameter ptrWid = 15,
    parameter depth = 2**ptrWid
)(
    input clk,
    input [ptrWid-1:0] addrR,
    input [ptrWid-1:0] addrW,
    input wren,
    input [width-1 : 0] datain,
    output reg [width-1 : 0] dataout
);

(* ram_style = "block" *) 
reg [width-1:0] MEM [0:depth-1];

initial begin
    $readmemb("D:/UNSW/Thesis/dtw_core/bram_init/reference.txt", MEM);
end

assign dataout <= MEM[addrR];
always @(posedge clk) begin
	if (wren) begin
		MEM[addrW] <= datain;
    end
end

endmodule