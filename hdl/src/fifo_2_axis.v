/*
Distributed under the MIT license.
Copyright (c) 2021 Dave McCoy (dave.mccoy@cospandesign.com)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
 * Author: David McCoy (dave.mccoy@cospandesign.com)
 *         Po Jui Shih  (beebdev@gmail.com)
 * Description: Simple adapter to connect AXI stream slave interface to FIFO
 *
 * Changes:     Author         Description
 *  03/24/2017  David McCoy    Initial Commit
 *  03/31/2022  Po Jui Shih     Removed unused extra padding for tuser and tlast.
 *                             Also removed the redundant i_fifo_not_empty anding
 *                             for o_fifo_r_stb. Style changes for consistency.
 */

`timescale 1ps / 1ps

module fifo_2_axis_adapter #(
    parameter AXIS_DATA_WIDTH   = 32,
    parameter FIFO_DATA_WIDTH   = AXIS_DATA_WIDTH
)(
    input  wire [FIFO_DATA_WIDTH - 1: 0]    i_fifo_data,
    output wire                             o_fifo_r_stb,
    input  wire                             i_fifo_not_empty,
    input  wire                             i_fifo_last,

    output wire                             o_axis_tuser,
    output wire [AXIS_DATA_WIDTH - 1: 0]    o_axis_tdata,
    output wire                             o_axis_tvalid,
    input  wire                             i_axis_tready,
    output wire                             o_axis_tlast
);

/* ===============================
 * asynchronous logic
 * =============================== */
assign o_axis_tuser     = 0;
assign o_axis_tlast     = i_fifo_last;
assign o_axis_tdata     = i_fifo_data;
assign o_axis_tvalid    = i_fifo_not_empty;
assign o_fifo_r_stb     = i_axis_tready & o_axis_tvalid;

endmodule
