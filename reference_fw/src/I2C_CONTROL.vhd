----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 12/22/2019 11:14:35 AM
-- Design Name: 
-- Module Name: I2C_CONTROL - Behavioral
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

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity I2C_CONTROL is
 Port ( 
 
          I2C_SELECT                : IN STD_LOGIC_VECTOR(7 downto 0);
          
          iic_rtl_0_scl_i           : OUT STD_LOGIC;
          iic_rtl_0_scl_o           : IN STD_LOGIC;
          iic_rtl_0_scl_t           : IN STD_LOGIC;
          
          iic_rtl_0_sda_i           : OUT STD_LOGIC;
          iic_rtl_0_sda_o           : IN STD_LOGIC;
          iic_rtl_0_sda_t           : IN STD_LOGIC;
 
  
          SI5344_SCL                : INOUT STD_LOGIC;                --    2.5V, default
          SI5344_SDA                : INOUT STD_LOGIC;                --    2.5V, default
    
          SI5342_SCL                : INOUT STD_LOGIC;                --    2.5V, default
          SI5342_SDA                : INOUT STD_LOGIC;                --    2.5V, default
    
          QSFP_SCL                  : INOUT STD_LOGIC;                --    3.3V, default    
          QSFP_SDA                  : INOUT STD_LOGIC;                --    3.3V, default
    
          PL_FEMB_PWR_SCL           : INOUT STD_LOGIC;                --    3.3V, PWR I2C clk control
          PL_FEMB_PWR_SDA           : INOUT STD_LOGIC;                --    3.3V, PWR I2C SDA control
     
          PL_FEMB_EN_SCL            : INOUT STD_LOGIC;                --    3.3V, PWR I2C clk control
          PL_FEMB_EN_SDA            : INOUT STD_LOGIC;                --    3.3V, PWR I2C SDA control 
    
          SENSOR_I2C_SCL            : INOUT STD_LOGIC;                 --    3.3V,
          SENSOR_I2C_SDA            : INOUT STD_LOGIC;                 --    3.3V,
    
          PL_FEMB_PWR2_SCL          : INOUT STD_LOGIC;                 --    3.3V,
          PL_FEMB_PWR2_SDA          : INOUT STD_LOGIC;                 --    3.3V,
    
          LTC2977_SDA               : INOUT STD_LOGIC;                 --    3.3V,
          LTC2977_SCL               : INOUT STD_LOGIC;                 --    3.3V,
    
          PL_FEMB_PWR3_SCL          : INOUT STD_LOGIC;                 --    1.8V, default 
          PL_FEMB_PWR3_SDA          : INOUT STD_LOGIC;                 --    1.8V, default 
          
          FLASH_SCL                 : INOUT STD_LOGIC;                 --    2.5V,  24lc64
          FLASH_SDA                 : INOUT STD_LOGIC;                 --    2.5V,  24lc64

          ADN2814_SCL               : INOUT STD_LOGIC;              --    3.3V,  
          ADN2814_SDA               : INOUT STD_LOGIC              --    3.3V, 

 );
end I2C_CONTROL;

architecture Behavioral of I2C_CONTROL is


  component IOBUF is
  port (
    I : in STD_LOGIC;
    O : out STD_LOGIC;
    T : in STD_LOGIC;
    IO : inout STD_LOGIC
  );
  end component IOBUF;



SIGNAL  SI5344_SCL_i                : STD_LOGIC;                --    2.5V, default
SIGNAL  SI5344_SDA_i                : STD_LOGIC;                --    2.5V, default
SIGNAL  SI5344_SDA_o                : STD_LOGIC;                --    2.5V, default

SIGNAL  SI5342_SCL_i                : STD_LOGIC;                --    2.5V, default
SIGNAL  SI5342_SDA_i                : STD_LOGIC;                --    2.5V, default
SIGNAL  SI5342_SDA_o                : STD_LOGIC;                --    2.5V, default

SIGNAL  QSFP_SCL_i                  : STD_LOGIC;                --    3.3V, default    
SIGNAL  QSFP_SDA_i                  : STD_LOGIC;                --    3.3V, default
SIGNAL  QSFP_SDA_o                  : STD_LOGIC;                --    3.3V, default

SIGNAL  PL_FEMB_PWR_SCL_i           : STD_LOGIC;                --    3.3V, PWR I2C clk control
SIGNAL  PL_FEMB_PWR_SDA_i           : STD_LOGIC;                --    3.3V, PWR I2C SDA control 
SIGNAL  PL_FEMB_PWR_SDA_o           : STD_LOGIC;                --    3.3V, PWR I2C SDA control
 
SIGNAL  PL_FEMB_EN_SCL_i            : STD_LOGIC;                --    3.3V, PWR I2C clk control
SIGNAL  PL_FEMB_EN_SDA_i            : STD_LOGIC;                --    3.3V, PWR I2C SDA control 
SIGNAL  PL_FEMB_EN_SDA_o            : STD_LOGIC;                --    3.3V, PWR I2C SDA control 


SIGNAL  SENSOR_I2C_SCL_i            : STD_LOGIC;                 --    3.3V,
SIGNAL  SENSOR_I2C_SDA_i            : STD_LOGIC;                 --    3.3V,
SIGNAL  SENSOR_I2C_SDA_o            : STD_LOGIC;                 --    3.3V,


SIGNAL  PL_FEMB_PWR2_SCL_i          : STD_LOGIC;                 --    3.3V,
SIGNAL  PL_FEMB_PWR2_SDA_i          : STD_LOGIC;                 --    3.3V,
SIGNAL  PL_FEMB_PWR2_SDA_o          : STD_LOGIC;                 --    3.3V,


SIGNAL  LTC2977_SDA_i               : STD_LOGIC;                 --    3.3V,
SIGNAL  LTC2977_SCL_i               : STD_LOGIC;                 --    3.3V,
SIGNAL  LTC2977_SCL_o               : STD_LOGIC;                 --    3.3V,


SIGNAL  PL_FEMB_PWR3_SCL_i          : STD_LOGIC;                 --    1.8V, default 
SIGNAL  PL_FEMB_PWR3_SDA_i          : STD_LOGIC;                 --    1.8V, default       
SIGNAL  PL_FEMB_PWR3_SDA_o          : STD_LOGIC;                 --    1.8V, default       
        

SIGNAL  FLASH_SCL_i                 : STD_LOGIC;                 --    2.5V,  24lc64
SIGNAL  FLASH_SDA_i                 : STD_LOGIC;                 --    2.5V,  24lc64
SIGNAL  FLASH_SDA_o                 : STD_LOGIC;                 --    2.5V,  24lc64


SIGNAL  ADN2814_SCL_i                 : STD_LOGIC;                 --    2.5V,  24lc64
SIGNAL  ADN2814_SDA_i                 : STD_LOGIC;                 --    2.5V,  24lc64
SIGNAL  ADN2814_SDA_o                 : STD_LOGIC;                 --    2.5V,  24lc64

begin

   iic_rtl_0_scl_i           <= SI5344_SCL_i        when(I2C_SELECT = x"00") else
                                SI5342_SCL_i        when(I2C_SELECT = x"01") else
                                QSFP_SCL_i          when(I2C_SELECT = x"02") else
                                PL_FEMB_PWR_SCL_i   when(I2C_SELECT = x"03") else
                                PL_FEMB_EN_SCL_i    when(I2C_SELECT = x"04") else
                                SENSOR_I2C_SCL_i    when(I2C_SELECT = x"05") else
                                PL_FEMB_PWR2_SCL_i  when(I2C_SELECT = x"06") else
                                LTC2977_SCL_i       when(I2C_SELECT = x"07") else
                                PL_FEMB_PWR3_SCL_i  when(I2C_SELECT = x"08") else
                                FLASH_SCL_i         when(I2C_SELECT = x"09") else
                                ADN2814_SCL_i       when(I2C_SELECT = x"0A") else
                                '1';
   
   iic_rtl_0_sda_i           <= SI5344_SDA_i        when(I2C_SELECT = x"00") else
                                SI5342_SDA_i        when(I2C_SELECT = x"01") else
                                QSFP_SDA_i          when(I2C_SELECT = x"02") else
                                PL_FEMB_PWR_SDA_i   when(I2C_SELECT = x"03") else
                                PL_FEMB_EN_SDA_i    when(I2C_SELECT = x"04") else
                                SENSOR_I2C_SDA_i    when(I2C_SELECT = x"05") else
                                PL_FEMB_PWR2_SDA_i  when(I2C_SELECT = x"06") else
                                LTC2977_SDA_i       when(I2C_SELECT = x"07") else
                                PL_FEMB_PWR3_SDA_i  when(I2C_SELECT = x"08") else
                                FLASH_SDA_i         when(I2C_SELECT = x"09") else
                                ADN2814_SDA_i       when(I2C_SELECT = x"0A") else
                                '1';
                            
                            
 SI5344_SDA_o        <= iic_rtl_0_sda_o when(I2C_SELECT = x"00") else  '1';
 SI5342_SDA_o        <= iic_rtl_0_sda_o when(I2C_SELECT = x"01") else  '1';
 QSFP_SDA_o          <= iic_rtl_0_sda_o when(I2C_SELECT = x"02") else  '1';
 PL_FEMB_PWR_SDA_o   <= iic_rtl_0_sda_o when(I2C_SELECT = x"03") else  '1';
 PL_FEMB_EN_SDA_o    <= iic_rtl_0_sda_o when(I2C_SELECT = x"04") else  '1';
 SENSOR_I2C_SDA_o    <= iic_rtl_0_sda_o when(I2C_SELECT = x"05") else  '1';                         
 PL_FEMB_PWR2_SDA_o  <= iic_rtl_0_sda_o when(I2C_SELECT = x"06") else  '1';                     
 LTC2977_SCL_o       <= iic_rtl_0_sda_o when(I2C_SELECT = x"07") else  '1';                
 PL_FEMB_PWR3_SDA_o  <= iic_rtl_0_sda_o when(I2C_SELECT = x"08") else  '1';                       
 FLASH_SDA_o         <= iic_rtl_0_sda_o when(I2C_SELECT = x"09") else  '1';                
 ADN2814_SDA_o       <= iic_rtl_0_sda_o when(I2C_SELECT = x"0A") else  '1';      
 
                      
                                
SI5344_SCL_iobuf:       component IOBUF  port map (IO => SI5344_SCL,I => iic_rtl_0_scl_o,O => SI5344_SCL_i,T => iic_rtl_0_scl_t );
SI5344_SDA_iobuf:       component IOBUF  port map (IO => SI5344_SDA,I => SI5344_SDA_o,O => SI5344_SDA_i,T => iic_rtl_0_sda_t );	

SI5342_SCL_iobuf:       component IOBUF  port map (IO => SI5342_SCL,I => iic_rtl_0_scl_o,O => SI5342_SCL_i,T => iic_rtl_0_scl_t );
SI5342_SDA_iobuf:       component IOBUF  port map (IO => SI5342_SDA,I => SI5342_SDA_o,O => SI5342_SDA_i,T => iic_rtl_0_sda_t );	

QSFP_SCL_iobuf:         component IOBUF  port map (IO => QSFP_SCL,I => iic_rtl_0_scl_o,O => QSFP_SCL_i,T => iic_rtl_0_scl_t );
QSFP_SDA_iobuf:         component IOBUF  port map (IO => QSFP_SDA,I => QSFP_SDA_o,O => QSFP_SDA_i,T => iic_rtl_0_sda_t );	

PL_FEMB_PWR_SCL_iobuf:  component IOBUF  port map (IO => PL_FEMB_PWR_SCL,I => iic_rtl_0_scl_o,O => PL_FEMB_PWR_SCL_i,T => iic_rtl_0_scl_t );
PL_FEMB_PWR_SDA_iobuf:  component IOBUF  port map (IO => PL_FEMB_PWR_SDA,I => PL_FEMB_PWR_SDA_o,O => PL_FEMB_PWR_SDA_i,T => iic_rtl_0_sda_t );	

PL_FEMB_EN_SCL_iobuf:   component IOBUF  port map (IO => PL_FEMB_EN_SCL,I => iic_rtl_0_scl_o,O => PL_FEMB_EN_SCL_i,T => iic_rtl_0_scl_t );
PL_FEMB_EN_SDA_iobuf:   component IOBUF  port map (IO => PL_FEMB_EN_SDA,I => PL_FEMB_EN_SDA_o,O => PL_FEMB_EN_SDA_i,T => iic_rtl_0_sda_t );	
    
SENSOR_I2C_SCL_iobuf:   component IOBUF  port map (IO => SENSOR_I2C_SCL,I => iic_rtl_0_scl_o,O => SENSOR_I2C_SCL_i,T => iic_rtl_0_scl_t );
SENSOR_I2C_SDA_iobuf:   component IOBUF  port map (IO => SENSOR_I2C_SDA,I => SENSOR_I2C_SDA_o,O => SENSOR_I2C_SDA_i,T => iic_rtl_0_sda_t );	

PL_FEMB_PWR2_SCL_iobuf: component IOBUF  port map (IO => PL_FEMB_PWR2_SCL,I => iic_rtl_0_scl_o, O => PL_FEMB_PWR2_SCL_i,T => iic_rtl_0_scl_t );
PL_FEMB_PWR2_SDA_iobuf: component IOBUF  port map (IO => PL_FEMB_PWR2_SDA,I => PL_FEMB_PWR2_SDA_o,O => PL_FEMB_PWR2_SDA_i,T => iic_rtl_0_sda_t );	

LTC2977_SCL_iobuf:      component IOBUF  port map (IO => LTC2977_SCL,I => iic_rtl_0_scl_o,O => LTC2977_SCL_i,T => iic_rtl_0_scl_t );
LTC2977_SDA_iobuf:      component IOBUF  port map (IO => LTC2977_SDA,I => LTC2977_SCL_o,O => LTC2977_SDA_i,T => iic_rtl_0_sda_t );	

PL_FEMB_PWR3_SCL_iobuf: component IOBUF  port map (IO => PL_FEMB_PWR3_SCL,I => iic_rtl_0_scl_o,O => PL_FEMB_PWR3_SCL_i,T => iic_rtl_0_scl_t );
PL_FEMB_PWR3_SDA_iobuf: component IOBUF  port map (IO => PL_FEMB_PWR3_SDA,I => PL_FEMB_PWR3_SDA_o,O => PL_FEMB_PWR3_SDA_i,T => iic_rtl_0_sda_t );	

FLASH_SCL_iobuf:        component IOBUF  port map (IO => FLASH_SCL,   I => iic_rtl_0_scl_o,   O => FLASH_SCL_i,    T => iic_rtl_0_scl_t );
FLASH_SDA_iobuf:        component IOBUF  port map (IO => FLASH_SDA,   I => FLASH_SDA_o,       O => FLASH_SDA_i,    T => iic_rtl_0_sda_t );	

ADN2814_SCL_iobuf:      component IOBUF  port map (IO => ADN2814_SCL,  I => iic_rtl_0_scl_o,  O => ADN2814_SCL_i,  T => iic_rtl_0_scl_t );
ADN2814_SDA_iobuf:      component IOBUF  port map (IO => ADN2814_SDA,  I => ADN2814_SDA_o,    O => ADN2814_SDA_i,  T => iic_rtl_0_sda_t );	

end Behavioral;
