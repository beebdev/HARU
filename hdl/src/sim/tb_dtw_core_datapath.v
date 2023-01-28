`timescale 1ns / 1ps

module tb_dtw_core_datapath ();

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
    reg running;
    reg load_squiggle;
    reg [WORD_LEN-1:0] squiggle_word;
    reg [WORD_LEN-1:0] reference_word;
    reg [31:0] reference_len;
    wire [WORD_LEN-1:0] best_score;
    wire [31:0] best_position;
    wire done;
    
    // DUT instantiation
    dtw_core_datapath #(
        .WORD_LEN (WORD_LEN),
        .SQG_LEN  (SQG_LEN)
    ) dut (
        .clk            (clk),
        .rst            (rst),
        .running        (running),
        
        .load_squiggle  (load_squiggle),
        .squiggle_word  (squiggle_word),
        .reference_word (reference_word),
        .reference_len  (reference_len),
        .best_score     (best_score),
        .best_position  (best_position),
        .done           (done)
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
        // Query and reference data (abs windows path)
        q_fd = $fopen("D:/Projects/haru-project/HARU/tb_scripts/squiggle_list.dat", "r");
        r_fd = $fopen("D:/Projects/haru-project/HARU/tb_scripts/reference_list.dat", "r");

        // Generate inputs
        i = 0; val = 0;
        running = 0;
        load_squiggle = 1;
        squiggle_word = 0;
        reference_word = 0;
        reference_len = REF_LEN;

        #3
        
        // Load squiggle
        test_case = "load q";
        for (i = 0; i < SQG_LEN; i = i + 1) begin
            if ($feof(q_fd)) begin
                $display("End of file");
                $finish;
            end
            ret = $fscanf(q_fd, "%d\n", val);
            $display("qval = %d", val);
            squiggle_word = val[WORD_LEN-1:0];
            #2
            squiggle_word = 1;
        end
        load_squiggle = 0;
        squiggle_word = 0;
        #4
        
        // start search
        test_case = "Start search";
        running = 1;
        for (i = 0; i < REF_LEN; i = i + 1) begin
            if ($feof(r_fd)) begin
                $display("End of file");
                $finish;
            end
            ret = $fscanf(r_fd, "%d\n", val);
            $display("rval = %d", val);
            #2
            reference_word = val[WORD_LEN-1:0];
            // load_squiggle = 0;
        end
        
        // end
        #2
        test_case = "";
        
        $fclose(q_fd);
        $fclose(r_fd);
    end

endmodule


