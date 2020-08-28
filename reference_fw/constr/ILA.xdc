

create_debug_core u_ila_0 ila
set_property ALL_PROBE_SAME_MU true [get_debug_cores u_ila_0]
set_property ALL_PROBE_SAME_MU_CNT 1 [get_debug_cores u_ila_0]
set_property C_ADV_TRIGGER false [get_debug_cores u_ila_0]
set_property C_DATA_DEPTH 1024 [get_debug_cores u_ila_0]
set_property C_EN_STRG_QUAL false [get_debug_cores u_ila_0]
set_property C_INPUT_PIPE_STAGES 0 [get_debug_cores u_ila_0]
set_property C_TRIGIN_EN false [get_debug_cores u_ila_0]
set_property C_TRIGOUT_EN false [get_debug_cores u_ila_0]
set_property port_width 1 [get_debug_ports u_ila_0/clk]
connect_debug_port u_ila_0/clk [get_nets [list {FEMB_GTH_inst/FEMB_GTH_example_top_inst/example_wrapper_inst/FEMB_GTH_inst/inst/gen_gtwizard_gthe4_top.FEMB_GTH_gtwizard_gthe4_inst/gen_gtwizard_gthe4.gen_rx_user_clocking_internal.gen_single_instance.gtwiz_userclk_rx_inst/gtwiz_userclk_rx_usrclk2_out[0]}]]
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe0]
set_property port_width 4 [get_debug_ports u_ila_0/probe0]
connect_debug_port u_ila_0/probe0 [get_nets [list {FEMB_GTH_inst/rxcommadet_out[0]} {FEMB_GTH_inst/rxcommadet_out[1]} {FEMB_GTH_inst/rxcommadet_out[2]} {FEMB_GTH_inst/rxcommadet_out[3]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe1]
set_property port_width 4 [get_debug_ports u_ila_0/probe1]
connect_debug_port u_ila_0/probe1 [get_nets [list {FEMB_GTH_inst/gtpowergood_out[0]} {FEMB_GTH_inst/gtpowergood_out[1]} {FEMB_GTH_inst/gtpowergood_out[2]} {FEMB_GTH_inst/gtpowergood_out[3]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe2]
set_property port_width 64 [get_debug_ports u_ila_0/probe2]
connect_debug_port u_ila_0/probe2 [get_nets [list {FEMB_GTH_inst/userdata_rx_out[0]} {FEMB_GTH_inst/userdata_rx_out[1]} {FEMB_GTH_inst/userdata_rx_out[2]} {FEMB_GTH_inst/userdata_rx_out[3]} {FEMB_GTH_inst/userdata_rx_out[4]} {FEMB_GTH_inst/userdata_rx_out[5]} {FEMB_GTH_inst/userdata_rx_out[6]} {FEMB_GTH_inst/userdata_rx_out[7]} {FEMB_GTH_inst/userdata_rx_out[8]} {FEMB_GTH_inst/userdata_rx_out[9]} {FEMB_GTH_inst/userdata_rx_out[10]} {FEMB_GTH_inst/userdata_rx_out[11]} {FEMB_GTH_inst/userdata_rx_out[12]} {FEMB_GTH_inst/userdata_rx_out[13]} {FEMB_GTH_inst/userdata_rx_out[14]} {FEMB_GTH_inst/userdata_rx_out[15]} {FEMB_GTH_inst/userdata_rx_out[16]} {FEMB_GTH_inst/userdata_rx_out[17]} {FEMB_GTH_inst/userdata_rx_out[18]} {FEMB_GTH_inst/userdata_rx_out[19]} {FEMB_GTH_inst/userdata_rx_out[20]} {FEMB_GTH_inst/userdata_rx_out[21]} {FEMB_GTH_inst/userdata_rx_out[22]} {FEMB_GTH_inst/userdata_rx_out[23]} {FEMB_GTH_inst/userdata_rx_out[24]} {FEMB_GTH_inst/userdata_rx_out[25]} {FEMB_GTH_inst/userdata_rx_out[26]} {FEMB_GTH_inst/userdata_rx_out[27]} {FEMB_GTH_inst/userdata_rx_out[28]} {FEMB_GTH_inst/userdata_rx_out[29]} {FEMB_GTH_inst/userdata_rx_out[30]} {FEMB_GTH_inst/userdata_rx_out[31]} {FEMB_GTH_inst/userdata_rx_out[32]} {FEMB_GTH_inst/userdata_rx_out[33]} {FEMB_GTH_inst/userdata_rx_out[34]} {FEMB_GTH_inst/userdata_rx_out[35]} {FEMB_GTH_inst/userdata_rx_out[36]} {FEMB_GTH_inst/userdata_rx_out[37]} {FEMB_GTH_inst/userdata_rx_out[38]} {FEMB_GTH_inst/userdata_rx_out[39]} {FEMB_GTH_inst/userdata_rx_out[40]} {FEMB_GTH_inst/userdata_rx_out[41]} {FEMB_GTH_inst/userdata_rx_out[42]} {FEMB_GTH_inst/userdata_rx_out[43]} {FEMB_GTH_inst/userdata_rx_out[44]} {FEMB_GTH_inst/userdata_rx_out[45]} {FEMB_GTH_inst/userdata_rx_out[46]} {FEMB_GTH_inst/userdata_rx_out[47]} {FEMB_GTH_inst/userdata_rx_out[48]} {FEMB_GTH_inst/userdata_rx_out[49]} {FEMB_GTH_inst/userdata_rx_out[50]} {FEMB_GTH_inst/userdata_rx_out[51]} {FEMB_GTH_inst/userdata_rx_out[52]} {FEMB_GTH_inst/userdata_rx_out[53]} {FEMB_GTH_inst/userdata_rx_out[54]} {FEMB_GTH_inst/userdata_rx_out[55]} {FEMB_GTH_inst/userdata_rx_out[56]} {FEMB_GTH_inst/userdata_rx_out[57]} {FEMB_GTH_inst/userdata_rx_out[58]} {FEMB_GTH_inst/userdata_rx_out[59]} {FEMB_GTH_inst/userdata_rx_out[60]} {FEMB_GTH_inst/userdata_rx_out[61]} {FEMB_GTH_inst/userdata_rx_out[62]} {FEMB_GTH_inst/userdata_rx_out[63]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe3]
set_property port_width 4 [get_debug_ports u_ila_0/probe3]
connect_debug_port u_ila_0/probe3 [get_nets [list {FEMB_GTH_inst/rxbyterealign_out[0]} {FEMB_GTH_inst/rxbyterealign_out[1]} {FEMB_GTH_inst/rxbyterealign_out[2]} {FEMB_GTH_inst/rxbyterealign_out[3]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe4]
set_property port_width 4 [get_debug_ports u_ila_0/probe4]
connect_debug_port u_ila_0/probe4 [get_nets [list {FEMB_GTH_inst/rxbyteisaligned_out[0]} {FEMB_GTH_inst/rxbyteisaligned_out[1]} {FEMB_GTH_inst/rxbyteisaligned_out[2]} {FEMB_GTH_inst/rxbyteisaligned_out[3]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe5]
set_property port_width 1 [get_debug_ports u_ila_0/probe5]
connect_debug_port u_ila_0/probe5 [get_nets [list FEMB_GTH_inst/gtwiz_reset_rx_cdr_stable_out]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe6]
set_property port_width 1 [get_debug_ports u_ila_0/probe6]
connect_debug_port u_ila_0/probe6 [get_nets [list FEMB_GTH_inst/reset_rx_done_out]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe7]
set_property port_width 1 [get_debug_ports u_ila_0/probe7]
connect_debug_port u_ila_0/probe7 [get_nets [list FEMB_GTH_inst/userclk_rx_active_out]]
set_property C_CLK_INPUT_FREQ_HZ 300000000 [get_debug_cores dbg_hub]
set_property C_ENABLE_CLK_DIVIDER false [get_debug_cores dbg_hub]
set_property C_USER_SCAN_CHAIN 1 [get_debug_cores dbg_hub]
connect_debug_port dbg_hub/clk [get_nets clk]
