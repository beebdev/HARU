`timescale 1ns / 1ps

module tb_zynq_dtw;

    integer i, ret;
    integer val;
    integer q_fd, r_fd;
    localparam S_AXI_ADDR_WIDTH = 16;
    localparam S_AXI_DATA_WIDTH = 32;
    localparam S_AXIS_DATA_WIDTH = 32;
    localparam WORD_LEN = 16;
    localparam SQG_LEN = 5;
    localparam REF_LEN = 25;

    
    localparam REG_0_CONTROL = 0;
    localparam REG_1_STATUS = 1;
    localparam REG_2_VERSION = 2;
    localparam REG_3_KEY = 3;
    localparam REG_4_REF_LEN = 4;
    localparam REG_5_BEST_SCORE = 5;
    localparam REG_6_BEST_POSITION = 6;

    // Clock and reset signals
    reg clk;
    reg rst;
    reg[8*20:1] test_case;
    reg[8*25:1] tag;

    
    // AXI Lite signals
    reg                             axi_lite_awvalid;
    reg [S_AXI_ADDR_WIDTH-1:0]      axi_lite_awaddr;
    wire                            axi_lite_awready;

    reg                             axi_lite_wvalid;
    wire                            axi_lite_wready;
    reg [S_AXI_DATA_WIDTH-1:0]      axi_lite_wdata;

    wire                            axi_lite_bvalid;
    reg                             axi_lite_bready;
    wire [1:0]                      axi_lite_bresp;

    reg                             axi_lite_arvalid;
    wire                            axi_lite_arready;
    reg  [S_AXI_ADDR_WIDTH-1:0]     axi_lite_araddr;

    wire                            axi_lite_rvalid;
    reg                            axi_lite_rready;
    wire [1:0]                      axi_lite_rresp;
    wire [S_AXI_DATA_WIDTH-1:0]     axi_lite_rdata;

    // AXI Stream signals
    reg                             axi_stream_tuser;
    reg                             axi_stream_tvalid;
    wire                            axi_stream_tready;
    reg                             axi_stream_tlast;
    wire [S_AXIS_DATA_WIDTH-1:0]    axi_stream_tdata;
    

    /* DUT instantiation */
    zynq_dtw #(
        .S_AXI_ADDR_WIDTH (S_AXI_ADDR_WIDTH),
        .S_AXI_DATA_WIDTH (S_AXI_DATA_WIDTH),
        .S_AXI_RST_INVERT (0),
        .S_AXIS_DATA_WIDTH (S_AXIS_DATA_WIDTH),
        .S_AXIS_RST_INVERT (0),
        .SRC_FIFO_DEPTH (4),
        .WORD_LEN (WORD_LEN),
        .SQG_LEN (SQG_LEN)
    ) dut (
        .S_AXI_clk (clk),
        .S_AXI_rst (rst),
        
        .S_AXI_awvalid (axi_lite_awvalid),
        .S_AXI_awaddr (axi_lite_awaddr),
        .S_AXI_awready (axi_lite_awready),

        .S_AXI_wvalid (axi_lite_wvalid),
        .S_AXI_wdata (axi_lite_wdata),
        .S_AXI_wready (axi_lite_wready),

        .S_AXI_bvalid (axi_lite_bvalid),
        .S_AXI_bresp (axi_lite_bresp),
        .S_AXI_bready (axi_lite_bready),

        .S_AXI_arvalid (axi_lite_arvalid),
        .S_AXI_araddr (axi_lite_araddr),
        .S_AXI_arready (axi_lite_arready),

        .S_AXI_rvalid (axi_lite_rvalid),
        .S_AXI_rdata (axi_lite_rdata),
        .S_AXI_rresp (axi_lite_rresp),
        .S_AXI_rready (axi_lite_rready),

        .S_AXIS_clk (clk),
        .S_AXIS_rst (rst),
        .S_AXIS_tuser(axi_stream_tuser),
        .S_AXIS_tvalid (axi_stream_tvalid),
        .S_AXIS_tready (axi_stream_tready),
        .S_AXIS_tlast (axi_stream_tlast),
        .S_AXIS_tdata (axi_stream_tdata)
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
    //=========================================================================
    // AXI stream signals
    reg [31:0] data = 10;

    // Generate data
    always @(posedge clk) begin
        if (axi_stream_tvalid && axi_stream_tready) begin
            data <= data + 1;
        end
    end
    assign axi_stream_tdata = data; // Wait for tready signal

    // initial begin
    //     repeat (100) begin
    //         #10;
    //         if (axi_stream_tready) begin
    //         break;
    //         end
    //     end

    //     // Check the received data
    //     if (axi_stream_tlast && (data != rdata)) begin
    //         $display("Received data does not match expected data");
    //         $finish;
    //     end
    // end
    //=========================================================================

    /* Stimulus */
    initial begin
        // Init
        axi_lite_bready <= 1'b1;
        axi_stream_tuser <= 1'b0;
        axi_stream_tvalid <= 1'b0;
        axi_stream_tlast <= 1'b0;

        test_case = "";
        tag = "";
        // Query and reference data (abs windows path)
        q_fd = $fopen("D:/Projects/haru-project/HARU/tb_scripts/squiggle_list.dat", "r");
        r_fd = $fopen("D:/Projects/haru-project/HARU/tb_scripts/reference_list.dat", "r");

        // ================================================================
        test_case = "Version read"; tag = "";
        // ================================================================
        // Read from the version register
        axi_lite_arvalid <= 1'b1;
        axi_lite_araddr[31:1] <= REG_2_VERSION;
        axi_lite_araddr[0] <= 0;
        axi_lite_rready <= 1'b1;
        
        // Wait for read response
        repeat (100) begin
            #2;
            if (axi_lite_rvalid) begin
                break;
            end
        end
        if (axi_lite_rdata != 32'h00020000) begin
            $display("[ERROR] Version read mismatch, got value %h", axi_lite_rdata);
            $finish;
        end

        #4;
        // ================================================================
        test_case = "Key read"; tag = "";
        // ================================================================
        // Read from the version register
        axi_lite_arvalid <= 1'b1;
        axi_lite_araddr[31:1] <= REG_3_KEY;
        axi_lite_araddr[0] <= 0;
        axi_lite_rready <= 1'b1;
        
        // Wait for read response
        repeat (100) begin
            #2;
            if (axi_lite_rvalid) begin
                break;
            end
        end
        if (axi_lite_rdata != 32'h0ca7cafe) begin
            $display("[ERROR] Key read mismatch, got value %h", axi_lite_rdata);
            $finish;
        end

        #4;
        // ================================================================
        test_case = "ref_len write";tag = "";
        // ================================================================
        tag = "read";
        // Read from the version register
        axi_lite_arvalid <= 1'b1;
        axi_lite_araddr[31:1] <= REG_4_REF_LEN;
        axi_lite_araddr[0] <= 0;
        axi_lite_rready <= 1'b1;
        
        // Wait for read response
        repeat (100) begin
            #2;
            if (axi_lite_rvalid) begin
                break;
            end
        end
        if (axi_lite_rdata != 32'h0) begin
            $display("[ERROR] Reflen read mismatch, got value %h", axi_lite_rdata);
            $finish;
        end
        
        tag = "write";
        // Write to reference length register
        axi_lite_awvalid <= 1'b1;
        axi_lite_awaddr[31:1] <= REG_4_REF_LEN;
        axi_lite_awaddr[0] <= 0;
        axi_lite_wvalid <= 1'b1;
        axi_lite_wdata <= REF_LEN;

        // Wait for write response
        axi_lite_bready <= 1'b1;
        repeat (100) begin
            #2;
            if (axi_lite_bvalid) begin
                axi_lite_awvalid <= 1'b0;
                axi_lite_wvalid <= 1'b0;
                break;
            end
        end

        #2;
        tag = "read";
        // Read from the version register
        axi_lite_arvalid <= 1'b1;
        axi_lite_araddr[31:1] <= REG_4_REF_LEN;
        axi_lite_araddr[0] <= 0;
        axi_lite_rready <= 1'b1;
        
        // Wait for read response
        repeat (100) begin
            #2;
            if (axi_lite_rvalid) begin
                break;
            end
        end
        if (axi_lite_rdata != REF_LEN) begin
            $display("[ERROR] Reflen read mismatch, got value %h", axi_lite_rdata);
            $finish;
        end

        // ================================================================
        test_case = "start search"; tag = "";
        // ================================================================
        tag = "rst";

        /* Reset the core */
        // Write to reference length register
        axi_lite_awvalid <= 1'b1;
        axi_lite_awaddr[31:1] <= REG_0_CONTROL;
        axi_lite_awaddr[0] <= 0;
        axi_lite_wvalid <= 1'b1;
        axi_lite_wdata <= 1;

        // Wait for write response
        axi_lite_bready <= 1'b1;
        repeat (100) begin
            #2;
            if (axi_lite_bvalid) begin
                axi_lite_awvalid <= 1'b0;
                axi_lite_wvalid <= 1'b0;
                break;
            end
        end
        // Write to reference length register
        axi_lite_awvalid <= 1'b1;
        axi_lite_awaddr[31:1] <= REG_0_CONTROL;
        axi_lite_awaddr[0] <= 0;
        axi_lite_wvalid <= 1'b1;
        axi_lite_wdata <= 0;

        // Wait for write response
        axi_lite_bready <= 1'b1;
        repeat (100) begin
            #2;
            if (axi_lite_bvalid) begin
                axi_lite_awvalid <= 1'b0;
                axi_lite_wvalid <= 1'b0;
                break;
            end
        end

        /* start the core */
        tag = "rs";
        // Write to reference length register
        axi_lite_awvalid <= 1'b1;
        axi_lite_awaddr[31:1] <= REG_0_CONTROL;
        axi_lite_awaddr[0] <= 0;
        axi_lite_wvalid <= 1'b1;
        axi_lite_wdata <= 2;

        // Wait for write response
        axi_lite_bready <= 1'b1;
        repeat (100) begin
            #2;
            if (axi_lite_bvalid) begin
                axi_lite_awvalid <= 1'b0;
                axi_lite_wvalid <= 1'b0;
                break;
            end
        end
        
        tag = "stream-q";
        axi_stream_tvalid <= 1'b1;


        // ================================================================
        #1000
        test_case = "";
        // ================================================================
    end

endmodule