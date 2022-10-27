----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 10/26/2022 10:22:49 PM
-- Design Name: 
-- Module Name: PWM_tb - Behavioral
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
use IEEE.Std_logic_1164.all;
use IEEE.Numeric_Std.all;

entity PWM_tb is
end;

architecture bench of PWM_tb is

  component PWM
      Generic (pwm_bits: INTEGER;
               clk_cnt_len: INTEGER := 1);
      Port ( clk : in STD_LOGIC;
             rst : in STD_LOGIC;
             duty_cycle : in UNSIGNED((pwm_bits - 1) DOWNTO 0);
             pwm_out : out STD_LOGIC);
  end component;

  signal clk: STD_LOGIC;
  signal rst: STD_LOGIC;
  signal duty_cycle: UNSIGNED((pwm_bits - 1) DOWNTO 0);
  signal pwm_out: STD_LOGIC;

  constant clock_period: time := 10 ns;
  signal stop_the_clock: boolean;

begin

  -- Insert values for generic parameters !!
  uut: PWM generic map ( pwm_bits    => ,
                         clk_cnt_len =>  )
              port map ( clk         => clk,
                         rst         => rst,
                         duty_cycle  => duty_cycle,
                         pwm_out     => pwm_out );

  stimulus: process
  begin
  
    -- Put initialisation code here


    -- Put test bench stimulus code here

    stop_the_clock <= true;
    wait;
  end process;

  clocking: process
  begin
    while not stop_the_clock loop
      clk <= '0', '1' after clock_period / 2;
      wait for clock_period;
    end loop;
    wait;
  end process;

end;

end Behavioral;
