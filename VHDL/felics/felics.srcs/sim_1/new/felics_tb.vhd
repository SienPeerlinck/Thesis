----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 12/07/2023 08:36:06 PM
-- Design Name: 
-- Module Name: felics_tb - Behavioral
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

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity felics_tb is
end felics_tb;

architecture Behavioral of felics_tb is
-- component declaration 

    component FELICS is
    generic(
        IMG_WIDTH : natural := 384;
        IMG_HEIGTH : natural := 288;
        BITS_PER_PIXEL : natural := 20;
        DEFAULT_DELTA_MIN :  std_logic_vector(20-1 downto 0) := std_logic_vector(to_unsigned(255, 20))
    
        );
    port ( 
        FELICS_in : in std_logic_vector(BITS_PER_PIXEL-1 downto 0);
        FELICS_out : out std_logic_vector(BITS_PER_PIXEL-1 downto 0);
        clk : in std_logic
    );
    end component;
    
    -- constants
    constant clk_period : time := 20 ns;
    constant IMG_WIDTH : natural := 3;
    constant IMG_HEIGTH : natural := 2;
    constant BITS_PER_PIXEL : natural := 20;
    constant DEFAULT_DELTA_MIN :  std_logic_vector(20-1 downto 0) := std_logic_vector(to_unsigned(255, 20));

-- signal declaration 
    signal FELICS_in_tb : std_logic_vector(BITS_PER_PIXEL-1 downto 0);
    signal FELICS_out_tb : std_logic_vector(BITS_PER_PIXEL-1 downto 0);
    signal clk_tb : std_logic;
    
    constant p1 : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := "11011101110111010001";
    constant p2 : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := "00010011101010010000";
    constant p3 : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := "11111111100101001000";
    constant p4 : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := "00001001111111011101";
    constant p5 : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := "00000000011000100000";
    constant p6 : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := "10010011011010000101";
    
    type im is array ((IMG_WIDTH*IMG_HEIGTH)-1 downto 0) of std_logic_vector(BITS_PER_PIXEL-1 downto 0);
    constant data_in : im := (p1, p2, p3, p4, p5, p6);

begin
-- DUT instantiation 
    DUT1: FELICS 
    port map(
        FELICS_in => FELICS_in_tb,
        FELICS_out => FELICS_out_tb,
        clk => clk_tb
    );

-- stimulus generation
    CLK_PROC: process
    begin
        clk_tb <= '0';
        wait for clk_period/2;
        clk_tb <= '1';
        wait for clk_period/2;
    end process CLK_PROC;


end Behavioral;
