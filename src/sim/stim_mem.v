module stim_mem #(
    parameter width = 16,
    parameter ptrWid = 15,
    parameter depth = 2**ptrWid,
    parameter init_mode = 1
)(
    input clk,
    input [ptrWid-1:0] addrR,
    input [ptrWid-1:0] addrW,
    input wren,
    input [width-1 : 0] datain,
    output [width-1 : 0] dataout
);

(* ram_style = "block" *) 
reg [width-1:0] MEM [0:depth-1];
reg [depth-1:0] i;

initial begin
    if (init_mode == 0) begin
        $readmemb("D:/UNSW/Thesis/dtw_core/bram_init/query_with_id.txt", MEM);
    end else if (init_mode == 1) begin
        $readmemb("D:/UNSW/Thesis/dtw_core/bram_init/reference.txt", MEM);
    end else begin
        for (i=0; i<depth; i=i+1) begin
            MEM[i] = 0;
        end
    end
end

assign dataout = MEM[addrR];
always @(posedge clk) begin
	if (wren) begin
		MEM[addrW] <= datain;
    end
end

endmodule