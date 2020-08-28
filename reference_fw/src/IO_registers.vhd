--/////////////////////////////////////////////////////////////////////
--////                              
--////  File: IO_registers.VHD
--////                                                                                                                                      
--////  Author: Jack Fried                                        
--////          jfried@bnl.gov                
--////  Created:  5/19/2005
--////  Modified: 11/09/2016
--////  Description:  
--////                                  
--////
--/////////////////////////////////////////////////////////////////////
--////
--//// Copyright (C) 2014 Brookhaven National Laboratory
--////
--/////////////////////////////////////////////////////////////////////


LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.std_logic_arith.all;
USE ieee.std_logic_unsigned.all;


--  Entity Declaration

ENTITY IO_registers IS

	PORT
	(
		rstn        : IN STD_LOGIC;				-- state machine reset
		clk         : IN STD_LOGIC;
		data        : IN STD_LOGIC_VECTOR(31 downto 0);	
		WR_address  : IN STD_LOGIC_VECTOR(7 downto 0); 
		RD_address  : IN STD_LOGIC_VECTOR(7 downto 0); 
		WR    	 	: IN STD_LOGIC;				
		data_out	: OUT  STD_LOGIC_VECTOR(31 downto 0);				
		reg0_i		: IN  STD_LOGIC_VECTOR(31 downto 0);		
		reg1_i		: IN  STD_LOGIC_VECTOR(31 downto 0);	
		reg2_i		: IN  STD_LOGIC_VECTOR(31 downto 0);	
		reg3_i		: IN  STD_LOGIC_VECTOR(31 downto 0);	
		reg4_i		: IN  STD_LOGIC_VECTOR(31 downto 0);		
		reg5_i		: IN  STD_LOGIC_VECTOR(31 downto 0);	
		reg6_i		: IN  STD_LOGIC_VECTOR(31 downto 0);	
		reg7_i		: IN  STD_LOGIC_VECTOR(31 downto 0);	
		reg8_i		: IN  STD_LOGIC_VECTOR(31 downto 0);		
		reg9_i		: IN  STD_LOGIC_VECTOR(31 downto 0);	
		reg10_i		: IN  STD_LOGIC_VECTOR(31 downto 0);	
		reg11_i		: IN  STD_LOGIC_VECTOR(31 downto 0);	
		reg12_i		: IN  STD_LOGIC_VECTOR(31 downto 0);		
		reg13_i		: IN  STD_LOGIC_VECTOR(31 downto 0);	
		reg14_i		: IN  STD_LOGIC_VECTOR(31 downto 0);	
		reg15_i		: IN  STD_LOGIC_VECTOR(31 downto 0);	
		reg16_i	    : IN  STD_LOGIC_VECTOR(31 downto 0);	
		
		reg0_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);		
		reg1_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);	
		reg2_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);	
		reg3_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);	
		reg4_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);		
		reg5_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);	
		reg6_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);	
		reg7_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);	
		reg8_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);		
		reg9_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);	
		reg10_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);	
		reg11_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);	
		reg12_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);		
		reg13_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);	
		reg14_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);	
		reg15_o		: OUT  STD_LOGIC_VECTOR(31 downto 0);
		reg16_o	    : OUT  STD_LOGIC_VECTOR(31 downto 0)
	);
	
END IO_registers;


ARCHITECTURE behavior OF IO_registers IS


begin

  data_out		<=	     reg0_i 	when (RD_address(7 downto 0) = x"00")	else
						 reg1_i 	when (RD_address(7 downto 0) = x"01")	else
						 reg2_i 	when (RD_address(7 downto 0) = x"02")	else
						 reg3_i 	when (RD_address(7 downto 0) = x"03")	else
						 reg4_i 	when (RD_address(7 downto 0) = x"04")	else
						 reg5_i 	when (RD_address(7 downto 0) = x"05")	else
						 reg6_i 	when (RD_address(7 downto 0) = x"06")	else
						 reg7_i 	when (RD_address(7 downto 0) = x"07")	else
						 reg8_i 	when (RD_address(7 downto 0) = x"08")	else
						 reg9_i 	when (RD_address(7 downto 0) = x"09")	else
						 reg10_i	when (RD_address(7 downto 0) = x"0a")	else
						 reg11_i	when (RD_address(7 downto 0) = x"0b")	else
						 reg12_i	when (RD_address(7 downto 0) = x"0c")	else
						 reg13_i	when (RD_address(7 downto 0) = x"0d")	else
						 reg14_i	when (RD_address(7 downto 0) = x"0e")	else
						 reg15_i	when (RD_address(7 downto 0) = x"0f")	else
						 reg16_i	when (RD_address(7 downto 0) = x"10")	else				 
						 X"00000000";
				 
					 
  process(clk,WR,rstn) 
  begin
		if (rstn = '0') then
			reg0_o		<= X"00000000";			
			reg1_o		<= X"00000000";	
			reg2_o		<= X"00000000";	
			reg3_o		<= X"00000000";	
			reg4_o		<= X"00000000";	
			reg5_o		<= X"00000000";		
			reg6_o		<= X"00000000";	
			reg7_o		<= X"00000000";	
			reg8_o		<= X"00000000";			
			reg9_o		<= X"00000000";	
			reg10_o		<= X"00000000";
			reg11_o		<= X"00000000";	
			reg12_o		<= X"00000000";		
			reg13_o		<= X"00000000";
			reg14_o		<= X"00000000";	
			reg15_o		<= X"00000000";	
			reg16_o	    <= X"00000000";		
			
		elsif (clk'event  AND  clk = '1') then
			if (WR = '1') then
				CASE WR_address(7 downto 0) IS
					when x"00" => 	reg0_o   <= data;
					when x"01" => 	reg1_o   <= data;	
					when x"02" => 	reg2_o   <= data;
					when x"03" => 	reg3_o   <= data;
					when x"04" => 	reg4_o   <= data;
					when x"05" => 	reg5_o   <= data;
					when x"06" => 	reg6_o   <= data;
					when x"07" => 	reg7_o   <= data;
					when x"08" => 	reg8_o   <= data;
					when x"09" => 	reg9_o   <= data;	
					when x"0A" => 	reg10_o   <= data;
					when x"0B" => 	reg11_o   <= data;
					when x"0C" => 	reg12_o   <= data;
					when x"0D" => 	reg13_o   <= data;
					when x"0E" => 	reg14_o   <= data;
					when x"0F" => 	reg15_o   <= data;
					when x"10" => 	reg16_o   <= data;
					WHEN OTHERS =>  
				end case;  
			 end if;
		end if;
end process;
	

END behavior;
