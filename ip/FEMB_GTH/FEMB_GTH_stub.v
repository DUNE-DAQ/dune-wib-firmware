// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2019.1 (lin64) Build 2552052 Fri May 24 14:47:09 MDT 2019
// Date        : Tue Mar 31 13:39:59 2020
// Host        : nubar0 running 64-bit Ubuntu 14.04.6 LTS
// Command     : write_verilog -force -mode synth_stub
//               /nfs/wimp/home/benland100/IcebergWIB/DUNE_WIB/DUNE_WIB.srcs/sources_1/ip/FEMB_GTH/FEMB_GTH_stub.v
// Design      : FEMB_GTH
// Purpose     : Stub declaration of top-level module interface
// Device      : xczu9eg-ffvb1156-2-e
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* X_CORE_INFO = "FEMB_GTH_gtwizard_top,Vivado 2019.1" *)
module FEMB_GTH(gtwiz_userclk_tx_reset_in, 
  gtwiz_userclk_tx_srcclk_out, gtwiz_userclk_tx_usrclk_out, 
  gtwiz_userclk_tx_usrclk2_out, gtwiz_userclk_tx_active_out, gtwiz_userclk_rx_reset_in, 
  gtwiz_userclk_rx_srcclk_out, gtwiz_userclk_rx_usrclk_out, 
  gtwiz_userclk_rx_usrclk2_out, gtwiz_userclk_rx_active_out, 
  gtwiz_reset_clk_freerun_in, gtwiz_reset_all_in, gtwiz_reset_tx_pll_and_datapath_in, 
  gtwiz_reset_tx_datapath_in, gtwiz_reset_rx_pll_and_datapath_in, 
  gtwiz_reset_rx_datapath_in, gtwiz_reset_rx_cdr_stable_out, gtwiz_reset_tx_done_out, 
  gtwiz_reset_rx_done_out, gtwiz_userdata_tx_in, gtwiz_userdata_rx_out, drpclk_in, 
  gthrxn_in, gthrxp_in, gtrefclk0_in, rx8b10ben_in, rxcommadeten_in, rxmcommaalignen_in, 
  rxpcommaalignen_in, tx8b10ben_in, txctrl0_in, txctrl1_in, txctrl2_in, txpd_in, gthtxn_out, 
  gthtxp_out, gtpowergood_out, rxbyteisaligned_out, rxbyterealign_out, rxcommadet_out, 
  rxctrl0_out, rxctrl1_out, rxctrl2_out, rxctrl3_out, rxpmaresetdone_out, txpmaresetdone_out)
/* synthesis syn_black_box black_box_pad_pin="gtwiz_userclk_tx_reset_in[0:0],gtwiz_userclk_tx_srcclk_out[0:0],gtwiz_userclk_tx_usrclk_out[0:0],gtwiz_userclk_tx_usrclk2_out[0:0],gtwiz_userclk_tx_active_out[0:0],gtwiz_userclk_rx_reset_in[0:0],gtwiz_userclk_rx_srcclk_out[0:0],gtwiz_userclk_rx_usrclk_out[0:0],gtwiz_userclk_rx_usrclk2_out[0:0],gtwiz_userclk_rx_active_out[0:0],gtwiz_reset_clk_freerun_in[0:0],gtwiz_reset_all_in[0:0],gtwiz_reset_tx_pll_and_datapath_in[0:0],gtwiz_reset_tx_datapath_in[0:0],gtwiz_reset_rx_pll_and_datapath_in[0:0],gtwiz_reset_rx_datapath_in[0:0],gtwiz_reset_rx_cdr_stable_out[0:0],gtwiz_reset_tx_done_out[0:0],gtwiz_reset_rx_done_out[0:0],gtwiz_userdata_tx_in[63:0],gtwiz_userdata_rx_out[63:0],drpclk_in[3:0],gthrxn_in[3:0],gthrxp_in[3:0],gtrefclk0_in[3:0],rx8b10ben_in[3:0],rxcommadeten_in[3:0],rxmcommaalignen_in[3:0],rxpcommaalignen_in[3:0],tx8b10ben_in[3:0],txctrl0_in[63:0],txctrl1_in[63:0],txctrl2_in[31:0],txpd_in[7:0],gthtxn_out[3:0],gthtxp_out[3:0],gtpowergood_out[3:0],rxbyteisaligned_out[3:0],rxbyterealign_out[3:0],rxcommadet_out[3:0],rxctrl0_out[63:0],rxctrl1_out[63:0],rxctrl2_out[31:0],rxctrl3_out[31:0],rxpmaresetdone_out[3:0],txpmaresetdone_out[3:0]" */;
  input [0:0]gtwiz_userclk_tx_reset_in;
  output [0:0]gtwiz_userclk_tx_srcclk_out;
  output [0:0]gtwiz_userclk_tx_usrclk_out;
  output [0:0]gtwiz_userclk_tx_usrclk2_out;
  output [0:0]gtwiz_userclk_tx_active_out;
  input [0:0]gtwiz_userclk_rx_reset_in;
  output [0:0]gtwiz_userclk_rx_srcclk_out;
  output [0:0]gtwiz_userclk_rx_usrclk_out;
  output [0:0]gtwiz_userclk_rx_usrclk2_out;
  output [0:0]gtwiz_userclk_rx_active_out;
  input [0:0]gtwiz_reset_clk_freerun_in;
  input [0:0]gtwiz_reset_all_in;
  input [0:0]gtwiz_reset_tx_pll_and_datapath_in;
  input [0:0]gtwiz_reset_tx_datapath_in;
  input [0:0]gtwiz_reset_rx_pll_and_datapath_in;
  input [0:0]gtwiz_reset_rx_datapath_in;
  output [0:0]gtwiz_reset_rx_cdr_stable_out;
  output [0:0]gtwiz_reset_tx_done_out;
  output [0:0]gtwiz_reset_rx_done_out;
  input [63:0]gtwiz_userdata_tx_in;
  output [63:0]gtwiz_userdata_rx_out;
  input [3:0]drpclk_in;
  input [3:0]gthrxn_in;
  input [3:0]gthrxp_in;
  input [3:0]gtrefclk0_in;
  input [3:0]rx8b10ben_in;
  input [3:0]rxcommadeten_in;
  input [3:0]rxmcommaalignen_in;
  input [3:0]rxpcommaalignen_in;
  input [3:0]tx8b10ben_in;
  input [63:0]txctrl0_in;
  input [63:0]txctrl1_in;
  input [31:0]txctrl2_in;
  input [7:0]txpd_in;
  output [3:0]gthtxn_out;
  output [3:0]gthtxp_out;
  output [3:0]gtpowergood_out;
  output [3:0]rxbyteisaligned_out;
  output [3:0]rxbyterealign_out;
  output [3:0]rxcommadet_out;
  output [63:0]rxctrl0_out;
  output [63:0]rxctrl1_out;
  output [31:0]rxctrl2_out;
  output [31:0]rxctrl3_out;
  output [3:0]rxpmaresetdone_out;
  output [3:0]txpmaresetdone_out;
endmodule
