----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 10/25/2022 05:52:47 PM
-- Design Name: 
-- Module Name: PWM - Behavioral
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

-- clock debe ser de 20 MHz
-- 3 entradas y 1 salida
-- pwm_bits determina el valor de frecuencia de salida
-- 

entity PWM is
    Generic (pwm_bits: INTEGER;
             clk_cnt_len: POSITIVE := 1);
    Port ( clk : in STD_LOGIC;
           rst : in STD_LOGIC;
           duty_cycle : in UNSIGNED((pwm_bits - 1) DOWNTO 0);
           pwm_out : out STD_LOGIC);
end PWM;

architecture Behavioral of PWM is

signal pwm_cnt : UNSIGNED(pwm_bits - 1 DOWNTO 0);
signal clk_cnt : INTEGER range 0 to clk_cnt_len - 1;

begin

    clk_rst: PROCESS(clk, rst)
    BEGIN
      if rising_edge(clk) then
        if rst = '1' then
          clk_cnt <= 0;
           
        else
          if clk_cnt < clk_cnt_len - 1 then
            clk_cnt <= clk_cnt + 1;
          else
            clk_cnt <= 0;
          end if;
           
        end if;
      end if;
    END PROCESS;
    
    out_logic: PROCESS(clk)
    BEGIN
      if rising_edge(clk) then
        if rst = '1' then
          pwm_cnt <= (others => '0');
          pwm_out <= '0';
     
        else
          if clk_cnt_len = 1 or clk_cnt = 0 then
     
            pwm_cnt <= pwm_cnt + 1;
            pwm_out <= '0';
     
            if pwm_cnt = unsigned(to_signed(-2, pwm_cnt'length)) then
              pwm_cnt <= (0 => '0', others => '0');
            end if;
     
            if pwm_cnt < duty_cycle then
              pwm_out <= '1';
            end if;
     
          end if;
        end if;
      end if;
    END PROCESS;

end Behavioral;
