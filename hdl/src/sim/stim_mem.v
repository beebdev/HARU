/* MIT License

Copyright (c) 2022 Po Jui Shih
Copyright (c) 2022 Hassaan Saadat
Copyright (c) 2022 Sri Parameswaran
Copyright (c) 2022 Hasindu Gamaarachchi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

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