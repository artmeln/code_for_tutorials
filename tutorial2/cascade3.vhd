library ieee;
use ieee.std_logic_1164.all;
 
entity cascade3 is
  port(
    clk             : in std_logic;
    input_casc      : in std_logic;
    output_casc     : out std_logic
  );
end cascade3;

architecture behavior of cascade3 is
    signal output_csc1 : std_logic;
    signal output_csc2 : std_logic;
    signal output_csc3 : std_logic;
    
begin    

    prCascade1 : process(clk)
    begin 
      if (rising_edge(clk)) then
        output_csc1 <= input_casc;
      end if;
    end process;

    prCascade2 : process(clk)
    begin 
      if (rising_edge(clk)) then
        output_csc2 <= output_csc1;
      end if;
    end process;

    prCascade3 : process(clk)
    begin 
      if (rising_edge(clk)) then
        output_csc3 <= output_csc2;
      end if;
    end process;

    output_casc <= output_csc3;
        
end behavior;