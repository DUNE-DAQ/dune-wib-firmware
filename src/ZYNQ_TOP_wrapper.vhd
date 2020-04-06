--Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
----------------------------------------------------------------------------------
--Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
--Date        : Tue Mar 24 18:33:34 2020
--Host        : Jack-I7-PC running 64-bit major release  (build 9200)
--Command     : generate_target ZYNQ_TOP_wrapper.bd
--Design      : ZYNQ_TOP_wrapper
--Purpose     : IP block netlist
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity ZYNQ_TOP_wrapper is
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
end ZYNQ_TOP_wrapper;

architecture STRUCTURE of ZYNQ_TOP_wrapper is
  component ZYNQ_TOP is
  port (
    Data_OUT : out STD_LOGIC_VECTOR ( 31 downto 0 );
    Data_IN : in STD_LOGIC_VECTOR ( 31 downto 0 );
    WR_strb : out STD_LOGIC;
    WR_addr : out STD_LOGIC_VECTOR ( 10 downto 0 );
    RD_strb : out STD_LOGIC;
    RD_addr : out STD_LOGIC_VECTOR ( 10 downto 0 );
    AXI_CLK_OUT : out STD_LOGIC;
    AXI_RSTn : out STD_LOGIC_VECTOR ( 0 to 0 );
    iic_rtl_0_scl_i : in STD_LOGIC;
    iic_rtl_0_scl_o : out STD_LOGIC;
    iic_rtl_0_scl_t : out STD_LOGIC;
    iic_rtl_0_sda_i : in STD_LOGIC;
    iic_rtl_0_sda_o : out STD_LOGIC;
    iic_rtl_0_sda_t : out STD_LOGIC;
    WIB_LED_tri_o : out STD_LOGIC_VECTOR ( 7 downto 0 )
  );
  end component ZYNQ_TOP;


begin
ZYNQ_TOP_i: component ZYNQ_TOP
     port map (
      AXI_CLK_OUT => AXI_CLK_OUT,
      AXI_RSTn(0) => AXI_RSTn(0),
      Data_IN(31 downto 0) => Data_IN(31 downto 0),
      Data_OUT(31 downto 0) => Data_OUT(31 downto 0),
      RD_addr(10 downto 0) => RD_addr(10 downto 0),
      RD_strb => RD_strb,
      WIB_LED_tri_o(7 downto 0) => WIB_LED_tri_o(7 downto 0),
      WR_addr(10 downto 0) => WR_addr(10 downto 0),
      WR_strb => WR_strb,
      iic_rtl_0_scl_i => iic_rtl_0_scl_i,
      iic_rtl_0_scl_o => iic_rtl_0_scl_o,
      iic_rtl_0_scl_t => iic_rtl_0_scl_t,
      iic_rtl_0_sda_i => iic_rtl_0_sda_i,
      iic_rtl_0_sda_o => iic_rtl_0_sda_o,
      iic_rtl_0_sda_t => iic_rtl_0_sda_t
    );
end STRUCTURE;
