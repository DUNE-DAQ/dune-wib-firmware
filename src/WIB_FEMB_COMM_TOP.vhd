--/////////////////////////////////////////////////////////////////////
--////                              
--////  File: WIB_FEMB_COMM_TOP.VHD           
--////                                                                                                                                      
--////  Author: Jack Fried			                  
--////          jfried@bnl.gov	              
--////  Created: 08/08/2016
--////  Description:  WIB_FEMB_COMM
--////					
--////
--/////////////////////////////////////////////////////////////////////
--////
--//// Copyright (C) 2016 Brookhaven National Laboratory
--////
--/////////////////////////////////////////////////////////////////////

library IEEE;
library UNISIM;
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_arith.all;
use IEEE.STD_LOGIC_unsigned.all;
use UNISIM.VComponents.all;

--  Entity Declaration



ENTITY WIB_FEMB_COMM_TOP IS
	PORT
	(
		RESET   	   			: IN 	STD_LOGIC;					-- SYSTEM RESET
		SYS_CLK	   			: IN 	STD_LOGIC;					-- SYSTEM CLOCK
				
				
		FEMB_wr_strb 			: IN STD_LOGIC;								-- FEMB REGISTER WRITE
		FEMB_rd_strb 			: IN STD_LOGIC;								-- FEMB REGISTER READ
		FEMB_address 			: IN  STD_LOGIC_VECTOR(15 downto 0);	-- REGISTER ADDRESS
		FEMB_BRD					: IN  STD_LOGIC_VECTOR(3 downto 0);    --  BOARD CHANNEL NUMBER
		FEMB_DATA_TO_FEMB		: IN  STD_LOGIC_VECTOR(31 downto 0);	-- DATA TO THE FEMB
		FEMB_DATA_RDY			: OUT  STD_LOGIC;								-- DATA READY FROM FEMB READBACK STROBE
		FEMB_DATA_FRM_FEMB 	: OUT  STD_LOGIC_VECTOR(31 downto 0);	-- FROM THE FEMB
		
		
		FEMB_SCL_BRD0_P		:	OUT	STD_LOGIC;				--	LVDS ,	FEMB DIFF I2C  CLOCK
		FEMB_SCL_BRD0_N		:	OUT	STD_LOGIC;				--	LVDS ,	FEMB DIFF I2C  CLOCK
		
		FEMB_SDA_BRD0_P		:	INOUT STD_LOGIC;				-- DIFF 2.5V SSTL CLASS I , FEMB	DIFF I2C  DATA
		FEMB_SDA_BRD0_N		:	INOUT STD_LOGIC;				-- DIFF 2.5V SSTL CLASS I , FEMB	DIFF I2C  DATA

		FEMB_SCL_BRD1_P		:	OUT	STD_LOGIC;				--	LVDS ,	FEMB DIFF I2C  CLOCK
		FEMB_SCL_BRD1_N		:	OUT	STD_LOGIC;				--	LVDS ,	FEMB DIFF I2C  CLOCK		
		FEMB_SDA_BRD1_P		:	INOUT STD_LOGIC;				-- DIFF 2.5V SSTL CLASS I , FEMB	DIFF I2C  DATA
		FEMB_SDA_BRD1_N		:	INOUT STD_LOGIC;				-- DIFF 2.5V SSTL CLASS I , FEMB	DIFF I2C  DATA	

		FEMB_SCL_BRD2_P		:	OUT	STD_LOGIC;				--	LVDS ,	FEMB DIFF I2C  CLOCK
		FEMB_SCL_BRD2_N		:	OUT	STD_LOGIC;				--	LVDS ,	FEMB DIFF I2C  CLOCK		
		FEMB_SDA_BRD2_P		:	INOUT STD_LOGIC;				-- DIFF 2.5V SSTL CLASS I , FEMB	DIFF I2C  DATA
		FEMB_SDA_BRD2_N		:	INOUT STD_LOGIC;				-- DIFF 2.5V SSTL CLASS I , FEMB	DIFF I2C  DATA	
		
		FEMB_SCL_BRD3_P		:	OUT	STD_LOGIC;				--	LVDS ,	FEMB DIFF I2C  CLOCK
		FEMB_SCL_BRD3_N		:	OUT	STD_LOGIC;				--	LVDS ,	FEMB DIFF I2C  CLOCK		
		FEMB_SDA_BRD3_P		:	INOUT STD_LOGIC;				-- DIFF 2.5V SSTL CLASS I , FEMB	DIFF I2C  DATA
		FEMB_SDA_BRD3_N		:	INOUT STD_LOGIC;				-- DIFF 2.5V SSTL CLASS I , FEMB	DIFF I2C  DATA		
		FEMB_BUSY			: OUT  STD_LOGIC_VECTOR(3 downto 0)
	);
	
END WIB_FEMB_COMM_TOP;


ARCHITECTURE behavior OF WIB_FEMB_COMM_TOP IS

		
SIGNAL		FEMB_wr_strb_0      		: STD_LOGIC;		
SIGNAL		FEMB_rd_strb_0      		: STD_LOGIC;		
SIGNAL		FEMB_DATA_RDY_0      	    : STD_LOGIC;		
SIGNAL		FEMB_DATA_FRM_FEMB_0		: STD_LOGIC_VECTOR(31 downto 0);	

SIGNAL		FEMB_wr_strb_1      		: STD_LOGIC;		
SIGNAL		FEMB_rd_strb_1      		: STD_LOGIC;		
SIGNAL		FEMB_DATA_RDY_1      	    : STD_LOGIC;		
SIGNAL		FEMB_DATA_FRM_FEMB_1		: STD_LOGIC_VECTOR(31 downto 0);	

SIGNAL		FEMB_wr_strb_2      		: STD_LOGIC;		
SIGNAL		FEMB_rd_strb_2      		: STD_LOGIC;		
SIGNAL		FEMB_DATA_RDY_2      	    : STD_LOGIC;		
SIGNAL		FEMB_DATA_FRM_FEMB_2		: STD_LOGIC_VECTOR(31 downto 0);	

SIGNAL		FEMB_wr_strb_3      		: STD_LOGIC;		
SIGNAL		FEMB_rd_strb_3      		: STD_LOGIC;		
SIGNAL		FEMB_DATA_RDY_3      	    : STD_LOGIC;		
SIGNAL		FEMB_DATA_FRM_FEMB_3		: STD_LOGIC_VECTOR(31 downto 0);	

SIGNAL      FEMB_SCL_BRD0               : STD_LOGIC;
SIGNAL      FEMB_SCL_BRD1               : STD_LOGIC;
SIGNAL      FEMB_SCL_BRD2               : STD_LOGIC;
SIGNAL      FEMB_SCL_BRD3               : STD_LOGIC;


begin
	

	FEMB_wr_strb_0		<= FEMB_wr_strb WHEN (FEMB_BRD = X"0") ELSE '0';
	FEMB_rd_strb_0		<= FEMB_rd_strb WHEN (FEMB_BRD = X"0") ELSE '0';
	FEMB_wr_strb_1		<= FEMB_wr_strb WHEN (FEMB_BRD = X"1") ELSE '0';
	FEMB_rd_strb_1		<= FEMB_rd_strb WHEN (FEMB_BRD = X"1") ELSE '0';	
	FEMB_wr_strb_2		<= FEMB_wr_strb WHEN (FEMB_BRD = X"2") ELSE '0';
	FEMB_rd_strb_2		<= FEMB_rd_strb WHEN (FEMB_BRD = X"2") ELSE '0';	
	FEMB_wr_strb_3		<= FEMB_wr_strb WHEN (FEMB_BRD = X"3") ELSE '0';
	FEMB_rd_strb_3		<= FEMB_rd_strb WHEN (FEMB_BRD = X"3") ELSE '0';
	
	
	FEMB_DATA_FRM_FEMB	<= FEMB_DATA_FRM_FEMB_0 WHEN (FEMB_BRD = X"0") ELSE 
									FEMB_DATA_FRM_FEMB_1 WHEN (FEMB_BRD = X"1") ELSE 
									FEMB_DATA_FRM_FEMB_2 WHEN (FEMB_BRD = X"2") ELSE 
									FEMB_DATA_FRM_FEMB_3 WHEN (FEMB_BRD = X"3") ELSE 
									X"00000000";

	FEMB_DATA_RDY			<= FEMB_DATA_RDY_0 WHEN (FEMB_BRD = X"0") ELSE 
									FEMB_DATA_RDY_1 WHEN (FEMB_BRD = X"1") ELSE 
									FEMB_DATA_RDY_2 WHEN (FEMB_BRD = X"2") ELSE 
									FEMB_DATA_RDY_3 WHEN (FEMB_BRD = X"3") ELSE 
									'0';

OBUFDS_FEMB0 : OBUFDS  port map (O => FEMB_SCL_BRD0_P, OB => FEMB_SCL_BRD0_N, I => FEMB_SCL_BRD0 );
OBUFDS_FEMB1 : OBUFDS  port map (O => FEMB_SCL_BRD1_P, OB => FEMB_SCL_BRD1_N, I => FEMB_SCL_BRD1 );
OBUFDS_FEMB2 : OBUFDS  port map (O => FEMB_SCL_BRD2_P, OB => FEMB_SCL_BRD2_N, I => FEMB_SCL_BRD2 );
OBUFDS_FEMB3 : OBUFDS  port map (O => FEMB_SCL_BRD3_P, OB => FEMB_SCL_BRD3_N, I => FEMB_SCL_BRD3 );
								
									
WIB_FEMB_COMM_INST1 : ENTITY WORK.WIB_FEMB_COMM
	PORT MAP
	(
		RESET   	   		=> RESET,
		SYS_CLK	   		    => SYS_CLK,
		
		FEMB_SCL			=> FEMB_SCL_BRD0,
		FEMB_SDA_P			=> FEMB_SDA_BRD0_P,
		FEMB_SDA_N			=> FEMB_SDA_BRD0_N,	
		
		FEMB_wr_strb 		=> FEMB_wr_strb_0,
		FEMB_rd_strb 		=> FEMB_rd_strb_0, 
		FEMB_address 	 	=> FEMB_address,
		FEMB_DATA_TO_FEMB   => FEMB_DATA_TO_FEMB,
		FEMB_DATA_RDY		=> FEMB_DATA_RDY_0,
		FEMB_DATA_FRM_FEMB  => FEMB_DATA_FRM_FEMB_0,
		FEMB_BUSY			=> FEMB_BUSY(0)
	);
	
	
WIB_FEMB_COMM_INST2 : ENTITY WORK.WIB_FEMB_COMM
	PORT MAP
	(
		RESET   	   		=> RESET,
		SYS_CLK	   		    => SYS_CLK,
		
		FEMB_SCL			=> FEMB_SCL_BRD1,
		FEMB_SDA_P			=> FEMB_SDA_BRD1_P,
		FEMB_SDA_N			=> FEMB_SDA_BRD1_N,	
		
		FEMB_wr_strb 		 => FEMB_wr_strb_1,
		FEMB_rd_strb 		 => FEMB_rd_strb_1, 
		FEMB_address 	 	 => FEMB_address,
		FEMB_DATA_TO_FEMB    => FEMB_DATA_TO_FEMB,
		FEMB_DATA_RDY		 => FEMB_DATA_RDY_1,
		FEMB_DATA_FRM_FEMB   => FEMB_DATA_FRM_FEMB_1,
		FEMB_BUSY			 => FEMB_BUSY(1)
	);
		
WIB_FEMB_COMM_INST3 : ENTITY WORK.WIB_FEMB_COMM
	PORT MAP
	(
		RESET   	   		=> RESET,
		SYS_CLK	   		    => SYS_CLK,
		
		FEMB_SCL			=> FEMB_SCL_BRD2,
		FEMB_SDA_P			=> FEMB_SDA_BRD2_P,
		FEMB_SDA_N			=> FEMB_SDA_BRD2_N,	
		
		FEMB_wr_strb 		=> FEMB_wr_strb_2,
		FEMB_rd_strb 		=> FEMB_rd_strb_2, 
		FEMB_address 	 	=> FEMB_address,
		FEMB_DATA_TO_FEMB   => FEMB_DATA_TO_FEMB,
		FEMB_DATA_RDY		=> FEMB_DATA_RDY_2,
		FEMB_DATA_FRM_FEMB  => FEMB_DATA_FRM_FEMB_2,
		FEMB_BUSY			=> FEMB_BUSY(2)
	);
		
WIB_FEMB_COMM_INST4 : ENTITY WORK.WIB_FEMB_COMM
	PORT MAP
	(
		RESET   	   		=> RESET,
		SYS_CLK	   		    => SYS_CLK,
		
		FEMB_SCL			=> FEMB_SCL_BRD3,
		FEMB_SDA_P			=> FEMB_SDA_BRD3_P,
		FEMB_SDA_N			=> FEMB_SDA_BRD3_N,	
		FEMB_wr_strb 		=> FEMB_wr_strb_3,
		FEMB_rd_strb 		=> FEMB_rd_strb_3, 
		FEMB_address 	 	=> FEMB_address,
		FEMB_DATA_TO_FEMB   => FEMB_DATA_TO_FEMB,
		FEMB_DATA_RDY		=> FEMB_DATA_RDY_3,
		FEMB_DATA_FRM_FEMB  => FEMB_DATA_FRM_FEMB_3,
		FEMB_BUSY			=> FEMB_BUSY(3)
	);
		
	
END behavior;

