library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity eventSim is  generic(
    DATA_WIDTH : integer := 16
  );
  port(
    clk         : in std_logic;
    enable      : in std_logic;
    max_value   : in std_logic_vector(DATA_WIDTH-1 downto 0);
    eventOut    : out std_logic
  );
end eventSim;

architecture behavior of eventSim is
    signal count : unsigned(DATA_WIDTH-1 downto 0);
    signal current_max_count : unsigned(DATA_WIDTH-1 downto 0);
    signal ready_flag : std_logic;
begin    

    prCount : process (clk,enable)
    begin
      if rising_edge(clk) then
        if (enable = '1') then
          count <= to_unsigned(0,DATA_WIDTH);
          current_max_count <= to_unsigned(0,DATA_WIDTH);
          eventOut <= '0';
        else
          count <= count + 1;                       -- increment count
          if (count>=current_max_count) then        -- produce output 
            eventOut <= '1';
            count <= to_unsigned(0,DATA_WIDTH);
            if (current_max_count<unsigned(max_value)-1) then  -- increment current_max_count
              current_max_count <= current_max_count + 1;
            else
              current_max_count <= to_unsigned(1,DATA_WIDTH);
            end if;
          else
            eventOut <= '0';
          end if;
        end if;
      end if;
    end process;
    
end behavior;