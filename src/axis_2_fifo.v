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
 * Author:
 * Description:
 *
 * Changes:     Who?    What?
 *  XX/XX/XXXX  XXX     XXXX
 */

`timescale 1ps / 1ps
`default_nettype none

module axis_2_fifo_adapter #(
  parameter AXIS_DATA_WIDTH     = 32,
  parameter FIFO_DATA_WIDTH     = AXIS_DATA_WIDTH + 1 + 1 //Space for data, user, last
)(
  input  wire                             i_axis_tuser,
  input  wire                             i_axis_tvalid,
  output wire                             o_axis_tready,
  input  wire                             i_axis_tlast,
  input  wire     [AXIS_DATA_WIDTH - 1:0] i_axis_tdata,

  output wire     [FIFO_DATA_WIDTH - 1:0] o_fifo_data,
  output wire                             o_fifo_w_stb,
  input  wire                             i_fifo_not_full
);
//local parameters
//registes/wires
//submodules
//asynchronous logic

assign      o_fifo_data              =  {i_axis_tuser, i_axis_tlast, i_axis_tdata};
assign      o_axis_tready            = i_fifo_not_full;
assign      o_fifo_w_stb             = i_axis_tvalid & o_axis_tready & i_fifo_not_full;

//synchronous logic
endmodule
