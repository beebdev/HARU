`timescale 1ns / 1ps

module tb_dtw_core_pe ();

    localparam WORD_LEN = 16;

    // Clock and reset signals
    reg clk;
    reg rst;
    
    // DUT inputs and outputs
    reg running;
    
    reg[8*5:1] test_case;
    reg [WORD_LEN-1:0] x;
    reg [WORD_LEN-1:0] y;
    reg [WORD_LEN-1:0] N;
    reg [WORD_LEN-1:0] W;
    reg [WORD_LEN-1:0] NW;
    wire [WORD_LEN-1:0] score;
    wire [WORD_LEN-1:0] yp;
    
    // DUT instantiation
    dtw_core_pe dut (
        .clk (clk),
        .rst (rst),
        .running (running),
        
        .x (x),
        .y (y),
        .N (N),
        .W (W),
        .NW (NW),
        .score (score),
        .yp (yp)
    );
    
    // Generate the clock
    initial begin
        clk = 1'b0;
        forever #1 clk = ~clk;
    end

    // Generate the reset signal
    initial begin
        rst = 1'b1;
        #2
        rst = 1'b0;
    end
    
    
    // Stimulus
    initial begin
        $monitor("time=%3d, x=%d, y=%d, n=%d, w=%d, nw=%d; score=%d, yp=%d\n",
                $time, x, y, N, W, NW, score, yp);
                
        // Generate inputs
        running = 0;
        #2
        running = 1;
        #1
        
        // Case 1-1: X > Y; N smallest
        test_case = "1-1";
        x = 16'd30;
        y = 16'd20;
        N = 16'd0;
        W = 16'd1;
        NW = 16'd1;
        #1
        
        // Case 1-2: X > Y; W smallest
        test_case = "1-2";
        x = 16'd30;
        y = 16'd20;
        N = 16'd1;
        W = 16'd0;
        NW = 16'd1;
        #1
        
        // Case 1-3: X > Y; NW smallest
        test_case = "1-3";
        x = 16'd30;
        y = 16'd20;
        N = 16'd3;
        W = 16'd2;
        NW = 16'd1;
        #1
  
        
        // Case 2-1: Y > X; N smallest
        test_case = "2-1";
        x = 16'd20;
        y = 16'd30;
        N = 16'd0;
        W = 16'd1;
        NW = 16'd1;
        #1
        
        // Case 2-2: Y > X; W smallest
        test_case = "2-2";
        x = 16'd20;
        y = 16'd30;
        N = 16'd1;
        W = 16'd0;
        NW = 16'd1;
        #1
        
        // Case 2-3: Y > X; NW smallest
        test_case = "2-3";
        x = 16'd20;
        y = 16'd30;
        N = 16'd3;
        W = 16'd2;
        NW = 16'd1;

        // Case 3-1: X -Inf, Y Inf; N=W=INF, NW=1
        test_case = "3-1";
        x = -(16'd0 - 16'd1);
        y = (16'd0 - 16'd1);
        N = (16'd0 - 16'd1);
        W = (16'd0 - 16'd1);
        NW = 16'd1;
        // end
        #1
        test_case = "";
        x = 16'd0;
        y = 16'd0;
        N = 16'd0;
        W = 16'd0;
        NW = 16'd0;
         
    end

endmodule


