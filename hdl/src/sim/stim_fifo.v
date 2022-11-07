module stim_mem #(
    parameter width = 16,
    parameter ptrWid = 15,
    parameter depth = 2**ptrWid,
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
reg [depth-1:0] i;

initial begin
    for (i=0; i<depth; i=i+1) begin
        MEM[i] = 0;
    end
end
end

always @(posedge clk) begin
    dataout <= MEM[addrR];
	if (wren) begin
		MEM[addrW] <= datain;
    end
end

endmodule