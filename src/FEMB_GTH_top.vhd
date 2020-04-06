----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 01/11/2020 10:15:32 PM
-- Design Name: 
-- Module Name: FEMB_GTH_top - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;

entity FEMB_GTH_top is
port(
	gthtxn_out								: OUT STD_LOGIC_VECTOR(3 downto 0); 
	gthtxp_out								: OUT STD_LOGIC_VECTOR(3 downto 0); 		
	gthrxn_in								: IN STD_LOGIC_VECTOR(3 downto 0); 
	gthrxp_in								: IN STD_LOGIC_VECTOR(3 downto 0); 
	
    gtrefclk0_p                             : IN STD_LOGIC; 
    gtrefclk0_n                             : IN STD_LOGIC; 	
		
    reset                                   : IN STD_LOGIC;   -- reset core
    clk_50MHz                               : IN STD_LOGIC;   -- freerun clock
    		
    userdata_rx_out					        : OUT STD_LOGIC_VECTOR(63 downto 0); 			
	rxusrclk2_OUT							: OUT STD_LOGIC;	
	rxctrl0_out								: OUT STD_LOGIC_VECTOR(63 downto 0); 
	rxctrl1_out								: OUT STD_LOGIC_VECTOR(63 downto 0); 
	rxctrl2_out								: OUT STD_LOGIC_VECTOR(31 downto 0); 
	rxctrl3_out								: OUT STD_LOGIC_VECTOR(31 downto 0); 
	gtwiz_reset_rx_cdr_stable_out			: OUT STD_LOGIC; 
	reset_rx_done_out					    : OUT STD_LOGIC; 	
    userclk_rx_active_out				    : OUT STD_LOGIC; 			
		
	gtpowergood_out							: OUT STD_LOGIC_VECTOR(3 downto 0); 
	rxbyteisaligned_out						: OUT STD_LOGIC_VECTOR(3 downto 0); 
	rxbyterealign_out						: OUT STD_LOGIC_VECTOR(3 downto 0); 
	rxcommadet_out							: OUT STD_LOGIC_VECTOR(3 downto 0)	

  );
end FEMB_GTH_top;

architecture Behavioral of FEMB_GTH_top is


 component FEMB_GTH_example_top is
  port (
         mgtrefclk0_x1y1_p		: IN  STD_LOGIC;
         mgtrefclk0_x1y1_n		: IN  STD_LOGIC;
        

          gthrxn_in             : IN  STD_LOGIC_VECTOR(3 downto 0);
          gthrxp_in             : IN  STD_LOGIC_VECTOR(3 downto 0);
          gthtxn_out            : OUT  STD_LOGIC_VECTOR(3 downto 0);
          gthtxp_out            : OUT  STD_LOGIC_VECTOR(3 downto 0);


          userdata_rx_out					    : OUT STD_LOGIC_VECTOR(63 downto 0); 			
          rxusrclk2_OUT							: OUT STD_LOGIC;	
          rxctrl0_out							: OUT STD_LOGIC_VECTOR(63 downto 0); 
          rxctrl1_out							: OUT STD_LOGIC_VECTOR(63 downto 0); 
          rxctrl2_out							: OUT STD_LOGIC_VECTOR(31 downto 0); 
          rxctrl3_out							: OUT STD_LOGIC_VECTOR(31 downto 0); 
          gtwiz_reset_rx_cdr_stable_out			: OUT STD_LOGIC; 
          reset_rx_done_out					    : OUT STD_LOGIC; 	
          userclk_rx_active_out				    : OUT STD_LOGIC; 			


         hb_gtwiz_reset_clk_freerun_in   : IN  STD_LOGIC;
         hb_gtwiz_reset_all_in   		 : IN  STD_LOGIC;
        
        
         link_down_latched_reset_in 	: IN  STD_LOGIC;
         link_status_out				: OUT STD_LOGIC;
         clk_sel_out					: OUT STD_LOGIC;
         link_down_latched_out 			: OUT STD_LOGIC
    );
    end component FEMB_GTH_example_top;


    SIGNAL       link_status_out				:  STD_LOGIC;
    SIGNAL       clk_sel_out					:  STD_LOGIC;
    SIGNAL       link_down_latched_out 			:  STD_LOGIC;







  attribute mark_debug : string;  
  attribute mark_debug of userdata_rx_out	                      : signal is "true";
  attribute mark_debug of rxbyteisaligned_out                     : signal is "true"; 
  attribute mark_debug of rxbyterealign_out                       : signal is "true";  
  attribute mark_debug of rxcommadet_out                          : signal is "true";     
--  attribute mark_debug of rxpmaresetdone_out                      : signal is "true";
  attribute mark_debug of gtwiz_reset_rx_cdr_stable_out           : signal is "true";    
  attribute mark_debug of reset_rx_done_out                       : signal is "true";  
  attribute mark_debug of userclk_rx_active_out                   : signal is "true";     
  attribute mark_debug of gtpowergood_out                         : signal is "true";  
--  attribute mark_debug of reset_in                                : signal is "true";  
--  attribute mark_debug of reset_int                               : signal is "true";  
--  attribute mark_debug of reset_all_init                          : signal is "true";  
--  attribute mark_debug of gtwiz_reset_rx_datapath_in              : signal is "true";  
--  attribute mark_debug of gtwiz_reset_rx_done_out	              : signal is "true";  
--  attribute mark_debug of gtwiz_userclk_rx_reset_in               : signal is "true";  





begin

FEMB_GTH_example_top_inst : FEMB_GTH_example_top
  port map ( 
        mgtrefclk0_x1y1_p		        =>    gtrefclk0_p,		      
        mgtrefclk0_x1y1_n		        =>    gtrefclk0_n, 	
        
        gthtxn_out                       =>   gthtxn_out,           
        gthtxp_out                       =>   gthtxp_out,  
        gthrxn_in	                     =>   gthrxn_in,
        gthrxp_in	                     =>   gthrxp_in,	
             
        userdata_rx_out					 =>     userdata_rx_out,
        rxusrclk2_OUT				     =>     rxusrclk2_OUT,
        rxctrl0_out						 =>	    rxctrl0_out,
        rxctrl1_out						 =>	    rxctrl1_out,
        rxctrl2_out						 =>	    rxctrl2_out,
        rxctrl3_out						 =>     rxctrl3_out,
        gtwiz_reset_rx_cdr_stable_out    =>     gtwiz_reset_rx_cdr_stable_out,	
        reset_rx_done_out				 =>	    reset_rx_done_out,
        userclk_rx_active_out			 =>     userclk_rx_active_out,
           
        hb_gtwiz_reset_clk_freerun_in   =>    clk_50MHz ,
        hb_gtwiz_reset_all_in   		=>    reset,   		
        link_down_latched_reset_in 	    =>    '0', 	
        link_status_out				    =>    link_status_out,				
        clk_sel_out					    =>    clk_sel_out,					
        link_down_latched_out 			=>    link_down_latched_out 			
                                             
);

end Behavioral;
