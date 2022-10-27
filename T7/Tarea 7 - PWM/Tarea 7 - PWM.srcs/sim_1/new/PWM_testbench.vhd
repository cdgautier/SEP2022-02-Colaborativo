-- Testbench created online at:
--   https://www.doulos.com/knowhow/perl/vhdl-testbench-creation-using-perl/
-- Copyright Doulos Ltd

library IEEE;
use IEEE.Std_logic_1164.all;
use IEEE.Numeric_Std.all;

entity PWM_tb is
end;

architecture bench of PWM_tb is

  component PWM(pwm_bits - 1)(pwm_bits - 1)
      Generic (pwm_bits: INTEGER;
               clk_cnt_len: POSITIVE := 1);
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
  uut: PWM generic map ( pwm_bits    => 8,
                         clk_cnt_len =>  1)
              port map ( clk         => clk,
                         rst         => rst,
                         duty_cycle  => duty_cycle,
                         pwm_out     => pwm_out );

  stimulus: process
  begin
  
    -- Put initialisation code here
    wait for 100 ms;


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
  