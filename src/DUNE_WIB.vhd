----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 12/06/2019 03:16:27 PM
-- Design Name: 
-- Module Name: DUNE_WIB - Behavioral
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




entity DUNE_WIB is
port(
--    WIB_LED_tri_o    : out STD_LOGIC_VECTOR ( 7 downto 0 );
--    iic_rtl_0_scl_io : inout STD_LOGIC;
--    iic_rtl_0_sda_io : inout STD_LOGIC;
    


    
      CLK_IN_50MHz           : IN    STD_LOGIC;        --    2.5V, default 50MHz  
      CORE_CLK1_P            : IN    STD_LOGIC;        --    LVDS
      CORE_CLK1_N            : IN    STD_LOGIC;        --    LVDS
      CORE_CLK2_P            : IN    STD_LOGIC;        --    LVDS
      CORE_CLK2_N            : IN    STD_LOGIC;        --    LVDS
      CORE_CLK3_P            : IN    STD_LOGIC;        --    LVDS
      CORE_CLK3_N            : IN    STD_LOGIC;        --    LVDS
     
      FPGA_CLK_N             : IN    STD_LOGIC;        --    LVDS
      FPGA_CLK_P             : IN    STD_LOGIC;        --    LVDS
      

          
          
          -- FEMB MGT
          
    FEMB0_TXn								: OUT STD_LOGIC_VECTOR(3 downto 0); 
	FEMB0_TXp								: OUT STD_LOGIC_VECTOR(3 downto 0); 		
	FEMB0_RXn								: IN STD_LOGIC_VECTOR(3 downto 0); 
	FEMB0_RXp								: IN STD_LOGIC_VECTOR(3 downto 0); 
	
    FEMB0_refclk0_p                         : IN STD_LOGIC; 
    FEMB0_refclk0_n                         : IN STD_LOGIC; 	
          
            -- DUNE TIMING SYSTEM 
  
      FP_SFP_SEL                : OUT   STD_LOGIC;                    -- 2.5V
           
      TX_TIMING_P               : OUT   STD_LOGIC;                    -- LVDS     
      TX_TIMING_N               : OUT   STD_LOGIC;                    -- LVDS           
      TX_TIMING_DISABLE         : OUT   STD_LOGIC;                    --    2.5V, default
      RX_TIMING_SEL             : OUT   STD_LOGIC;                    --    2.5V, default 
    
      ADN2814_DATA_P            : IN    STD_LOGIC;                    --    LVDS ,
      ADN2814_DATA_N            : IN    STD_LOGIC;                    --    LVDS ,       
           
      ADN2814_LOL               : IN    STD_LOGIC;                    --    3.3V, default 16MHz  EXTRA SYSTEM CLOCK
      ADN2814_LOS               : IN    STD_LOGIC;                    --    3.3V, default 16MHz  EXTRA SYSTEM CLOCK
      ADN2814_SDA               : INOUT STD_LOGIC;                    --    3.3V, default 16MHz  EXTRA SYSTEM CLOCK
      ADN2814_SCK               : INOUT STD_LOGIC;                    --    3.3V, default 16MHz  EXTRA SYSTEM CLOCK     
      
   
          -- DUNE SYSTEM CLOCKS & SYNCRONUS COMMANDS
         
      MGT_CLK_SEL               : OUT   STD_LOGIC;                    --    2.5V, default
         
      FEMB_CMD_SEL              : OUT   STD_LOGIC;                    --    2.5V, default
      FEMB_CLK_SEL              : OUT   STD_LOGIC;                    --    2.5V, default   
       

                
      FEMB_CMD_FPGA_OUT_P       : OUT  STD_LOGIC;                    --    LVDS ,  
      FEMB_CMD_FPGA_OUT_N       : OUT  STD_LOGIC;                    --    LVDS 
     
      DUNE_CLK_FPGA_OUT_P       : OUT  STD_LOGIC;                    --    LVDS ,  
      DUNE_CLK_FPGA_OUT_N       : OUT  STD_LOGIC;                    --    LVDS ,     
  
      DUNE_CLK_FPGA_IN_P        : IN  STD_LOGIC;                    --    LVDS ,
      DUNE_CLK_FPGA_IN_N        : IN  STD_LOGIC;                    --    LVDS ,
  
  
    --        WIB DUNE BACK PLANE SIGNALS
    

      BP_CRATE_ADDR             : IN     STD_LOGIC_VECTOR(3 downto 0);      --    2.5V, default
      BP_SLOT_ADDR              : IN     STD_LOGIC_VECTOR(3 downto 0);      --    1.8V, default    
      BP_IO                     : INOUT  STD_LOGIC_VECTOR(7 downto 0);      --    2.5V, default

   
      
          --    SI5344 clock control
          
      SI5344_SCL                : INOUT STD_LOGIC;                --    2.5V, default
      SI5344_SDA                : INOUT STD_LOGIC;                --    2.5V, default
      SI5344_INTR               : INOUT STD_LOGIC;                --    2.5V, default
      SI5344_SEL0               : OUT   STD_LOGIC;                --    2.5V, default
      SI5344_SEL1               : OUT   STD_LOGIC;                --    2.5V, default
      SI5344_RST                : OUT   STD_LOGIC;                --    2.5V, default
      SI5344_OE                 : OUT   STD_LOGIC;                --    2.5V, default    
      SI5344_lol                : IN    STD_LOGIC;                --    1.8V, default    
      SI5344_LOSXAXB            : IN    STD_LOGIC;                --    1.8V, default    
      
      SI5344_OUT0_P             : IN     STD_LOGIC;                  --    LVDS ,      
      SI5344_OUT0_N             : IN     STD_LOGIC;                  --    LVDS ,      
      SI5344_OUT1_P             : IN     STD_LOGIC;                  --    LVDS ,       
      SI5344_OUT1_N             : IN     STD_LOGIC;                  --    LVDS ,          
      
      
          --    SI5342 clock control
          
      SI5342_SCL                : INOUT STD_LOGIC;                --    2.5V, default
      SI5342_SDA                : INOUT STD_LOGIC;                --    2.5V, default
      SI5342_INTR               : INOUT STD_LOGIC;                --    2.5V, default
      SI5342_SEL0               : OUT   STD_LOGIC;                --    2.5V, default
      SI5342_SEL1               : OUT   STD_LOGIC;                --    2.5V, default    
      SI5342_RST                : OUT   STD_LOGIC;                --    2.5V, default        
      SI5342_OE                 : OUT   STD_LOGIC;                --    2.5V, default    
      SI5342_lol                : IN    STD_LOGIC;                --    1.8V, default    
      SI5342_LOSXAXB            : IN    STD_LOGIC;                --    1.8V, default  
      SI5342_LOS0               : IN    STD_LOGIC;                --    1.8V, default    
      SI5342_LOS1               : IN    STD_LOGIC;                --    1.8V, default    
      SI5342_LOS2               : IN    STD_LOGIC;                --    1.8V, default    
      SI5342_LOS3               : IN    STD_LOGIC;                --    1.8V, default        
      
      RECOV_CLK_P               : OUT   STD_LOGIC;                --    LVDS ,
      RECOV_CLK_N               : OUT   STD_LOGIC;                --    LVDS ,      
      
          --    HIGH SPEED  GIG-E LINK
  
--      SFP_rx                    : IN   STD_LOGIC;                    --    1.5-V PCML, GIG-E  RX
--      SFP_tx                    : OUT  STD_LOGIC;                    --    1.5-V PCML, GIG-E  TX
      
          --    HIGH SPEED  DAQ LINK
          
  
  --    QSFP_TX                   : OUT    STD_LOGIC_VECTOR(3 downto 0);        --    1.5-V PCML, DAQ Transmit Data
--      QSFP_RX                   : IN     STD_LOGIC_VECTOR(3 downto 0);        --    1.5-V PCML, DAQ Transmit Data
      QSFP_MODE                 : OUT    STD_LOGIC;                --    3.3V, default
      QSFP_SEL                  : OUT    STD_LOGIC;                --    3.3V, default        
      QSFP_RST                  : OUT    STD_LOGIC;                --    3.3V, default
      QSFP_SCL                  : INOUT  STD_LOGIC;                --    3.3V, default    
      QSFP_SDA                  : INOUT  STD_LOGIC;                --    3.3V, default
      QSFP_INTn                 : INOUT  STD_LOGIC;                --    3.3V, default
      QSFP_PRSN                 : INOUT  STD_LOGIC;                --    3.3V, default    
          --    HIGH SPEED  FEMB LINK
  
 --     FEMB_GXB_RX               : IN     STD_LOGIC_VECTOR(15 downto 0);    --    1.5-V PCML, Cold electronics board reciver
  
          
           --    ZYNQ UART         
          
      UART_nRST    : OUT    STD_LOGIC;                --    3.3V, default  
          
          
          --  WIB-FEMB CMD , CLOCK & CONTROL INTERFACE
          
      FEMB_BRD0_IO_P            : INOUT STD_LOGIC_VECTOR(5 downto 0);                --    LVDS ,    FEMB DIFF I2C  
      FEMB_BRD0_IO_N            : INOUT STD_LOGIC_VECTOR(5 downto 0);                --    LVDS ,    FEMB DIFF I2C 

      FEMB_BRD1_IO_P            : INOUT STD_LOGIC_VECTOR(5 downto 0);                --    LVDS ,    FEMB DIFF I2C  
      FEMB_BRD1_IO_N            : INOUT STD_LOGIC_VECTOR(5 downto 0);                --    LVDS ,    FEMB DIFF I2C  

      FEMB_BRD2_IO_P            : INOUT STD_LOGIC_VECTOR(5 downto 0);                --    LVDS ,    FEMB DIFF I2C  
      FEMB_BRD2_IO_N            : INOUT STD_LOGIC_VECTOR(5 downto 0);                --    LVDS ,    FEMB DIFF I2C  

      FEMB_BRD3_IO_P            : INOUT STD_LOGIC_VECTOR(5 downto 0);                --    LVDS ,    FEMB DIFF I2C  
      FEMB_BRD3_IO_N            : INOUT STD_LOGIC_VECTOR(5 downto 0);                --    LVDS ,    FEMB DIFF I2C 
   
      EQ_LOS_BRD0_RX            : IN     STD_LOGIC_VECTOR(3 downto 0);               --    3.3V, default
      EQ_LOS_BRD1_RX            : IN     STD_LOGIC_VECTOR(3 downto 0);               --    3.3V, default
      EQ_LOS_BRD2_RX            : IN     STD_LOGIC_VECTOR(3 downto 0);               --    3.3V, default
      EQ_LOS_BRD3_RX            : IN     STD_LOGIC_VECTOR(3 downto 0);               --    3.3V, default

       
      -- WIB FEMB POWER MONITOR     
       
      PL_FEMB_PWR_SCL           : INOUT  STD_LOGIC;                --    3.3V, PWR I2C clk control
      PL_FEMB_PWR_SDA           : INOUT  STD_LOGIC;                --    3.3V, PWR I2C SDA control 
      PL_FEMB_EN_SCL            : INOUT  STD_LOGIC;                --    3.3V, PWR I2C clk control
      PL_FEMB_EN_SDA            : INOUT  STD_LOGIC;                --    3.3V, PWR I2C SDA control 

      -- WIB POWER
      
      PS_DIMM_SUSPEND_EN        : OUT    STD_LOGIC;                 --    1.8V, default 
      PS_DIMM_VDDQ_TO_PSVCCO_ON : OUT    STD_LOGIC;                 --    3.3V,
      
      SENSOR_I2C_SCL            : INOUT  STD_LOGIC;                 --    3.3V,
      SENSOR_I2C_SDA            : INOUT  STD_LOGIC;                 --    3.3V,
      PL_FEMB_PWR2_SCL          : INOUT  STD_LOGIC;                 --    3.3V,
      PL_FEMB_PWR2_SDA          : INOUT  STD_LOGIC;                 --    3.3V,
      PWR_GOOD                  : IN     STD_LOGIC;                 --    3.3V,
      LTC2977_SDA               : INOUT  STD_LOGIC;                 --    3.3V,
      LTC2977_SCL               : INOUT  STD_LOGIC;                 --    3.3V,

      TEMP_S1_ALERT             : IN     STD_LOGIC;                 --    1.8V, default 
      TEMP_S2_ALERT             : IN     STD_LOGIC;                 --    1.8V, default 
      TEMP_S3_ALERT             : IN     STD_LOGIC;                 --    1.8V, default 
      PL_FEMB_PWR3_SCL          : INOUT  STD_LOGIC;                 --    1.8V, default 
      PL_FEMB_PWR3_SDA          : INOUT  STD_LOGIC;                 --    1.8V, default 
      
      
      --WIB AMON


      MON_ADC_SCK               : OUT    STD_LOGIC;                --    3.3V,
      MON_ADC_CS                : OUT    STD_LOGIC;                --    3.3V,
      MON_ADC_SDO0              : IN     STD_LOGIC;                --    3.3V,
      MON_ADC_SDO1              : IN     STD_LOGIC;                --    3.3V,
      MON_ADC_SDO2              : IN     STD_LOGIC;                --    3.3V,
      MON_ADC_SDO3              : IN     STD_LOGIC;                --    2.5V, default

      -- WIB MISC_IO
          
      RESET_EXT                 : IN     STD_LOGIC;                     --    3.3V,  MAX811    
      LEMO_IO                   : INOUT  STD_LOGIC_VECTOR(1 downto 0);  --    1.8V,  LEMO_FRNT PANNEL     
      LEMO_DIR                  : OUT    STD_LOGIC_VECTOR(1 downto 0);  --    1.8V,  LEMO_FRNT PANNEL       
      FP_IO                     : INOUT  STD_LOGIC_VECTOR(3 downto 0);   --    2.5V,
      MISC_IO                   : OUT    STD_LOGIC_VECTOR(15 downto 0);     --    2.5V, 
      DIP_SW                    : IN     STD_LOGIC_VECTOR(7 downto 0);      --    1.8,             -- USED FOR IP HARD CODE
      FLASH_SCL                 : INOUT  STD_LOGIC;                         --    2.5V,  24lc64
      FLASH_SDA                 : INOUT  STD_LOGIC;                         --    2.5V,  24lc64
      WIB_LED                   : OUT    STD_LOGIC_VECTOR(7 downto 0)       --    2.5V   
    
    
    
    
  );
end DUNE_WIB;

architecture Behavioral of DUNE_WIB is



component ZYNQ_TOP_wrapper is
  port (
    AXI_CLK_OUT : out STD_LOGIC;
    AXI_RSTn : out STD_LOGIC_VECTOR ( 0 to 0 );
    Data_IN : in STD_LOGIC_VECTOR ( 31 downto 0 );
    Data_OUT : out STD_LOGIC_VECTOR ( 31 downto 0 );
    RD_addr : out STD_LOGIC_VECTOR ( 10 downto 0 );
    RD_strb : out STD_LOGIC;
    WIB_LED_tri_o : out STD_LOGIC_VECTOR ( 7 downto 0 );
    WR_addr : out STD_LOGIC_VECTOR ( 10 downto 0 );
    WR_strb : out STD_LOGIC;
    iic_rtl_0_scl_i : in STD_LOGIC;
    iic_rtl_0_scl_o : out STD_LOGIC;
    iic_rtl_0_scl_t : out STD_LOGIC;
    iic_rtl_0_sda_i : in STD_LOGIC;
    iic_rtl_0_sda_o : out STD_LOGIC;
    iic_rtl_0_sda_t : out STD_LOGIC
    
  );
end component ZYNQ_TOP_wrapper;


component  WIB_FEMB_COMM_TOP IS
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
end component WIB_FEMB_COMM_TOP;

component FEMB_GTH_top is
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
end component FEMB_GTH_top;

  component IOBUF is
  port (
    I : in STD_LOGIC;
    O : out STD_LOGIC;
    T : in STD_LOGIC;
    IO : inout STD_LOGIC
  );
  end component IOBUF;
  
  
 component  clk_wiz_0 is
  Port ( 
    clk_out1 : out STD_LOGIC;
    locked : out STD_LOGIC;
    clk_in1 : in STD_LOGIC
  );

end component clk_wiz_0;
  
  
    SIGNAL  clk_50MHz       : STD_LOGIC;
    SIGNAL  RESET           : STD_LOGIC;
    SIGNAL	iic_rtl_0_scl_i : STD_LOGIC;
    SIGNAL	iic_rtl_0_scl_o : STD_LOGIC;
    SIGNAL	iic_rtl_0_scl_t : STD_LOGIC;
    SIGNAL	iic_rtl_0_sda_i : STD_LOGIC;
    SIGNAL	iic_rtl_0_sda_o : STD_LOGIC;
    SIGNAL	iic_rtl_0_sda_t : STD_LOGIC;

    SIGNAL	reg0_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg1_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg2_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg3_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg4_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg5_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg6_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg7_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg8_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg9_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg10_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg11_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg12_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg13_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg14_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg15_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    SIGNAL	reg16_p 			:  STD_LOGIC_VECTOR(31 DOWNTO 0);
    
    
    SIGNAL	AXI_CLK_OUT      : STD_LOGIC;
    SIGNAL	AXI_RSTn         : STD_LOGIC_VECTOR ( 0 to 0 );
    SIGNAL	Data_IN          : STD_LOGIC_VECTOR ( 31 downto 0 );
    SIGNAL	Data_OUT         : STD_LOGIC_VECTOR ( 31 downto 0 );
    SIGNAL	RD_addr          : STD_LOGIC_VECTOR ( 10 downto 0 );
    SIGNAL	RD_strb          : STD_LOGIC;
    SIGNAL	WR_addr          : STD_LOGIC_VECTOR ( 10 downto 0 );
    SIGNAL	WR_strb          : STD_LOGIC;
    SIGNAL  FEMB_CONV_CLK    : STD_LOGIC;
    SIGNAL  FEMB_GTH_RST     : STD_LOGIC;
    SIGNAL  pll_locked       : STD_LOGIC;
   
    SIGNAL  userdata_rx_out		    : STD_LOGIC_VECTOR(63 downto 0); 			
    SIGNAL  rxusrclk2_OUT			: STD_LOGIC;	
    SIGNAL  userclk_rx_active_out	: STD_LOGIC; 			
    SIGNAL  rxcommadet_out			: STD_LOGIC_VECTOR(3 downto 0);		  
       
    
begin


QSFP_RST       <= '1';
RESET          <= not AXI_RSTn(0);

MGT_CLK_SEL    <= '1';


ZYNQ_TOP_wrapper_i: component ZYNQ_TOP_wrapper
     port map (
      AXI_CLK_OUT      => AXI_CLK_OUT,
      AXI_RSTn(0)      => AXI_RSTn(0),
      Data_IN          => Data_IN,
      Data_OUT         => Data_OUT,
      RD_addr          => RD_addr,
      RD_strb          => RD_strb,
      WR_addr          => WR_addr,
      WR_strb          => WR_strb,
      WIB_LED_tri_o    => WIB_LED,
      
      iic_rtl_0_scl_i => iic_rtl_0_scl_i,
      iic_rtl_0_scl_o => iic_rtl_0_scl_o,
      iic_rtl_0_scl_t => iic_rtl_0_scl_t,
      iic_rtl_0_sda_i => iic_rtl_0_sda_i,
      iic_rtl_0_sda_o => iic_rtl_0_sda_o,
      iic_rtl_0_sda_t => iic_rtl_0_sda_t
    );



io_registers_inst : entity work.IO_registers
PORT MAP(       rstn 			=> AXI_RSTn(0),
				clk 			=> AXI_CLK_OUT,
				WR 			    => WR_strb,
				WR_address 	    => WR_addr(7 downto 0),
				RD_address 	    => RD_addr(7 downto 0),
				data 			=> Data_OUT,
				data_out        => Data_IN,
				reg0_i  	    => reg0_p,
				reg1_i	        => reg1_p,		 
				reg2_i 	        => reg2_p,		 
				reg3_i 	        => reg3_p,	
				reg4_i 	        => reg4_p,
				reg5_i 	        => reg5_p,
				reg6_i 	        => reg6_p,
				reg7_i 	        => reg7_p,
				reg8_i 	        => reg8_p,
				reg9_i 	        => reg9_p,
				reg10_i 	    => reg10_p,
				reg11_i 	    => reg11_p,
				reg12_i 	    => reg12_p,
				reg13_i 	    => reg13_p,
				reg14_i 	    => pll_locked & reg14_p(30 downto 0),
				reg15_i 	    => x"DEADBEEF",
				reg16_i 	    => reg16_p(30 downto 0) & '0', --is this times 2
	
				reg0_o          => reg0_p,
				reg1_o          => reg1_p,				
				reg2_o          => reg2_p,		
				reg3_o          => reg3_p,		
				reg4_o          => open,
				reg5_o          => reg5_p,
				reg6_o          => reg6_p,
				reg7_o          => reg7_p,
				reg8_o          => reg8_p,
				reg9_o          => reg9_p,		
				reg10_o         => reg10_p,
				reg11_o         => reg11_p,
				reg12_o         => reg12_p,
				reg13_o         => reg13_p,
				reg14_o         => reg14_p,
				reg15_o         => reg15_p,
				reg16_o         => reg16_p
);


 SBND_PWM_CLK_ENCODER_INST : ENTITY WORK.SBND_PWM_CLK_ENCODER
	PORT map
	(	
			RESET			 => RESET,
			CLK_100MHz		 => AXI_CLK_OUT,
			SAMPLE_RATE		 => X"0",		
			EXT_CMD1		 => '0',	
			EXT_CMD2		 => '0',
			EXT_CMD3		 => '0',
			EXT_CMD4		 => '0',	
			SW_CMD1			 => '0',       --reg1_p(0),
			SW_CMD2			 => '0',    --reg1_p(1),
			SW_CMD3			 => '0',  --reg1_p(2),
			SW_CMD4			 => '0', --reg1_p(3),
			DIS_CMD1	     => '0',
			DIS_CMD2		 => '0',
			DIS_CMD3		 => '0',
			DIS_CMD4		 => '0',			
			SBND_SYNC_CMD	 => FEMB_CONV_CLK,
			SBND_ADC_CLK	 => open   -- 2MHZ sample clock				
	);

 FEMB_CMD_SEL             <= '0';
 FEMB_CLK_SEL             <= '1';  
       

obuf3: OBUFDS  port map (O => DUNE_CLK_FPGA_OUT_P, OB => DUNE_CLK_FPGA_OUT_N, I => AXI_CLK_OUT);
obuf4: OBUFDS  port map (O => FEMB_CMD_FPGA_OUT_P, OB => FEMB_CMD_FPGA_OUT_N, I => FEMB_CONV_CLK); 
 
  
I2C_CONTROL_inst : entity work.I2C_CONTROL
PORT MAP(
 
          I2C_SELECT               => reg1_p( 7 downto 0),
          iic_rtl_0_scl_i          => iic_rtl_0_scl_i,
          iic_rtl_0_scl_o          => iic_rtl_0_scl_o,
          iic_rtl_0_scl_t          => iic_rtl_0_scl_t, 
          
          iic_rtl_0_sda_i          => iic_rtl_0_sda_i,
          iic_rtl_0_sda_o          => iic_rtl_0_sda_o,
          iic_rtl_0_sda_t          => iic_rtl_0_sda_t,
  
          SI5344_SCL               => SI5344_SCL,
          SI5344_SDA               => SI5344_SDA, 
    
          SI5342_SCL               => SI5342_SCL,
          SI5342_SDA               => SI5342_SDA, 
    
          QSFP_SCL                 => QSFP_SCL, 
          QSFP_SDA                 => QSFP_SDA,
    
          PL_FEMB_PWR_SCL          => PL_FEMB_PWR_SCL, 
          PL_FEMB_PWR_SDA          => PL_FEMB_PWR_SDA,
     
          PL_FEMB_EN_SCL           => PL_FEMB_EN_SCL,
          PL_FEMB_EN_SDA           => PL_FEMB_EN_SDA, 
    
          SENSOR_I2C_SCL           => SENSOR_I2C_SCL,
          SENSOR_I2C_SDA           => SENSOR_I2C_SDA,
    
          PL_FEMB_PWR2_SCL         => PL_FEMB_PWR2_SCL,
          PL_FEMB_PWR2_SDA         => PL_FEMB_PWR2_SDA, 
    
          LTC2977_SDA              => LTC2977_SDA,
          LTC2977_SCL              => LTC2977_SCL,
    
          PL_FEMB_PWR3_SCL         => PL_FEMB_PWR3_SCL,
          PL_FEMB_PWR3_SDA         => PL_FEMB_PWR3_SDA,
          
          FLASH_SCL                => FLASH_SCL, 
          FLASH_SDA                => FLASH_SDA ,
          
          ADN2814_SCL              => ADN2814_SCK,               
          ADN2814_SDA              => ADN2814_SDA
          
 );
  
  

 	WIB_FEMB_COMM_TOP_INST : WIB_FEMB_COMM_TOP
	PORT MAP
	(
		RESET   	   			=> RESET,
		SYS_CLK	   			    => AXI_CLK_OUT,
						
		FEMB_wr_strb 			=> reg3_p(31),
		FEMB_rd_strb 			=> reg3_p(30),
		FEMB_address 			=> reg3_p(15 downto 0),
		FEMB_BRD				=> reg3_p(19 downto 16),
		FEMB_DATA_TO_FEMB		=> reg2_p,
		FEMB_DATA_RDY			=> open,
		FEMB_DATA_FRM_FEMB	    => reg4_p,
		
		
		FEMB_SCL_BRD0_P		=> FEMB_BRD0_IO_P(0),
		FEMB_SCL_BRD0_N		=> FEMB_BRD0_IO_N(0),
		FEMB_SDA_BRD0_P		=> FEMB_BRD0_IO_P(1),
		FEMB_SDA_BRD0_N		=> FEMB_BRD0_IO_N(1),

		FEMB_SCL_BRD1_P		=> FEMB_BRD1_IO_P(0),
		FEMB_SCL_BRD1_N		=> FEMB_BRD1_IO_N(0),
		FEMB_SDA_BRD1_P		=> FEMB_BRD1_IO_P(1),
		FEMB_SDA_BRD1_N		=> FEMB_BRD1_IO_N(1),

		FEMB_SCL_BRD2_P		=> FEMB_BRD2_IO_P(0),
		FEMB_SCL_BRD2_N		=> FEMB_BRD2_IO_N(0),
		FEMB_SDA_BRD2_P		=> FEMB_BRD2_IO_P(1),
		FEMB_SDA_BRD2_N		=> FEMB_BRD2_IO_N(1),
		
		FEMB_SCL_BRD3_P		=> FEMB_BRD3_IO_P(0),
		FEMB_SCL_BRD3_N		=> FEMB_BRD3_IO_N(0),
		FEMB_SDA_BRD3_P		=> FEMB_BRD3_IO_P(1),
		FEMB_SDA_BRD3_N		=> FEMB_BRD3_IO_N(1),
		FEMB_BUSY			=> open

	);	
				 

   
ibuf1: IBUFDS  port map (O => open, I => ADN2814_DATA_P, IB => ADN2814_DATA_N); 
ibuf2: IBUFDS  port map (O => open, I => DUNE_CLK_FPGA_IN_P, IB => DUNE_CLK_FPGA_IN_N);

ibuf3: IBUFDS  port map (O => open, I => SI5344_OUT0_P, IB => SI5344_OUT0_N);
ibuf4: IBUFDS  port map (O => open, I => SI5344_OUT1_P, IB => SI5344_OUT1_N);
 


obuf1: OBUFDS  port map (O => TX_TIMING_P, OB => TX_TIMING_N, I => '0');
obuf2: OBUFDS  port map (O => RECOV_CLK_P, OB => RECOV_CLK_N, I => '0');



  GEN_buf: 
   for I in 2 to 5 generate
        bufx0: IOBUFDS  port map (IO => FEMB_BRD0_IO_P(I), IOB => FEMB_BRD0_IO_N(I), I => '0',O => open,T =>'1');
        bufx1: IOBUFDS  port map (IO => FEMB_BRD1_IO_P(I), IOB => FEMB_BRD1_IO_N(I), I => '0',O => open,T =>'1');
        bufx2: IOBUFDS  port map (IO => FEMB_BRD2_IO_P(I), IOB => FEMB_BRD2_IO_N(I), I => '0',O => open,T =>'1');
        bufx3: IOBUFDS  port map (IO => FEMB_BRD3_IO_P(I), IOB => FEMB_BRD3_IO_N(I), I => '0',O => open,T =>'1');
   
   end generate GEN_buf;
 
 FEMB_GTH_RST   <= RESET or reg5_p(0) or  (not pll_locked);
 
 
 
clk_wiz_0_inst : clk_wiz_0  
  Port map( 
        clk_out1     => clk_50MHz,
        locked       => pll_locked,
        clk_in1      => AXI_CLK_OUT
  );

MISC_IO(0) <= clk_50MHz;
MISC_IO(1) <= AXI_CLK_OUT;
MISC_IO(2) <= pll_locked;
MISC_IO(3) <= CLK_IN_50MHz;
MISC_IO(4) <= rxusrclk2_OUT;

FEMB_GTH_inst : FEMB_GTH_top
port map (
	gthtxn_out								=> FEMB0_TXn,  --: OUT STD_LOGIC_VECTOR(3 downto 0); 
	gthtxp_out								=> FEMB0_TXp,  --: OUT STD_LOGIC_VECTOR(3 downto 0); 		
	gthrxn_in								=> FEMB0_RXn, --: IN STD_LOGIC_VECTOR(3 downto 0); 
	gthrxp_in								=> FEMB0_RXp, --: IN STD_LOGIC_VECTOR(3 downto 0); 
	
    gtrefclk0_p                             => FEMB0_refclk0_p,  --: IN STD_LOGIC; 
    gtrefclk0_n                             => FEMB0_refclk0_n,  --: IN STD_LOGIC; 	
		
    reset                                   => FEMB_GTH_RST, --: IN STD_LOGIC;   -- reset core
    clk_50MHz                               => CLK_IN_50MHz,  --: IN STD_LOGIC;   -- freerun clock
      		
    userdata_rx_out					        => userdata_rx_out,  --: OUT STD_LOGIC_VECTOR(63 downto 0); 			
	rxusrclk2_OUT							=> rxusrclk2_OUT,  --: OUT STD_LOGIC;	
	rxctrl0_out								=> open,  --: OUT STD_LOGIC_VECTOR(63 downto 0); 
	rxctrl1_out								=> open,  --: OUT STD_LOGIC_VECTOR(63 downto 0); 
	rxctrl2_out								=> open,  --: OUT STD_LOGIC_VECTOR(31 downto 0); 
	rxctrl3_out								=> open,  --: OUT STD_LOGIC_VECTOR(31 downto 0); 
	gtwiz_reset_rx_cdr_stable_out			=> open,  --: OUT STD_LOGIC; 
	reset_rx_done_out					    => open,  --: OUT STD_LOGIC; 	
    userclk_rx_active_out				    => userclk_rx_active_out,  --: OUT STD_LOGIC; 			
		
	gtpowergood_out							=> open,  --: OUT STD_LOGIC_VECTOR(3 downto 0); 
	rxbyteisaligned_out						=> open,  --: OUT STD_LOGIC_VECTOR(3 downto 0); 
	rxbyterealign_out						=> open,  --: OUT STD_LOGIC_VECTOR(3 downto 0); 
	rxcommadet_out							=> rxcommadet_out  --: OUT STD_LOGIC_VECTOR(3 downto 0)		

  );


end Behavioral;