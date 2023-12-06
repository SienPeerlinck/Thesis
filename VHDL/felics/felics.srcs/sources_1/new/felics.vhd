----------------------------------------------------------------------------------
-- Company: 
-- Engineer: Sien Peerlinck
-- 
-- Create Date: 11/26/2023 05:44:13 PM
-- Design Name: 
-- Module Name: felics - Behavioral
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
-- use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity FELICS is
    generic(
        -- Alle constanten (felics header?) hierin vastleggen      
        -- Header Structure --> Header gerbuiken?
        FELICS_VERSION_BITS : natural := 8;
        DELTA_MIN_BITS : natural := 24;
        IMG_WIDTH_BITS : natural := 12;
        IMG_HEIGTH_BITS : natural := 12;
        BITS_PER_PIXEL_BITS : natural := 5;
        
        -- Header info 
        
        IMG_WIDTH : natural := 384;
        IMG_HEIGTH : natural := 288;
        BITS_PER_PIXEL : natural := 20;
        DEFAULT_DELTA_MIN :  std_logic_vector(20-1 downto 0) := std_logic_vector(to_unsigned(255, 20))
    );
    port(
        I : in std_logic_vector(BITS_PER_PIXEL-1 downto 0);
        O : out std_logic_vector(BITS_PER_PIXEL-1 downto 0);
        clk : in std_logic
    );
end felics;

architecture Behavioral of FELICS is
    -- eerst signals 
    signal p : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0'); -- Pixel waar we mee werken
    signal b1 : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0'); 
    signal b2 : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    signal l : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0'); -- Kleinste buur 
    signal h : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0'); -- Grootste buur
    signal delta :  std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    type im is array (IMG_WIDTH-1 downto 0) of std_logic_vector(BITS_PER_PIXEL-1 downto 0);
    signal zero_vector : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    signal buffer_pixels : im := (others=>zero_vector);
    signal buffer_zeros : im := (others => zero_vector);
    
    signal ABC_in : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    signal ABC_delta : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    signal ABC_out : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    signal SEC_in : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    signal SEC_out : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    
    signal felics_in : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    signal felics_out : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    signal e : std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
    
    
    -- dan componenten (ab_code, se_code) --> ook nog instantiëren in begin zelf (port mapping)
    component ABC is
    generic(
        BITS_PER_PIXEL : natural := 20
        );
    port ( 
        ABC_in : in std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
        ABC_delta : in std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
        clk : in std_logic;
        ABC_out : out std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0')
    );
    end component;
    
    component SEC is 
    generic(
        BITS_PER_PIXEL : natural := 20
    );
    port ( 
        SEC_in : in std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0');
        clk : in std_logic;
        SEC_out : out std_logic_vector(BITS_PER_PIXEL-1 downto 0) := (others=>'0')
    );
    end component;
    
    -- dan effectieve implementatie 
    begin
        felics_in <= I;
        
        ab_code: ABC 
        generic map(
            BITS_PER_PIXEL => BITS_PER_PIXEL
        )
        port map(
            ABC_in => ABC_in,
            ABC_delta => ABC_delta,
            clk => clk,
            ABC_out => ABC_out
        );
        
        se_code: SEC
        generic map(
            BITS_PER_PIXEL => BITS_PER_PIXEL
        )
        port map(
            SEC_in => SEC_in,
            clk => clk,
            SEC_out => SEC_out
        );
        
               
        process (clk) is
        begin
            if rising_edge(clk) then
            -- Beginnen met selectie van de buren, dit gaat ervan uit dat geen enkel pixel als waarde allemaal nullen heeft
                p <= felics_in;
                if buffer_pixels = buffer_zeros then  -- eerste pixel
                    felics_out <= p;
                    buffer_pixels(0) <= p;
                elsif buffer_pixels(IMG_WIDTH-1) = zero_vector then -- Pixel in eerste rij (im[0,x])
                    if buffer_pixels(1) = zero_vector then -- im[0,1] 
                        felics_out <= p;
                        buffer_pixels <= buffer_pixels(IMG_WIDTH-2 downto 0) & p; -- huidige pixel wordt achteraan toegevoegd aan buffer 
                    else 
                        b1 <= buffer_pixels(1);
                        b2 <= buffer_pixels(0);
                    end if;
                else
                    b1 <= buffer_pixels(IMG_WIDTH-1);
                    b2 <= buffer_pixels(0);
                end if;
                
            -- Buren vergelijken
                if b1 < b2 then
                    l <= b1;
                    h <= b2;
                else
                    h <= b1;
                    l <= b2;
                end if;
                
            -- Delta berekenen
                delta <= std_logic_vector(unsigned(h) - unsigned(l)); 
                if delta < DEFAULT_DELTA_MIN then
                    if l < '0'&DEFAULT_DELTA_MIN(BITS_PER_PIXEL-1 downto 1) then  -- DEFAULT_DELTA_MIN/2 (right shift)
                        l <= zero_vector;
                    else
                         l <= std_logic_vector(unsigned(l)-unsigned('0'&DEFAULT_DELTA_MIN(BITS_PER_PIXEL-1 downto 1)));
                    end if;
                    h <= std_logic_vector(unsigned(l)  + unsigned(DEFAULT_DELTA_MIN));
                    delta <= DEFAULT_DELTA_MIN;
                end if;
                         
            -- Kijken of p tussen l en h zit, prefix erbij en juist laten coderen
                if l <= p and p <= h then  -- in range
                    ABC_in <= std_logic_vector(unsigned(p) - unsigned(l)); 
                    ABC_delta <= delta;
                    felics_out(BITS_PER_PIXEL-1) <= '0'; -- prefix 0 hier ook al direct ABC_out aan toevoegen?
                    felics_out(BITS_PER_PIXEL-2 downto 0) <= ABC_out;
                    buffer_pixels <= buffer_pixels(IMG_WIDTH-2 downto 0) & p; -- huidige pixel wordt achteraan toegevoegd aan buffer
                else  -- out of range
                    felics_out(BITS_PER_PIXEL-1) <= '1'; -- prefix 1
                    if p < l then -- below range 
                        felics_out(BITS_PER_PIXEL-2) <= '0'; -- prefix 0
                        e <= std_logic_vector(unsigned(l) - unsigned(p)-1); 
                    elsif p > h then -- above range 
                        felics_out(BITS_PER_PIXEL-2) <= '1'; -- prefix 1 
                        e <= std_logic_vector(unsigned(p) - unsigned(h)-1);
                    end if;
                    SEC_in <= e;
                    felics_out(BITS_PER_PIXEL-3 downto 0) <= SEC_out;   
                    buffer_pixels <= buffer_pixels(IMG_WIDTH-2 downto 0) & p; -- huidige pixel wordt achteraan toegevoegd aan buffer
                end if;
            -- nog ergens felics_out <= ABC_out en felics_out <= SEC_out, maar hoe zit dat met timing?        
            end if;
        end process;
    
    
end Behavioral;


