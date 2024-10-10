library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity iatcollector2ch_axis is
	generic (
		C_M_AXIS_TDATA_WIDTH	: integer	:= 16
	);
	port (
        enable_cnt : in std_logic;
        channel1   : in std_logic;
        channel2   : in std_logic;
        max_count  : in std_logic_vector(C_M_AXIS_TDATA_WIDTH-1 downto 0);
        overflow   : out std_logic;

		m_axis_aclk	     : in std_logic;
		m_axis_aresetn   : in std_logic;
		m_axis_tvalid	 : out std_logic;
		m_axis_tdata	 : out std_logic_vector(C_M_AXIS_TDATA_WIDTH-1 downto 0);
		m_axis_tready	 : in std_logic
	);
end iatcollector2ch_axis;

architecture behavior of iatcollector2ch_axis is

    -- inter-arrival time collector
    component iatcollector2ch is
    generic(
      DATA_WIDTH : integer
    );
	port (
      clk        : in std_logic;
      enable     : in std_logic;
      channel1   : in std_logic;
      channel2   : in std_logic;
      max_count  : in std_logic_vector(DATA_WIDTH - 1 downto 0);
      ready      : out std_logic;
      data       : out std_logic_vector(DATA_WIDTH - 1 downto 0)
    );
    end component;
    
    -- intermediate signals
    signal data_ready_i : std_logic;
    signal data_i       : std_logic_vector(C_M_AXIS_TDATA_WIDTH - 1 downto 0);
    signal overflow_i   : std_logic;

begin

	overflow_i <= '1' when (data_ready_i='1' and m_axis_tready='0') else
	              '0';
    
    prDelayOverflow : process(m_axis_aclk)
    begin 
      if (rising_edge(m_axis_aclk)) then
        overflow <= overflow_i; -- delay overflow signal to synchronize with m_axis_tvalid
      end if;
    end process;
		
	process( m_axis_aclk ) is
	begin
	  if (rising_edge(m_axis_aclk)) then
	    if (m_axis_aresetn = '0') then
	      m_axis_tvalid <= '0';
	      m_axis_tdata <= (others => '0');
	    else
	      m_axis_tvalid <= data_ready_i;
	      m_axis_tdata <= data_i;
	    end if;
	  end if;
	end process;
		
    -- instantiate the counter
   iatcollector2ch1 : iatcollector2ch
     generic map ( 
      C_M_AXIS_TDATA_WIDTH
     )
     port map (
       clk => m_axis_aclk,
       enable => enable_cnt, 
       channel1 => channel1,
       channel2 => channel2,
       max_count => max_count,
       ready => data_ready_i,
       data => data_i
     ); 
	
end behavior;
