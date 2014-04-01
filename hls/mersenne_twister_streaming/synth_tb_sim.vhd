----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 08/24/2013 02:02:25 PM
-- Design Name: 
-- Module Name: synth_tb_sim - Behavioral
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

entity synth_tb_sim is
    --Port ( );
end synth_tb_sim;

architecture Behavioral of synth_tb_sim is

component synth_tb
    port ( aclk : in STD_LOGIC;
           rstn : in STD_LOGIC;
           start : in STD_LOGIC;
           done : out STD_LOGIC;
           ready : out STD_LOGIC;
           failed : out STD_LOGIC);
end component;
           
signal clk : STD_LOGIC;
signal resetn : STD_LOGIC;
signal start : STD_LOGIC;
signal done : STD_LOGIC;
signal ready : STD_LOGIC;
signal failed : STD_LOGIC;

constant clk_period : time := 10 ns;

begin

  tb: component synth_tb
    port map (
        aclk => clk,
        rstn => resetn,
        start => start,
        done => done,
        ready => ready,
        failed => failed);

  clk_process: process
  begin
    clk <= '0';
    wait for clk_period/2;  --for 0.5 ns signal is '0'.
    clk <= '1';
    wait for clk_period/2;  --for next 0.5 ns signal is '1'.
  end process;
  
  reset_process: process
  begin
    wait until rising_edge(clk);
    resetn <= '0';
    start <= '0';
    wait for 100 * clk_period;
    resetn <= '1';
    wait for 100 * clk_period;
    start <= '1';
    
    wait until done = '1';
    wait for 100 * clk_period;
    if (failed = '1') then
        report "finished failure." severity FAILURE;
    else
        report "finished passed." severity FAILURE;
    end if;
    wait;
  end process;
  
end Behavioral;
