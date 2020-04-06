--/////////////////////////////////////////////////////////////////////
--////                              
--////  File: SBND_PWM_CLK_ENCODER.VHD            
--////                                                                                                                                      
--////  Author: Jack Fried			                  
--////          jfried@bnl.gov	              
--////  Created: 10/21/2016 
--////  Description:  		
--////					
--////
--/////////////////////////////////////////////////////////////////////
--////
--//// Copyright (C) 2016 Brookhaven National Laboratory
--////
--/////////////////////////////////////////////////////////////////////


library ieee;
use ieee.std_logic_1164.all;
USE ieee.std_logic_arith.all;
USE ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
library UNISIM;
use UNISIM.VComponents.all;



entity SBND_PWM_CLK_ENCODER is
	PORT
	(
	
			RESET					: IN STD_LOGIC;	
			CLK_100MHz			: IN STD_LOGIC;
			SAMPLE_RATE			: IN STD_LOGIC_vector(3 downto 0);  -- 0 = 2MHz   1= 1.5MHz   2 = 1MHz  3 = 500KHz
			
			EXT_CMD1				: IN STD_LOGIC; -- CALIB
			EXT_CMD2				: IN STD_LOGIC; -- TIMESTAMP RESET
			EXT_CMD3				: IN STD_LOGIC; -- START
			EXT_CMD4				: IN STD_LOGIC; -- STOP
			
			SW_CMD1				: IN STD_LOGIC;
			SW_CMD2				: IN STD_LOGIC;
			SW_CMD3				: IN STD_LOGIC;
			SW_CMD4				: IN STD_LOGIC;			

			DIS_CMD1				: IN STD_LOGIC;
			DIS_CMD2				: IN STD_LOGIC;
			DIS_CMD3				: IN STD_LOGIC;
			DIS_CMD4				: IN STD_LOGIC;					
			
			SBND_SYNC_CMD		: OUT STD_LOGIC;					
			SBND_ADC_CLK		: OUT STD_LOGIC					
					
	);
end SBND_PWM_CLK_ENCODER;



architecture SBND_PWM_CLK_ENCODER_arch of SBND_PWM_CLK_ENCODER is


	TYPE 	 	state_type is (S_START,S_PWM_POS , S_PWM_NEG);
	SIGNAL 	state				: state_type;	
	
	SIGNAL	CLR_RDY			: std_logic;
	SIGNAL	DLY_CNT			: std_logic_vector(7 downto 0);
	SIGNAL	CHN_CNT			: std_logic_vector(15 downto 0);
	SIGNAL	WORD_CNT			: integer range 127 downto 0;	
	SIGNAL	PW_CMD_CNT 		: std_logic_vector(7 downto 0);
	
	SIGNAL	EXT_CMD1_ACK	: std_logic;
	SIGNAL	EXT_CMD1_REQ	: std_logic;

	SIGNAL	EXT_CMD2_ACK	: std_logic;
	SIGNAL	EXT_CMD2_REQ	: std_logic;

	SIGNAL	EXT_CMD3_ACK	: std_logic;
	SIGNAL	EXT_CMD3_REQ	: std_logic;

	SIGNAL	EXT_CMD4_ACK	: std_logic;
	SIGNAL	EXT_CMD4_REQ	: std_logic;
	SIGNAL	CLK_2MHz			: std_logic;
	SIGNAL	CLK_CMD			: std_logic;
	
	SIGNAL	SW_CMD1_s1		: STD_LOGIC;
	SIGNAL	SW_CMD2_s1		: STD_LOGIC;
	SIGNAL	SW_CMD3_s1		: STD_LOGIC;
	SIGNAL	SW_CMD4_s1		: STD_LOGIC;			
	SIGNAL	SW_CMD1_s2		: STD_LOGIC;
	SIGNAL	SW_CMD2_s2		: STD_LOGIC;
	SIGNAL	SW_CMD3_s2		: STD_LOGIC;
	SIGNAL	SW_CMD4_s2		: STD_LOGIC;	
	
	SIGNAL	SW_CMD1_gen		: STD_LOGIC;
	SIGNAL	SW_CMD2_gen		: STD_LOGIC;
	SIGNAL	SW_CMD3_gen		: STD_LOGIC;
	SIGNAL	SW_CMD4_gen		: STD_LOGIC;	
		
	SIGNAL	SW_CMD_ack		: STD_LOGIC;	

	
begin
	
	
	
 process(EXT_CMD1,EXT_CMD1_ACK,RESET) 
  begin		
 
		if((RESET = '1' ) or (EXT_CMD1_ACK = '1')) then
			EXT_CMD1_REQ	<= '0';
		elsif (EXT_CMD1'event AND EXT_CMD1	 = '1') then
			EXT_CMD1_REQ	<= '1';
	  end if;
end process;
	
	
 process(EXT_CMD2,EXT_CMD2_ACK,RESET) 
  begin		
 
		if((RESET = '1' ) or (EXT_CMD2_ACK = '1')) then
			EXT_CMD2_REQ	<= '0';
		elsif (EXT_CMD2'event AND EXT_CMD2	 = '1') then
			EXT_CMD2_REQ	<= '1';
	  end if;
end process;	
	
	
 process(EXT_CMD3,EXT_CMD3_ACK,RESET) 
  begin		
 
		if((RESET = '1' ) or (EXT_CMD3_ACK = '1')) then
			EXT_CMD3_REQ	<= '0';
		elsif (EXT_CMD3'event AND EXT_CMD3	 = '1') then
			EXT_CMD3_REQ	<= '1';
	  end if;
end process;	
	
	
 process(EXT_CMD4,EXT_CMD4_ACK,RESET) 
  begin		
 
		if((RESET = '1' ) or (EXT_CMD4_ACK = '1')) then
			EXT_CMD4_REQ	<= '0';
		elsif (EXT_CMD4'event AND EXT_CMD4	 = '1') then
			EXT_CMD4_REQ	<= '1';
	  end if;
end process;	
	
	
	
process(CLK_100MHz) 
  begin		 
	if (CLK_100MHz'event AND CLK_100MHz	 = '1') then	
			SW_CMD1_s1	<= SW_CMD1;
			SW_CMD1_s2	<= SW_CMD1_s1;
			
			SW_CMD2_s1	<= SW_CMD2;
			SW_CMD2_s2	<= SW_CMD2_s1;
			
			SW_CMD3_s1	<= SW_CMD3;
			SW_CMD3_s2	<= SW_CMD3_s1;
			
			SW_CMD4_s1	<= SW_CMD4;
			SW_CMD4_s2	<= SW_CMD4_s1;	
		end if;
end process;		
	

process(CLK_100MHz,RESET,SW_CMD_ack) 
  begin		
 
		if(RESET = '1' OR SW_CMD_ack = '1') then
			SW_CMD1_gen	<= '0';
			SW_CMD2_gen	<= '0';
			SW_CMD3_gen	<= '0';
			SW_CMD4_gen	<= '0';
		elsif (CLK_100MHz'event AND CLK_100MHz	 = '1') then
			IF(SW_CMD1_s1 = '1' AND SW_CMD1_s2 = '0') THEN	
				SW_CMD1_gen	<= '1';
			END IF;
			IF(SW_CMD2_s1 = '1' AND SW_CMD2_s2 = '0') THEN	
				SW_CMD2_gen	<= '1';
			END IF;
			IF(SW_CMD3_s1 = '1' AND SW_CMD3_s2 = '0') THEN	
				SW_CMD3_gen	<= '1';
			END IF;
			IF(SW_CMD4_s1 = '1' AND SW_CMD4_s2 = '0') THEN	
				SW_CMD4_gen	<= '1';
			END IF;

		end if;
end process;	

	
  process(CLK_100MHz	,RESET) 
  begin		
 
			if(RESET = '1' ) then
				WORD_CNT			<=  0;
				PW_CMD_CNT		<=  x"19";
				EXT_CMD1_ACK 	<= '0';
				EXT_CMD2_ACK 	<= '0';
				EXT_CMD3_ACK 	<= '0';
				EXT_CMD4_ACK 	<= '0';
				CLK_2MHz			<= '1';
				CLK_CMD			<= '1';
				SW_CMD_ack		<= '0';
				state 			<= S_START;	
			elsif (CLK_100MHz'event AND CLK_100MHz	 = '1') then
				WORD_CNT	<= WORD_CNT + 1;
				CASE state IS
				when S_START =>	
					EXT_CMD1_ACK 	<= '0';
					EXT_CMD2_ACK 	<= '0';
					EXT_CMD3_ACK 	<= '0';
					EXT_CMD4_ACK 	<= '0';
					CLK_2MHz			<= '1';
					CLK_CMD			<= '1';
					SW_CMD_ack		<= '0';
					if    ((EXT_CMD1_REQ	= '1') AND (DIS_CMD1 = '0'))then
							PW_CMD_CNT 		<= x"05";
							EXT_CMD1_ACK 	<= '1';
					elsif ((EXT_CMD2_REQ	= '1') AND (DIS_CMD2 = '0')) then
							PW_CMD_CNT 		<= x"0a";
							EXT_CMD2_ACK 	<= '1';
					elsif ((EXT_CMD3_REQ	= '1') AND (DIS_CMD3 = '0')) then
							PW_CMD_CNT 		<= x"0E";
							EXT_CMD3_ACK 	<= '1';						
					elsif ((EXT_CMD4_REQ	= '1') AND (DIS_CMD4 = '0')) then
							PW_CMD_CNT 		<= x"14";
							EXT_CMD4_ACK 	<= '1';			
					elsif (SW_CMD1_gen	= '1') then
							PW_CMD_CNT 		<= x"05";	
							SW_CMD_ack		<='1';
					elsif (SW_CMD2_gen	= '1') then
							PW_CMD_CNT 		<= x"0A";	
							SW_CMD_ack		<='1';
					elsif (SW_CMD3_gen	= '1') then
							PW_CMD_CNT 		<= x"0e";			
							SW_CMD_ack		<='1';
					elsif (SW_CMD4_gen	= '1') then
							PW_CMD_CNT 		<= x"14";			
							SW_CMD_ack		<='1';
					else
							PW_CMD_CNT <=  x"00";					
					end if;		
					state 		<= S_PWM_POS;						
				when S_PWM_POS =>
					EXT_CMD1_ACK 	<= '0';
					EXT_CMD2_ACK 	<= '0';
					EXT_CMD3_ACK 	<= '0';
					EXT_CMD4_ACK 	<= '0';		
					CLK_2MHz			<= '1';
					CLK_CMD			<= '1';					
					IF(WORD_CNT >= (24-PW_CMD_CNT)) then
						CLK_CMD			<= '0';
					END IF;
					IF(WORD_CNT >= 24) then
						CLK_2MHz			<= '0';
					END IF;
					IF(WORD_CNT >= 49) then
						WORD_CNT			<=  0;
						CLK_2MHz			<= '1';
						CLK_CMD			<= '1';
						state 		<= S_PWM_NEG;	
					END IF;
				when S_PWM_NEG =>
					CLK_2MHz			<= '1';
					CLK_CMD			<= '1';					
					IF(WORD_CNT >= (24+PW_CMD_CNT)) then
						CLK_CMD			<= '0';
					END IF;
					IF(WORD_CNT >= 24) then
						CLK_2MHz			<= '0';
					END IF;
					IF(WORD_CNT >= 49) then
						WORD_CNT			<=  0;
						CLK_2MHz			<= '1';
						CLK_CMD			<= '1';							
						state 		<= S_START;	
					END IF;
				when others =>		
					state 	<= S_START;	
				end case; 			

	  end if;
end process;


SBND_ADC_CLK	<= CLK_2MHz;
SBND_SYNC_CMD	<= CLK_CMD;

end SBND_PWM_CLK_ENCODER_arch;


