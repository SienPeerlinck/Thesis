----------------------------------------------------------------------------------
-- Company: 
-- Engineer: Sien Peerlinck
-- 
-- Create Date: 11/28/2023 04:57:40 PM
-- Design Name: 
-- Module Name: ABC - Behavioral
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
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity ABC is
    generic(
        BITS_PER_PIXEL : natural := 20
    );
  port ( 
    ABC_in : in std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    ABC_delta : in std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    clk : in std_logic;
    ABC_out : out std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0')
  );

end ABC;

architecture Behavioral of ABC is
    -- eerst signals 
     signal n : natural := BITS_PER_PIXEL;
     signal a : std_logic_vector(BITS_PER_PIXEL-2 downto 0) :=  (others=>'0');
     signal b : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
     signal half_b : std_logic_vector(BITS_PER_PIXEL-2 downto 0) := (others=>'0');     
     signal help : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
     
    
begin
    process (clk) is

    begin
        if rising_edge(clk) then 
            a <= std_logic_vector(unsigned('1' & a(a'right-1 downto 0)) - unsigned(ABC_delta) - 1);
            b <= std_logic_vector(unsigned('1' & b(b'right-1 downto 0)) - unsigned(a(n-2 downto 0) & '0'));
            
            half_b <= b(BITS_PER_PIXEL-1 downto 1);
            
            
            if ABC_in < half_b then
                help <= std_logic_vector(unsigned(a) + unsigned(ABC_in));
                ABC_out <= help(BITS_PER_PIXEL-2 downto 0) & '0';
            else
                help <= std_logic_vector(unsigned(ABC_in) - unsigned(half_b));
                if help < a then
                    ABC_out <= help;
                else
                    ABC_out <= (help(BITS_PER_PIXEL-2 downto 0) & '0') or (help'left downto help'right+1 => '0')& '1';
                end if;
            end if;
        end if;
    end process;  

end Behavioral;