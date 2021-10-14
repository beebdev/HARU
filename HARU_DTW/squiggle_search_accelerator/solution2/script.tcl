############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2020 Xilinx, Inc. All Rights Reserved.
############################################################
open_project squiggle_search_accelerator
set_top subsequence_search
add_files src/squiggle_search.cpp
add_files -tb src/squiggle_search_top.cpp -cflags "-Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"
open_solution "solution2"
set_part {xc7z020-clg484-1}
create_clock -period 10 -name default
source "./squiggle_search_accelerator/solution2/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -format ip_catalog
