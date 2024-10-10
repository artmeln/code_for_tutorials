library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- entity declaration for your testbench. 
entity tb_iatcollector2chSmart_axis is
end tb_iatcollector2chSmart_axis;

architecture behavior of tb_iatcollector2chSmart_axis is

-- component declaration for the unit under test (uut)
component iatcollector2chSmart_axis is
    generic(
		C_M_AXIS_TDATA_WIDTH	: integer;
		C_S_AXI_DATA_WIDTH	    : integer;
		C_S_AXI_ADDR_WIDTH	    : integer
	);
	port (
        channel1   : in std_logic;
        channel2   : in std_logic;
        fifo_overflow   : out std_logic;
		M_AXIS_ACLK	: in std_logic;
		M_AXIS_ARESETN	: in std_logic;
		M_AXIS_TVALID	: out std_logic;
		M_AXIS_TDATA	: out std_logic_vector(C_M_AXIS_TDATA_WIDTH-1 downto 0);
		M_AXIS_TREADY	: in std_logic;
		S_AXI_ACLK	: in std_logic;
		S_AXI_ARESETN	: in std_logic;
		S_AXI_AWADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_AWPROT	: in std_logic_vector(2 downto 0);
		S_AXI_AWVALID	: in std_logic;
		S_AXI_AWREADY	: out std_logic;
		S_AXI_WDATA	: in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_WSTRB	: in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
		S_AXI_WVALID	: in std_logic;
		S_AXI_WREADY	: out std_logic;
		S_AXI_BRESP	: out std_logic_vector(1 downto 0);
		S_AXI_BVALID	: out std_logic;
		S_AXI_BREADY	: in std_logic;
		S_AXI_ARADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_ARPROT	: in std_logic_vector(2 downto 0);
		S_AXI_ARVALID	: in std_logic;
		S_AXI_ARREADY	: out std_logic;
		S_AXI_RDATA	: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_RRESP	: out std_logic_vector(1 downto 0);
		S_AXI_RVALID	: out std_logic;
		S_AXI_RREADY	: in std_logic
	);
end component;

constant C_M_AXIS_TDATA_WIDTH_TB : integer := 16;
constant C_S_AXI_DATA_WIDTH_TB : integer := 32;
constant C_S_AXI_ADDR_WIDTH_TB : integer := 4;
constant MAX_COUNT_TB  : integer := 6; 
constant CLK_PERIOD : time := 10 ns;
constant SIGNAL_DURATION : time := 3*CLK_PERIOD;
constant ENABLE_DELAY : time := 2 ns;

--declare inputs and initialize
signal channel1         : std_logic := '0';
signal channel2         : std_logic := '0';
signal M_AXIS_ACLK      : std_logic := '0';
signal M_AXIS_ARESETN	: std_logic := '0';
signal M_AXIS_TREADY	: std_logic := '1';
signal S_AXI_ACLK       : std_logic := '0';
signal S_AXI_ARESETN    : std_logic := '0';
signal S_AXI_AWADDR     : std_logic_vector(C_S_AXI_ADDR_WIDTH_TB-1 downto 0) := (others => '0');
signal S_AXI_AWPROT     : std_logic_vector(2 downto 0) := "000"; -- not used in this component
signal S_AXI_AWVALID	: std_logic := '0';
signal S_AXI_WDATA      : std_logic_vector(C_S_AXI_DATA_WIDTH_TB-1 downto 0) := (others => '0');
signal S_AXI_WSTRB      : std_logic_vector((C_S_AXI_DATA_WIDTH_TB/8)-1 downto 0) := (others => '1'); -- keep all bytes
signal S_AXI_WVALID     : std_logic := '0';
signal S_AXI_BREADY     : std_logic := '1';
signal S_AXI_ARADDR     : std_logic_vector(C_S_AXI_ADDR_WIDTH_TB-1 downto 0) := (others => '1');
signal S_AXI_ARPROT     : std_logic_vector(2 downto 0) := "000"; -- not used in this component
signal S_AXI_ARVALID	: std_logic := '0';
signal S_AXI_RREADY     : std_logic := '0';

--declare outputs
signal fifo_overflow    : std_logic;
signal M_AXIS_TVALID	: std_logic;
signal M_AXIS_TDATA	    : std_logic_vector(C_M_AXIS_TDATA_WIDTH_TB-1 downto 0);
signal S_AXI_AWREADY	: std_logic;
signal S_AXI_WREADY	    : std_logic;
signal S_AXI_BRESP      : std_logic_vector(1 downto 0);
signal S_AXI_BVALID     : std_logic;
signal S_AXI_ARREADY	: std_logic;
signal S_AXI_RDATA      : std_logic_vector(C_S_AXI_DATA_WIDTH_TB-1 downto 0);
signal S_AXI_RRESP      : std_logic_vector(1 downto 0);
signal S_AXI_RVALID     : std_logic;

begin

    -- instantiate the unit under test (uut)
   uut : iatcollector2chSmart_axis 
     generic map (
        C_M_AXIS_TDATA_WIDTH_TB,
        C_S_AXI_DATA_WIDTH_TB,
		C_S_AXI_ADDR_WIDTH_TB
     ) port map (
        channel1 => channel1,
        channel2 => channel2,
        fifo_overflow => fifo_overflow,
		M_AXIS_ACLK => M_AXIS_ACLK,
		M_AXIS_ARESETN => M_AXIS_ARESETN,
		M_AXIS_TVALID => M_AXIS_TVALID,
		M_AXIS_TDATA => M_AXIS_TDATA,
		M_AXIS_TREADY => M_AXIS_TREADY,
		S_AXI_ACLK => S_AXI_ACLK,
		S_AXI_ARESETN => S_AXI_ARESETN,
		S_AXI_AWADDR => S_AXI_AWADDR,
		S_AXI_AWPROT => S_AXI_AWPROT,
		S_AXI_AWVALID => S_AXI_AWVALID,
		S_AXI_AWREADY => S_AXI_AWREADY,
		S_AXI_WDATA => S_AXI_WDATA,
		S_AXI_WSTRB => S_AXI_WSTRB,
		S_AXI_WVALID => S_AXI_WVALID,
		S_AXI_WREADY => S_AXI_WREADY,
		S_AXI_BRESP => S_AXI_BRESP,
		S_AXI_BVALID => S_AXI_BVALID,
		S_AXI_BREADY => S_AXI_BREADY,
		S_AXI_ARADDR => S_AXI_ARADDR,
		S_AXI_ARPROT => S_AXI_ARPROT,
		S_AXI_ARVALID => S_AXI_ARVALID,
		S_AXI_ARREADY => S_AXI_ARREADY,
		S_AXI_RDATA => S_AXI_RDATA,
		S_AXI_RRESP => S_AXI_RRESP,
		S_AXI_RVALID => S_AXI_RVALID,
		S_AXI_RREADY => S_AXI_RREADY
      ); 
        
   -- Clock process definitions
   Clk_process_M :process
   begin
        M_AXIS_ACLK <= '0';
        wait for CLK_PERIOD/2;
        M_AXIS_ACLK <= '1';
        wait for CLK_PERIOD/2;
   end process;
    
   Clk_process_S :process
   begin
        S_AXI_ACLK <= '0';
        wait for CLK_PERIOD/2;
        S_AXI_ACLK <= '1';
        wait for CLK_PERIOD/2;
   end process;

   -- Stimulus process
  stim_proc: process
   begin        
        wait for CLK_PERIOD*5 + ENABLE_DELAY; -- wait for 5 clock cycles
                                              -- +2 ns for starting while clk is low
                                              -- +7 ns for starting while clk is high
        
        M_AXIS_ARESETN <= '1';
        S_AXI_ARESETN <= '1';
        
        -- set enable register to disable the counter
        S_AXI_AWVALID <= '1';
        S_AXI_AWADDR <= "0000";
        S_AXI_WVALID <= '1';
        S_AXI_WDATA <= std_logic_vector(to_unsigned(0,C_S_AXI_DATA_WIDTH_TB));        
        wait for CLK_PERIOD*6;
        S_AXI_AWVALID <= '0';
        S_AXI_WVALID <= '0';        
        wait for CLK_PERIOD*4;

        -- set max_count register to MAX_COUNT_TB
        S_AXI_AWVALID <= '1';
        S_AXI_AWADDR <= "0100";
        S_AXI_WVALID <= '1';
        S_AXI_WDATA <= std_logic_vector(to_unsigned(MAX_COUNT_TB,C_S_AXI_DATA_WIDTH_TB));        
        wait for CLK_PERIOD*6;
        S_AXI_AWVALID <= '0';
        S_AXI_WVALID <= '0';        
        wait for CLK_PERIOD*4;

        -- set enable register to enable the counter
        S_AXI_AWVALID <= '1';
        S_AXI_AWADDR <= "0000";
        S_AXI_WVALID <= '1';
        S_AXI_WDATA <= std_logic_vector(to_unsigned(1,C_S_AXI_DATA_WIDTH_TB));        
        wait for CLK_PERIOD*6;
        S_AXI_AWVALID <= '0';
        S_AXI_WVALID <= '0';        
        wait for CLK_PERIOD*4;

        -- read register 1
        S_AXI_ARVALID <= '1';
        S_AXI_ARADDR <= "0100";
        S_AXI_RREADY <= '1';
        wait for CLK_PERIOD*6;
        S_AXI_ARVALID <= '0';
        S_AXI_RREADY <= '0';        
        wait for CLK_PERIOD*4;

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

        -- simulate ocerflow of FIFO
        wait for CLK_PERIOD*4;
        M_AXIS_TREADY <= '0';
        
        wait;

  end process;

end;