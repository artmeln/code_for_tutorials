library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_eventSim is
end tb_eventSim;

architecture behavior of tb_eventSim is

-- component declaration for the unit under test (uut)
component eventSim is
  generic(
    DATA_WIDTH : integer
  );
  port(
    clk         : in std_logic;
    enable      : in std_logic;
    max_value   : in std_logic_vector(DATA_WIDTH-1 downto 0);
    eventOut    : out std_logic
  );
end component;

-- define the period of clock here.
-- It's recommended to use CAPITAL letters to define constants.
constant DATA_WIDTH_TB : integer := 8;
constant MAX_VALUE_TB  : integer := 6; 
constant CLK_PERIOD_TB : time := 10 ns;

--declare inputs and initialize them to zero.
signal clk              : std_logic := '0';
signal enable           : std_logic := '1';
signal max_value        : std_logic_vector(DATA_WIDTH_TB - 1 downto 0) := std_logic_vector(to_unsigned(MAX_VALUE_TB,DATA_WIDTH_TB));

--declare outputs
signal eventOut         : std_logic;

begin

    -- instantiate the unit under test (uut)
   uut : eventSim generic map (DATA_WIDTH_TB) port map (
            clk => clk,
            enable => enable,
            max_value => max_value,
            eventOut => eventOut
        ); 
        
   -- Clock process definitions
   Clk_process :process
   begin
        clk <= '0';
        wait for CLK_PERIOD_TB/2;
        clk <= '1';
        wait for CLK_PERIOD_TB/2;
   end process;
    
   -- Stimulus process, Apply inputs here.
  stim_proc: process
   begin        
        wait for CLK_PERIOD_TB*5;
        
        enable <='0';                  --enable the simulator

        -- event generation upon reaching max count
        wait for CLK_PERIOD_TB * MAX_VALUE_TB * 3;
                
        wait;

  end process;

end;