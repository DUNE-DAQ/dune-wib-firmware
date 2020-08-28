--/////////////////////////////////////////////////////////////////////
--////                              
--////  File: WIB_FEMB_COMM.VHD           
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
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_arith.all;
use IEEE.STD_LOGIC_unsigned.all;


--  Entity Declaration



ENTITY WIB_FEMB_COMM IS
	PORT
	(
		RESET   	   			: IN 	STD_LOGIC;					-- SYSTEM RESET
		SYS_CLK	   			: IN 	STD_LOGIC;					-- SYSTEM CLOCK
		
		FEMB_SCL					:	OUT	STD_LOGIC;				--	LVDS ,	FEMB DIFF I2C  CLOCK
		FEMB_SDA_P				:	INOUT STD_LOGIC;				-- DIFF 2.5V SSTL CLASS I , FEMB	DIFF I2C  DATA
		FEMB_SDA_N				:	INOUT STD_LOGIC;				-- DIFF 2.5V SSTL CLASS I , FEMB	DIFF I2C  DATA		
		
		FEMB_wr_strb 			: IN STD_LOGIC;								-- FEMB REGISTER WRITE
		FEMB_rd_strb 			: IN STD_LOGIC;								-- FEMB REGISTER READ
		FEMB_address 			: IN  STD_LOGIC_VECTOR(15 downto 0);	-- REGISTER ADDRESS
		FEMB_DATA_TO_FEMB 	: IN  STD_LOGIC_VECTOR(31 downto 0);	-- DATA TO THE FEMB
		FEMB_DATA_RDY			: OUT  STD_LOGIC;								-- DATA READY FROM FEMB READBACK
		FEMB_DATA_FRM_FEMB	: OUT  STD_LOGIC_VECTOR(31 downto 0);	-- FROM THE FEMB
		FEMB_BUSY				: OUT  STD_LOGIC
	);
	
END WIB_FEMB_COMM;


ARCHITECTURE behavior OF WIB_FEMB_COMM IS

  type state_type is (S_IDLE ,S_I2C_write,S_I2C_read_1,S_I2C_read_1_dly,S_I2C_read_2,S_I2C_read_3 ,S_I2C_read_3_DLY ,S_done,S_done_dly );
					 
  signal STATE : state_type;


SIGNAL		I2C_WR_STRB 	    : STD_LOGIC;
SIGNAL		I2C_RD_STRB 	    : STD_LOGIC;	
SIGNAL		I2C_NUM_BYTES	    : STD_LOGIC_VECTOR(3 downto 0);	  --I2C_NUM_BYTES --  For Writes 0 = address only,  1 = address + 1byte , 2 =  address + 2 bytes .... up to 4 bytes																		  --I2C_NUM_BYTES --  For Reads  0 = read 1 byte,   1 = read 1 byte,  2 = read 2 bytes  ..  up to 4 bytes
SIGNAL		I2C_ADDRESS		    : STD_LOGIC_VECTOR(15 downto 0);	  -- used only with WR_STRB
SIGNAL		I2C_DOUT			: STD_LOGIC_VECTOR(31 downto 0);	
SIGNAL		I2C_DIN			    : STD_LOGIC_VECTOR(31 downto 0);
SIGNAL		I2C_BUSY            : STD_LOGIC;
SIGNAL		BUSY_DLY_CNT	    : STD_LOGIC_VECTOR(7 downto 0);
SIGNAL      FEMB_wr_strb_dly    : STD_LOGIC;
SIGNAL      FEMB_rd_strb_dly    : STD_LOGIC;	
begin
	
		
	I2c_master_inst  : entity work.I2c_master_addr16 
	PORT MAP
	(
		rst   	   	=> RESET,				
		sys_clk	   	=> SYS_CLK,		
		SCL         	=> FEMB_SCL,
		SDA_P         	=> FEMB_SDA_P,
		SDA_N         	=> FEMB_SDA_N,
		I2C_WR_STRB 	=> I2C_WR_STRB,
		I2C_RD_STRB 	=> I2C_RD_STRB,
		I2C_DEV_ADDR	=>  b"0000101",
		I2C_NUM_BYTES	=> I2C_NUM_BYTES,
		I2C_ADDRESS		=> I2C_ADDRESS,  -- used only with WR_STRB
		I2C_EN_A16		=> '1',
		I2C_DOUT			=> I2C_DOUT,
		I2C_DIN			=> I2C_DIN,
		I2C_BUSY       => I2C_BUSY,
		I2C_DEV_AVL		=> open
	);
				
		
	process(SYS_CLK,RESET) 
	begin
	
		if(RESET = '1') then
			FEMB_DATA_RDY			<= '0';
			FEMB_DATA_FRM_FEMB	<= x"00000000";	
			I2C_WR_STRB 			<= '0';
			I2C_RD_STRB 			<= '0';
			I2C_NUM_BYTES			<= x"0";
			I2C_ADDRESS				<= x"0000";	
			I2C_DIN					<= x"00000000";
			BUSY_DLY_CNT			<= X"00";
			STATE						<= S_IDLE;
			FEMB_BUSY				<= '0';
		elsif (SYS_CLK'event  AND  SYS_CLK = '1') then	
	   FEMB_wr_strb_dly    <= FEMB_wr_strb;
	   FEMB_rd_strb_dly    <= FEMB_rd_strb;
	   case STATE is
			when S_IDLE =>		
		
					FEMB_DATA_RDY			<= '0';
					I2C_WR_STRB 			<= '0';
					I2C_RD_STRB 			<= '0';
					I2C_NUM_BYTES			<= x"0";	
					I2C_DIN					<=	FEMB_DATA_TO_FEMB;
					I2C_ADDRESS				<= FEMB_address;
					BUSY_DLY_CNT			<= X"00";
					FEMB_BUSY				<= '0';
					if( FEMB_wr_strb  = '1' and FEMB_wr_strb_dly = '0') then
							I2C_NUM_BYTES		<= x"4";
							FEMB_BUSY			<= '1';
							STATE					<= S_I2C_write;
					elsif (FEMB_rd_strb ='1' and FEMB_rd_strb_dly = '0') then
							I2C_WR_STRB 		<= '1';
							FEMB_BUSY			<= '1';
							STATE					<= S_I2C_read_1_dly;
					end if;
			when 	S_I2C_write =>
					I2C_WR_STRB 	<= '1';
					BUSY_DLY_CNT			<= X"00";
					STATE				<= S_done_dly;
					
			when S_I2C_read_1_dly =>	
					I2C_WR_STRB 	<= '0';
					I2C_RD_STRB 	<= '0';			
					BUSY_DLY_CNT	<= BUSY_DLY_CNT + 1;
					if(BUSY_DLY_CNT >= X"4") then
						STATE				<= S_I2C_read_1;
					else
						state 			<= S_I2C_read_1_dly;
					end if;											
			when 	S_I2C_read_1  =>
					I2C_WR_STRB 	<= '0';
					I2C_RD_STRB 	<= '0';
					if(I2C_BUSY = '1') then
						STATE				<= S_I2C_read_1;
					else
						I2C_NUM_BYTES	<= x"4";
						state 			<= S_I2C_read_2;						
					end if;				
			when 	S_I2C_read_2  =>				
					I2C_RD_STRB 			<= '1';
					BUSY_DLY_CNT			<= X"00";
					state 			<= S_I2C_read_3_dly;		
			when S_I2C_read_3_dly =>	
					I2C_WR_STRB 	<= '0';
					I2C_RD_STRB 	<= '0';			
					BUSY_DLY_CNT	<= BUSY_DLY_CNT + 1;
					if(BUSY_DLY_CNT >= X"4") then
						STATE				<= S_I2C_read_3;
					else
						state 			<= S_I2C_read_3_dly;
					end if;								
			when 	S_I2C_read_3  =>				
					I2C_RD_STRB 			<= '0';				
					if(I2C_BUSY = '1') then
						STATE				<= S_I2C_read_3;
					else
						FEMB_DATA_FRM_FEMB	<= I2C_DOUT;
						FEMB_DATA_RDY			<= '1';						
						STATE						<= S_done_dly;
					end if;		
			when S_done_dly =>	
					I2C_WR_STRB 	<= '0';
					I2C_RD_STRB 	<= '0';			
					BUSY_DLY_CNT	<= BUSY_DLY_CNT + 1;
					if(BUSY_DLY_CNT >= X"4") then
						STATE				<= S_done;
					else
						state 			<= S_done_dly;
					end if;
			when S_done =>	
					I2C_WR_STRB 	<= '0';
					I2C_RD_STRB 	<= '0';
					if(I2C_BUSY = '1') then
						STATE				<= S_done;
					else
						state 			<= S_IDLE;
					end if;
			when others =>		
					state <= S_IDLE;	
			end case; 		
		end if;
	end process;	

	
	
	

	
END behavior;
