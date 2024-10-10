library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_iatcollector2ch_axis is
end tb_iatcollector2ch_axis;

architecture behavior of tb_iatcollector2ch_axis is

component iatcollector2ch_axis is
    generic(
      C_M_AXIS_TDATA_WIDTH : integer
    );
	port (
        enable_cnt : in std_logic;
        channel1   : in std_logic;
        channel2   : in std_logic;
        max_count  : in std_logic_vector(C_M_AXIS_TDATA_WIDTH - 1 downto 0);
        overflow   : out std_logic;
		m_axis_aclk	: in std_logic;
		m_axis_aresetn	: in std_logic;
		m_axis_tvalid	: out std_logic;
		m_axis_tdata	: out std_logic_vector(C_M_AXIS_TDATA_WIDTH-1 downto 0);
		m_axis_tready	: in std_logic
	);
end component;

constant C_M_AXIS_TDATA_WIDTH_TB : integer := 8;
constant MAX_COUNT_TB  : integer := 5; 
constant CLK_PERIOD : time := 10 ns;
constant SIGNAL_DURATION : time := 3*CLK_PERIOD;
constant ENABLE_DELAY : time := 2 ns;

--declare inputs and initialize
signal m_axis_aclk      : std_logic := '0';
signal enable_cnt       : std_logic := '1';
signal channel1         : std_logic := '0';
signal channel2         : std_logic := '0';
signal max_count        : std_logic_vector(C_M_AXIS_TDATA_WIDTH_TB - 1 downto 0) := std_logic_vector(to_unsigned(MAX_COUNT_TB,C_M_AXIS_TDATA_WIDTH_TB));
signal m_axis_aresetn	: std_logic := '1';
signal m_axis_tready	: std_logic := '1';

--declare outputs
signal overflow         : std_logic;
signal m_axis_tvalid	: std_logic;
signal m_axis_tdata	    : std_logic_vector(C_M_AXIS_TDATA_WIDTH_TB-1 downto 0);

begin

    -- instantiate the unit under test (uut)
   uut : iatcollector2ch_axis generic map (C_M_AXIS_TDATA_WIDTH_TB) port map (
            enable_cnt => enable_cnt,
            channel1 => channel1,
            channel2 => channel2,
            max_count => max_count,
            overflow => overflow,
            m_axis_aclk => m_axis_aclk,
            m_axis_aresetn => m_axis_aresetn,
            m_axis_tvalid => m_axis_tvalid,
            m_axis_tdata => m_axis_tdata,
            m_axis_tready => m_axis_tready
        ); 
        
   -- Clock process definitions
   Clk_process :process
   begin
        m_axis_aclk <= '0';
        wait for CLK_PERIOD/2;
        m_axis_aclk <= '1';
        wait for CLK_PERIOD/2;
   end process;
    
   -- Stimulus process, Apply inputs here.
  stim_proc: process
   begin        
        wait for CLK_PERIOD*5 + ENABLE_DELAY; -- wait for 5 clock cycles
                                              -- +2 ns for starting while clk is low
                                              -- +7 ns for starting while clk is high
        
        enable_cnt <='0';                  --enable the counter

        -- event generation upon reaching max count
        wait for CLK_PERIOD*(MAX_COUNT_TB+1);--ENABLE_DELAY+CLK_PERIOD/2;
        
        
        channel1 <= '1';
        wait for SIGNAL_DURATION;
        channel1 <= '0';
        
        wait for CLK_PERIOD*8;
        channel1 <= '1';
        wait for SIGNAL_DURATION;
        channel1 <= '0';

        wait for CLK_PERIOD*4;
        channel1 <= '1';
        wait for CLK_PERIOD;
        channel2 <= '1';
        wait for 2*CLK_PERIOD;
        channel1 <= '0';
        wait for CLK_PERIOD;
        channel2 <= '0';

        -- simulate overflow
        wait for CLK_PERIOD*4 - ENABLE_DELAY;
        m_axis_tready <= '0';
        
        wait;

  end process;

end;