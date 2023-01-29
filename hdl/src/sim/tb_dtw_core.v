`timescale 1ns / 1ps

module tb_dtw_core ();

    localparam WORD_LEN = 16;
    localparam SQG_LEN = 5;
    localparam REF_LEN = 25;
    integer i, ret;
    integer val;
    integer q_fd, r_fd;

    // Clock and reset signals
    reg clk;
    reg rst;
    reg[8*20:1] test_case;
    
    // DUT inputs and outputs
    reg                     rs;
    wire                    running;

    wire                    src_fifo_clear;
    wire                    src_fifo_rden;
    reg                     src_fifo_empty;
    wire     [31:0]          src_fifo_data;

    reg     [31:0]          reference_len;
    wire    [WORD_LEN-1:0]  best_score;
    wire    [31:0]          best_position;
    wire                    done;
    
    /* DUT instantiation */
    dtw_core #(
        .WORD_LEN (WORD_LEN),
        .AXIS_WIDTH (32),
        .SQG_LEN  (SQG_LEN)
    ) dut (
        .clk            (clk),
        .rst            (rst),
        .rs             (rs),
        .running        (running),

        .src_fifo_clear (src_fifo_clear),
        .src_fifo_rden  (src_fifo_rden),
        .src_fifo_empty (src_fifo_empty),
        .src_fifo_data  (src_fifo_data),

        .reference_len  (reference_len),
        .best_score     (best_score),
        .best_position  (best_position),
        .done           (done)
    );
    
    /* Generate the clock */
    initial begin
        clk = 1'b0;
        forever #1 clk = ~clk;
    end

    /* Generate the reset signal */
    initial begin
        rst = 1'b1;
        #2
        rst = 1'b0;
    end
    
    assign src_fifo_data = val[WORD_LEN-1:0];
    
    /* Stimulus */
    initial begin
        test_case = "";
        // Query and reference data (abs windows path)
        q_fd = $fopen("D:/Projects/haru-project/HARU/tb_scripts/squiggle_list.dat", "r");
        r_fd = $fopen("D:/Projects/haru-project/HARU/tb_scripts/reference_list.dat", "r");

        // Generate inputs
        i = 0; val = 0;

        rs = 0;
        src_fifo_empty = 0;
        // src_fifo_data = 123;
        reference_len = REF_LEN;

        #5

        // Start dtw_core
        // test_case = "start rs";
        rs = 1;
        #2
        rs = 0;

        // Load squiggle
        test_case = "load q";
        for (i = 0; i < SQG_LEN; i = i + 1) begin
            if ($feof(q_fd)) begin
                $display("End of file");
                $finish;
            end
            ret = $fscanf(q_fd, "%d\n", val);
            // src_fifo_data = val[WORD_LEN-1:0];
            #2
            rs = 0;
        end
        
        // start search
        test_case = "Start search";
        for (i = 0; i < REF_LEN; i = i + 1) begin
            // if (src_fifo_rden) begin
            if ($feof(r_fd)) begin
                $display("End of file");
                $finish;
            end
            ret = $fscanf(r_fd, "%d\n", val);

            #2
            rs = 0;
            // src_fifo_data = val[WORD_LEN-1:0];
        end
        #30


        
        rs = 1;
        #2
        rs = 0;
        #1000
        
        // end
        #2
        test_case = "";
        #1000
        
        $fclose(q_fd);
        $fclose(r_fd);
    end

endmodule


