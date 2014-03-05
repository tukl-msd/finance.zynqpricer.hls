----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 08/24/2013 01:58:16 PM
-- Design Name: 
-- Module Name: synth_tb - Behavioral
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


library STD;
use STD.textio.all;

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.std_logic_textio.all;
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity synth_tb is
    Port ( aclk : in STD_LOGIC;
           rstn : in STD_LOGIC;
           start : in STD_LOGIC;
           done : out STD_LOGIC;
           ready : out STD_LOGIC;
           failed : out STD_LOGIC);
end synth_tb;

architecture Behavioral of synth_tb is



component design_1_wrapper
  port (
    random_numbers_V_tdata : out STD_LOGIC_VECTOR ( 31 downto 0 );
    random_numbers_V_tlast : out STD_LOGIC;
    random_numbers_V_tready : in STD_LOGIC;
    random_numbers_V_tvalid : out STD_LOGIC;
    aclk : in STD_LOGIC;
    aresetn: in STD_LOGIC;    
    S_AXI_SLV0_araddr : in STD_LOGIC_VECTOR ( 31 downto 0 );
    S_AXI_SLV0_arburst : in STD_LOGIC_VECTOR ( 1 downto 0 );
    S_AXI_SLV0_arcache : in STD_LOGIC_VECTOR ( 3 downto 0 );
    S_AXI_SLV0_arlen : in STD_LOGIC_VECTOR ( 7 downto 0 );
    S_AXI_SLV0_arlock : in STD_LOGIC_VECTOR ( 0 to 0 );
    S_AXI_SLV0_arprot : in STD_LOGIC_VECTOR ( 2 downto 0 );
    S_AXI_SLV0_arqos : in STD_LOGIC_VECTOR ( 3 downto 0 );
    S_AXI_SLV0_arready : out STD_LOGIC;
    S_AXI_SLV0_arregion : in STD_LOGIC_VECTOR ( 3 downto 0 );
    S_AXI_SLV0_arsize : in STD_LOGIC_VECTOR ( 2 downto 0 );
    S_AXI_SLV0_arvalid : in STD_LOGIC;
    S_AXI_SLV0_awaddr : in STD_LOGIC_VECTOR ( 31 downto 0 );
    S_AXI_SLV0_awburst : in STD_LOGIC_VECTOR ( 1 downto 0 );
    S_AXI_SLV0_awcache : in STD_LOGIC_VECTOR ( 3 downto 0 );
    S_AXI_SLV0_awlen : in STD_LOGIC_VECTOR ( 7 downto 0 );
    S_AXI_SLV0_awlock : in STD_LOGIC_VECTOR ( 0 to 0 );
    S_AXI_SLV0_awprot : in STD_LOGIC_VECTOR ( 2 downto 0 );
    S_AXI_SLV0_awqos : in STD_LOGIC_VECTOR ( 3 downto 0 );
    S_AXI_SLV0_awready : out STD_LOGIC;
    S_AXI_SLV0_awregion : in STD_LOGIC_VECTOR ( 3 downto 0 );
    S_AXI_SLV0_awsize : in STD_LOGIC_VECTOR ( 2 downto 0 );
    S_AXI_SLV0_awvalid : in STD_LOGIC;
    S_AXI_SLV0_bready : in STD_LOGIC;
    S_AXI_SLV0_bresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
    S_AXI_SLV0_bvalid : out STD_LOGIC;
    S_AXI_SLV0_rdata : out STD_LOGIC_VECTOR ( 31 downto 0 );
    S_AXI_SLV0_rlast : out STD_LOGIC;
    S_AXI_SLV0_rready : in STD_LOGIC;
    S_AXI_SLV0_rresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
    S_AXI_SLV0_rvalid : out STD_LOGIC;
    S_AXI_SLV0_wdata : in STD_LOGIC_VECTOR ( 31 downto 0 );
    S_AXI_SLV0_wlast : in STD_LOGIC;
    S_AXI_SLV0_wready : out STD_LOGIC;
    S_AXI_SLV0_wstrb : in STD_LOGIC_VECTOR ( 3 downto 0 );
    S_AXI_SLV0_wvalid : in STD_LOGIC
  );
end component;

signal random_numbers_V_tdata : STD_LOGIC_VECTOR ( 31 downto 0 );
signal random_numbers_V_tlast : STD_LOGIC;
signal random_numbers_V_tready : STD_LOGIC;
signal random_numbers_V_tvalid : STD_LOGIC;
signal S_AXI_SLV0_araddr : STD_LOGIC_VECTOR ( 31 downto 0 );
signal S_AXI_SLV0_arburst : STD_LOGIC_VECTOR ( 1 downto 0 );
signal S_AXI_SLV0_arcache : STD_LOGIC_VECTOR ( 3 downto 0 );
signal S_AXI_SLV0_arlen : STD_LOGIC_VECTOR ( 7 downto 0 );
signal S_AXI_SLV0_arlock : STD_LOGIC_VECTOR ( 0 to 0 );
signal S_AXI_SLV0_arprot : STD_LOGIC_VECTOR ( 2 downto 0 );
signal S_AXI_SLV0_arqos : STD_LOGIC_VECTOR ( 3 downto 0 );
signal S_AXI_SLV0_arready : STD_LOGIC;
signal S_AXI_SLV0_arregion : STD_LOGIC_VECTOR ( 3 downto 0 );
signal S_AXI_SLV0_arsize : STD_LOGIC_VECTOR ( 2 downto 0 );
signal S_AXI_SLV0_arvalid : STD_LOGIC;
signal S_AXI_SLV0_awaddr : STD_LOGIC_VECTOR ( 31 downto 0 );
signal S_AXI_SLV0_awburst : STD_LOGIC_VECTOR ( 1 downto 0 );
signal S_AXI_SLV0_awcache : STD_LOGIC_VECTOR ( 3 downto 0 );
signal S_AXI_SLV0_awlen : STD_LOGIC_VECTOR ( 7 downto 0 );
signal S_AXI_SLV0_awlock : STD_LOGIC_VECTOR ( 0 to 0 );
signal S_AXI_SLV0_awprot : STD_LOGIC_VECTOR ( 2 downto 0 );
signal S_AXI_SLV0_awqos : STD_LOGIC_VECTOR ( 3 downto 0 );
signal S_AXI_SLV0_awready : STD_LOGIC;
signal S_AXI_SLV0_awregion : STD_LOGIC_VECTOR ( 3 downto 0 );
signal S_AXI_SLV0_awsize : STD_LOGIC_VECTOR ( 2 downto 0 );
signal S_AXI_SLV0_awvalid : STD_LOGIC;
signal S_AXI_SLV0_bready : STD_LOGIC;
signal S_AXI_SLV0_bresp : STD_LOGIC_VECTOR ( 1 downto 0 );
signal S_AXI_SLV0_bvalid : STD_LOGIC;
signal S_AXI_SLV0_rdata : STD_LOGIC_VECTOR ( 31 downto 0 );
signal S_AXI_SLV0_rlast : STD_LOGIC;
signal S_AXI_SLV0_rready : STD_LOGIC;
signal S_AXI_SLV0_rresp : STD_LOGIC_VECTOR ( 1 downto 0 );
signal S_AXI_SLV0_rvalid : STD_LOGIC;
signal S_AXI_SLV0_wdata : STD_LOGIC_VECTOR ( 31 downto 0 );
signal S_AXI_SLV0_wlast : STD_LOGIC;
signal S_AXI_SLV0_wready : STD_LOGIC;
signal S_AXI_SLV0_wstrb : STD_LOGIC_VECTOR ( 3 downto 0 );
signal S_AXI_SLV0_wvalid : STD_LOGIC;

signal resetn : std_logic; -- synchronized reset signal
signal start_in: std_logic; -- synchronized start signal

signal is_started: boolean;
signal is_ready: std_logic;

constant NO_ADDR: std_logic_vector(31 downto 0) := (others => '0');

type read_state_type is (rs_idle, rs_wait_arready, rs_wait_rready, rs_done);
signal read_state: read_state_type;
signal read_addr: std_logic_vector (31 downto 0);
signal read_data: std_logic_vector (31 downto 0);

type write_state_type is (ws_idle, ws_wait_awready, ws_wait_wready, 
		ws_wait_bvalid, ws_done);
signal write_state: write_state_type;
signal write_addr: std_logic_vector(31 downto 0);
signal write_data: std_logic_vector(31 downto 0);

begin


reset_gen: process (aclk, rstn)
	variable r1, r2: std_logic;
begin
	if (rstn = '0') then
		r1 := '0';
		r2 := '0';
		resetn <= '0';
	elsif rising_edge(aclk) then
		resetn <= r2;
		r2 := r1;
		r1 := '1';
	end if;
end process;


start_sync: process (aclk, resetn)
	variable s1, s2: std_logic;
begin
	if (resetn = '0') then
		s1 := '0';
		s2 := '0';
		start_in <= '0';
	elsif rising_edge(aclk) then
		start_in <= s2;
		s2 := s1;
		s1 := start;
	end if;
end process;


ready <= is_ready;
gen_start: process (aclk, resetn)
begin
    if (resetn = '0') then
        is_ready <= '0';
        is_started <= false;
    elsif rising_edge(aclk) then
        if (is_ready = '1' and start_in = '1') then
            is_started <= true;
            is_ready <= '0';
        elsif (not is_started) then
            is_ready <= '1';
        end if;
    end if;
end process;


-- axi stream channel
random_numbers_V_tready <= '0';


read_cmd: process(aclk, resetn)
begin
	if (resetn = '0') then
		-- read address channel
		S_AXI_SLV0_araddr <= x"00000000";
		S_AXI_SLV0_arburst <= "00";
		S_AXI_SLV0_arcache <= "0011";
		S_AXI_SLV0_arlen <= x"00";
		--S_AXI_SLV0_arlock : in STD_LOGIC_VECTOR ( 0 to 0 );
		S_AXI_SLV0_arprot <= "000";
		--S_AXI_SLV0_arqos : in STD_LOGIC_VECTOR ( 3 downto 0 );
		----S_AXI_SLV0_arready : out STD_LOGIC;
		--S_AXI_SLV0_arregion : in STD_LOGIC_VECTOR ( 3 downto 0 );
		S_AXI_SLV0_arsize <= "010";
		S_AXI_SLV0_arvalid <= '0';

		-- read response channel
		----S_AXI_SLV0_rdata : out STD_LOGIC_VECTOR ( 31 downto 0 );
		----S_AXI_SLV0_rlast : out STD_LOGIC;
		S_AXI_SLV0_rready <= '0';
		----S_AXI_SLV0_rresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
		----S_AXI_SLV0_rvalid : out STD_LOGIC;

		read_state <= rs_idle;
	elsif rising_edge(aclk) then
		case read_state is
			when rs_idle =>
				if (read_addr /= NO_ADDR) then
					S_AXI_SLV0_araddr <= read_addr;
					S_AXI_SLV0_arvalid <= '1';
					read_state <= rs_wait_arready;
				end if;
			when rs_wait_arready =>
				if (S_AXI_SLV0_arready = '1') then
					S_AXI_SLV0_arvalid <= '0';
					S_AXI_SLV0_rready <= '1';
					read_state <= rs_wait_rready;
				end if;
			when rs_wait_rready =>
				if (S_AXI_SLV0_rvalid = '1') then
					S_AXI_SLV0_rready <= '0';
					read_data <= S_AXI_SLV0_rdata;
					read_state <= rs_done;
				end if;
			when rs_done =>
				read_state <= rs_idle;
		end case;
	end if;
end process;



write_cmd: process(aclk, resetn)
begin
    if (resetn = '0') then
		-- write addresss channel
		S_AXI_SLV0_awaddr <= x"00000000";
		S_AXI_SLV0_awburst <= "00";
		S_AXI_SLV0_awcache <= "0011";
		S_AXI_SLV0_awlen <= x"00";
		--S_AXI_SLV0_awlock : in STD_LOGIC_VECTOR ( 0 to 0 );
		S_AXI_SLV0_awprot <= "000";
		--S_AXI_SLV0_awqos : in STD_LOGIC_VECTOR ( 3 downto 0 );
		----S_AXI_SLV0_awready : out STD_LOGIC;
		--S_AXI_SLV0_awregion : in STD_LOGIC_VECTOR ( 3 downto 0 );
		S_AXI_SLV0_awsize <= "010";
		S_AXI_SLV0_awvalid <= '0';

		-- write response channel
		S_AXI_SLV0_bready <= '0';
		----S_AXI_SLV0_bresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
		----S_AXI_SLV0_bvalid : out STD_LOGIC;

		-- write data channel
		S_AXI_SLV0_wdata <= (others => '1');
		S_AXI_SLV0_wlast <= '1';
		----S_AXI_SLV0_wready : out STD_LOGIC;
		S_AXI_SLV0_wstrb <= "1111";
		S_AXI_SLV0_wvalid <= '0';

		write_state <= ws_idle;
    elsif rising_edge(aclk) then
		case write_state is
			when ws_idle =>
				if (write_addr /= NO_ADDR) then
					S_AXI_SLV0_awaddr <= write_addr;
					S_AXI_SLV0_awvalid <= '1';
					write_state <= ws_wait_awready;
				end if;
			when ws_wait_awready =>
				if (S_AXI_SLV0_awready = '1') then
					S_AXI_SLV0_awvalid <= '0';
					S_AXI_SLV0_wdata <= write_data;
					S_AXI_SLV0_wvalid <= '1';
					write_state <= ws_wait_wready;
				end if;
			when ws_wait_wready =>
				if (S_AXI_SLV0_wready = '1') then
					S_AXI_SLV0_wvalid <= '0';
					S_AXI_SLV0_bready <= '1';
					write_state <= ws_wait_bvalid;
				end if;
			when ws_wait_bvalid =>
				if (S_AXI_SLV0_bvalid = '1') then
					S_AXI_SLV0_bready <= '0';
					write_state <= ws_done;
--					if (S_AXI_SLV0_bresp /= "00") then
--						failed <= '1';
--						done <= '1';
--					end if;
				end if;
			when ws_done =>
				write_state <= ws_idle;
		end case;
    end if;
end process;


testbench: process (aclk, resetn)
--pragma synthesis_off
	function to_string(sv: std_Logic_Vector) return string is
		variable lp: line;
	begin
		hwrite(lp, sv);
		return lp.all;
	end;
--pragma synthesis_on

	variable cnt : integer range 0 to 100;

	variable mt_state_cnt : integer range 0 to 624;
	variable do_wait : boolean := false;

	variable wait_cnt : integer range 0 to 1000;

	variable rn_cnt : integer range 0 to 100;
	variable any_rn_not_zero : boolean;
begin
    if (resetn = '0') then
        done <= '0';
        failed <= '0';
		cnt := 0;
		read_addr <= NO_ADDR;
		write_addr <= NO_ADDR;
    elsif rising_edge(aclk) then
        if (is_started) then
			case cnt is
				-- read status register
				when 0 =>
					read_addr <= x"44A00000";
					cnt := cnt + 1;
				when 1 =>
					read_addr <= NO_ADDR;
					if (read_state = rs_done) then
--pragma synthesis_off
						report "MT status register: " & to_string(read_data);
--pragma synthesis_on
						cnt := cnt + 1;
					end if;

				-- write mt state
				when 2 =>
					do_wait := false;
					mt_state_cnt := 0;
					cnt := cnt + 1;
				when 3 =>
					if (mt_state_cnt >= 624) then
						cnt := cnt + 1;
					elsif (not do_wait) then
						write_addr <= std_logic_vector(x"44A01000" +
									to_unsigned(4 * mt_state_cnt, 32));
						write_data <= std_logic_vector(to_unsigned(
									mt_state_cnt, 32));
						do_wait := true;
					elsif (write_state = ws_done) then
						write_addr <= NO_ADDR;
						do_wait := false;
						mt_state_cnt := mt_state_cnt + 1;
					end if;

				-- start accelerator
				when 4 =>
					write_addr <= x"44A00000";
					-- auto restart (bit 7) + start (bit 0)
					write_data <= x"00000081";
					cnt := cnt + 1;
				when 5 =>
					write_addr <= NO_ADDR;
					cnt := cnt + 1;

				-- wait 1000 cycles
				when 6 =>
					wait_cnt := 0;
					cnt := cnt + 1;
				when 7 =>
					if (wait_cnt >= 1000) then
						cnt := cnt + 1;
					else
						wait_cnt := wait_cnt + 1;
					end if;

				-- read occupancy
				when 8 =>
					read_addr <= x"7600001C";
					cnt := cnt + 1;
				when 9 =>
					read_addr <= NO_ADDR;
					if (read_state = rs_done) then
--pragma synthesis_off
						report "Fifo occupancy: " & to_string(read_data);
--pragma synthesis_on
						if (unsigned(read_data) < to_unsigned(100, 32)) then
							failed <= '1';
							done <= '1';
						end if;
						cnt := cnt + 1;
					end if;

				-- read 100 random numbers
				when 10 =>
					rn_cnt := 0;
					do_wait := false;
					any_rn_not_zero := false;
					cnt := cnt + 1;
				when 11 =>
					if (rn_cnt >= 100) then
						cnt := cnt + 1;
					elsif (not do_wait) then
						read_addr <= x"76000020";
						do_wait := true;
					elsif (read_state = rs_done) then
						read_addr <= NO_ADDR;
						do_wait := false;
						if (read_data /= x"00000000") then
							any_rn_not_zero := true;
						end if;
						rn_cnt := rn_cnt + 1;
					end if;
				when 12 =>
					if (any_rn_not_zero) then
						cnt := cnt + 1;
					else
						failed <= '1';
						done <= '1';
					end if;

				-- done
				when 13 =>
					done <= '1';

				-- default => failed
				when others =>
					failed <= '1';
					done <= '1';
			end case;
        end if;
    end if;
end process;





dut: component design_1_wrapper
    port map (
      random_numbers_V_tdata(31 downto 0) => random_numbers_V_tdata(31 downto 0),
      random_numbers_V_tlast => random_numbers_V_tlast,
      random_numbers_V_tready => random_numbers_V_tready,
      random_numbers_V_tvalid => random_numbers_V_tvalid,
      aclk => aclk,
      aresetn => resetn,
      S_AXI_SLV0_araddr(31 downto 0) => S_AXI_SLV0_araddr(31 downto 0),
      S_AXI_SLV0_arburst(1 downto 0) => S_AXI_SLV0_arburst(1 downto 0),
      S_AXI_SLV0_arcache(3 downto 0) => S_AXI_SLV0_arcache(3 downto 0),
      S_AXI_SLV0_arlen(7 downto 0) => S_AXI_SLV0_arlen(7 downto 0),
      S_AXI_SLV0_arlock(0) => S_AXI_SLV0_arlock(0),
      S_AXI_SLV0_arprot(2 downto 0) => S_AXI_SLV0_arprot(2 downto 0),
      S_AXI_SLV0_arqos(3 downto 0) => S_AXI_SLV0_arqos(3 downto 0),
      S_AXI_SLV0_arready => S_AXI_SLV0_arready,
      S_AXI_SLV0_arregion(3 downto 0) => S_AXI_SLV0_arregion(3 downto 0),
      S_AXI_SLV0_arsize(2 downto 0) => S_AXI_SLV0_arsize(2 downto 0),
      S_AXI_SLV0_arvalid => S_AXI_SLV0_arvalid,
      S_AXI_SLV0_awaddr(31 downto 0) => S_AXI_SLV0_awaddr(31 downto 0),
      S_AXI_SLV0_awburst(1 downto 0) => S_AXI_SLV0_awburst(1 downto 0),
      S_AXI_SLV0_awcache(3 downto 0) => S_AXI_SLV0_awcache(3 downto 0),
      S_AXI_SLV0_awlen(7 downto 0) => S_AXI_SLV0_awlen(7 downto 0),
      S_AXI_SLV0_awlock(0) => S_AXI_SLV0_awlock(0),
      S_AXI_SLV0_awprot(2 downto 0) => S_AXI_SLV0_awprot(2 downto 0),
      S_AXI_SLV0_awqos(3 downto 0) => S_AXI_SLV0_awqos(3 downto 0),
      S_AXI_SLV0_awready => S_AXI_SLV0_awready,
      S_AXI_SLV0_awregion(3 downto 0) => S_AXI_SLV0_awregion(3 downto 0),
      S_AXI_SLV0_awsize(2 downto 0) => S_AXI_SLV0_awsize(2 downto 0),
      S_AXI_SLV0_awvalid => S_AXI_SLV0_awvalid,
      S_AXI_SLV0_bready => S_AXI_SLV0_bready,
      S_AXI_SLV0_bresp(1 downto 0) => S_AXI_SLV0_bresp(1 downto 0),
      S_AXI_SLV0_bvalid => S_AXI_SLV0_bvalid,
      S_AXI_SLV0_rdata(31 downto 0) => S_AXI_SLV0_rdata(31 downto 0),
      S_AXI_SLV0_rlast => S_AXI_SLV0_rlast,
      S_AXI_SLV0_rready => S_AXI_SLV0_rready,
      S_AXI_SLV0_rresp(1 downto 0) => S_AXI_SLV0_rresp(1 downto 0),
      S_AXI_SLV0_rvalid => S_AXI_SLV0_rvalid,
      S_AXI_SLV0_wdata(31 downto 0) => S_AXI_SLV0_wdata(31 downto 0),
      S_AXI_SLV0_wlast => S_AXI_SLV0_wlast,
      S_AXI_SLV0_wready => S_AXI_SLV0_wready,
      S_AXI_SLV0_wstrb(3 downto 0) => S_AXI_SLV0_wstrb(3 downto 0),
      S_AXI_SLV0_wvalid => S_AXI_SLV0_wvalid
    );


end Behavioral;
