----------------------------------------------------------------------------------
-- Company: 
-- Engineer: Sien Peerlinck
-- 
-- Create Date: 11/28/2023 04:57:40 PM
-- Design Name: 
-- Module Name: SEC - Behavioral
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

entity SEC is
    generic(
        BITS_PER_PIXEL : natural := 20
    );
  port ( 
    SEC_in : in std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    clk : in std_logic;
    SEC_out : out std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0')
  );
end SEC;

architecture Behavioral of SEC is
    -- eerst signals 
    signal k : natural := 12;
    signal b : natural := BITS_PER_PIXEL-1;
    signal u : natural := 0;

begin
    process (clk) is
    begin
        if rising_edge(clk) then 
            if b < k then
                b <= k;
                u <= 0;
            else
                u <= b - k + 1;
            end if;
            
            SEC_out(BITS_PER_PIXEL-1 downto BITS_PER_PIXEL-u) <= (others => '1');
            SEC_out(BITS_PER_PIXEL-u-1 downto 0) <= '0' & SEC_in(b-1 downto 0);  
        
        end if;
    end process;


end Behavioral;
