----------------------------------------------------------------------------------
-- SEP - Tarea 06 
--
-- Camilo Gutiérrez 
-- Marco Ramírez
--
-- Project Name: project_semaforo - Behavioral
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity project_semaforo is                                       -- se definen puertos de entrada y salida
    Port ( clk : in STD_LOGIC;
           btn_value : in STD_LOGIC;
           led0 : out STD_LOGIC;
           led1 : out STD_LOGIC;
           led2 : out STD_LOGIC;
           led3 : out STD_LOGIC);
end project_semaforo;

architecture Behavioral of project_semaforo is                   -- se definen las señales a utilizar
    signal tmr_count: std_logic_vector(26 downto 0);             
    signal max_counter: std_logic_vector(4 downto 0) := "01111"; -- su valor inicial es 15 (16 seg). Reinicia el cliclo del semaforo
    signal btn_press: std_logic;                                 -- vale 1 si el boton fue presionado y almacena este valor
    signal led_counter: std_logic_vector(4 downto 0) := "00000"; -- iterador del ciclo del semaforo
    signal led_data:  std_logic_vector(3 downto 0) := "0010";    -- lo que muestran los leds
    
begin
    Semaforo: process(clk, btn_value)
    begin
        if rising_edge(clk) then
            if tmr_count < "111011100110101100101000000" then    -- contador hasta 125_000_000 
                tmr_count <= tmr_count + 1; 
                if btn_value = '1' then                          -- recibe el input del boton en cada ciclo 
                    btn_press <= '1';                            -- btn_press guarda un 1 si el boton fue presionado
                end if;   
            else                                                 -- si alcanza la cuenta (1 seg)
                tmr_count <= (others => '0');                    -- resetea tmr_count
                
                if led_counter < "00101" then                                   -- si led_counter es menor a 5
                    led_data <= "0010";                                         -- luz verde
                elsif (led_counter >= "00101" AND led_counter < "00111") then   -- si 5 <= led_counter < 7
                    led_data <= "0100";                                         -- luz amarilla
                elsif led_counter >= "00111" then                               -- si led_counter >= 7
                    led_data <= "1001";                                         -- luz roja y peaton
                end if;
                
                led0 <= led_data(0);                    -- se asignan los leds segun el vector led_data
                led1 <= led_data(1);
                led2 <= led_data(2);
                led3 <= led_data(3);
                
                led_counter <= led_counter + 1;         -- se actualiza led_counter
                
                if led_counter = max_counter then       -- si led_counter alcanza el valor maximo
                    led_counter <= (others => '0');     -- resetea led_counter
                    if btn_press = '1' then             -- si el boton fue presionado durante el ciclo
                        max_counter <= "10001";         -- max_counter aumenta de 15(1111) a 17(10001). hint: para un efecto visual mas claro setear en 20 (10100)
                        btn_press <= '0';               -- se resetea btn_press
                    else
                        max_counter <= "01111";         -- en cualquier otro caso, max_counter vale 15
                    end if;
                end if;
            end if;
        end if;
    end process Semaforo;
end Behavioral;