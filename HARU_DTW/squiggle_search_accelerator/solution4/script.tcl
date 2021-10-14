############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2020 Xilinx, Inc. All Rights Reserved.
############################################################
open_project squiggle_search_accelerator
set_top subsequence_search
add_files src/squiggle_search.cpp
add_files -tb src/squiggle_search_top.cpp -cflags "-Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"
open_solution "solution4"
set_part {xc7z020-clg484-1}
create_clock -period 10 -name default
config_sdx -target none
config_export -vivado_optimization_level 2 -vivado_phys_opt place -vivado_report_level 0
set_clock_uncertainty 12.5%
source "./squiggle_search_accelerator/solution4/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -format ip_catalog
