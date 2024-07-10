
`ifndef CV32E40S_PKG_
`define CV32E40S_PKG_
    `include "cv32e40s/include/cv32e40s_pkg.sv"
`endif

`ifndef DM_PKG_
`define DM_PKG_
    `include "dm/dm_pkg.sv"
`endif

module CSoC_top import cv32e40s_pkg::*;
#(
    parameter int unsigned SOC_CLOCK_FREQUENCY = 50_000_000,

    parameter bit          CORE_DEBUG = 1,
    parameter int          CORE_DBG_NUM_TRIGGERS = 1,
    parameter int unsigned CORE_PMP_GRANULARITY = 0,
    parameter int          CORE_PMP_NUM_REGIONS = 4,
    parameter pmpncfg_t    CORE_PMP_PMPNCFG_RV[CORE_PMP_NUM_REGIONS-1:0] = '{default:PMPNCFG_DEFAULT},
    parameter logic [31:0] CORE_PMP_PMPADDR_RV[CORE_PMP_NUM_REGIONS-1:0] = '{default:32'h0},
    parameter mseccfg_t    CORE_PMP_MSECCFG_RV = MSECCFG_DEFAULT,
    parameter int          CORE_PMA_NUM_REGIONS = 0,
    parameter pma_cfg_t    CORE_PMA_CFG[CORE_PMA_NUM_REGIONS-1:0]= '{default:NO_PMA_R_DEFAULT},
    parameter lfsr_cfg_t   CORE_LFSR0_CFG = LFSR_CFG_DEFAULT, // TODO: This default value should not be used
    parameter lfsr_cfg_t   CORE_LFSR1_CFG = LFSR_CFG_DEFAULT, // TODO: This default value should not be used
    parameter lfsr_cfg_t   CORE_LFSR2_CFG = LFSR_CFG_DEFAULT, // TODO: This default value should not be used
    parameter logic [31:0] core_mhartid_i = 0,
    parameter logic [3:0]  core_mimpid_patch_i = 0,
    
    parameter              RAM_INIT_FILE = "vmem/ram.vmem",
    
    parameter int unsigned GPIO_INPUT_SIZE = 8,
    parameter int unsigned GPIO_OUTPUT_SIZE = 16,
    
    parameter int unsigned UART_BAUD_RATE = 500_000,
    
    parameter bit          SPI_CPOL = 0,
    parameter bit          SPI_CPHA = 1  
)
(
    input  logic clk_sys_i,
    input  logic rst_sys_ni,
    
    input  logic [GPIO_INPUT_SIZE-1:0]  gpio_i,
    output logic [GPIO_OUTPUT_SIZE-1:0] gpio_o,
    
    input  logic uart_rx_i,
    output logic uart_tx_o,
    
    input  logic spi_rx_i,
    output logic spi_tx_o,
    output logic spi_sck_o,
    
    input  logic dtm_tck_i,    // JTAG test clock pad
    input  logic dtm_tms_i,    // JTAG test mode select pad
    input  logic dtm_trst_ni,  // JTAG test reset pad
    input  logic dtm_td_i,     // JTAG test data input pad
    output logic dtm_td_o,     // JTAG test data output pad
    output logic dtm_tdo_oe_o  // Data out output enable
);

/*
  ___   ___   ___   ___  _   ___    _   __  __ ___ _____ ___ ___ 
 / __| / _ \ / __| | _ \/_\ | _ \  /_\ |  \/  | __|_   _| __| _ \
 \__ \| (_) | (__  |  _/ _ \|   / / _ \| |\/| | _|  | | | _||   /
 |___(_)___(_)___| |_|/_/ \_\_|_\/_/ \_\_|  |_|___| |_| |___|_|_\
                                                                 
*/
// Number of bus hosts and bus slaves (devices) in the SoC
localparam int          NR_HOSTS      = (CORE_DEBUG == 1) ? 2 : 1;
localparam int          NR_DEVICES    = (CORE_DEBUG == 1) ? 7 : 6;

// Memory scheme
localparam logic [31:0] SIM_CTRL_SIZE  =  1 * 1024; //  1 KiB
localparam logic [31:0] SIM_CTRL_START = 32'h20000;
localparam logic [31:0] SIM_CTRL_MASK  = ~(SIM_CTRL_SIZE-1);

localparam logic [31:0] MEM_SIZE       = 512 * 1024; // 512 KiB
localparam logic [31:0] MEM_START      = 32'h00100000;
localparam logic [31:0] MEM_MASK       = ~(MEM_SIZE-1);

localparam logic [31:0] DEBUG_SIZE     = 64 * 1024; // 64 KiB
localparam logic [31:0] DEBUG_START    = 32'h1a110000;
localparam logic [31:0] DEBUG_MASK     = ~(DEBUG_SIZE-1);

localparam logic [31:0] GPIO_SIZE      =  4 * 1024; //  4 KiB
localparam logic [31:0] GPIO_START     = 32'h80000000;
localparam logic [31:0] GPIO_MASK      = ~(GPIO_SIZE-1);

localparam logic [31:0] UART_SIZE      =  4 * 1024; //  4 KiB
localparam logic [31:0] UART_START     = 32'h80001000;
localparam logic [31:0] UART_MASK      = ~(UART_SIZE-1);

localparam logic [31:0] TIMER_SIZE     =  4 * 1024; //  4 KiB
localparam logic [31:0] TIMER_START    = 32'h80002000;
localparam logic [31:0] TIMER_MASK     = ~(TIMER_SIZE-1);

localparam logic [31:0] SPI_SIZE       = 1 * 1024; // 1kB
localparam logic [31:0] SPI_START      = 32'h80004000;
localparam logic [31:0] SPI_MASK       = ~(SPI_SIZE-1);

// Core reset boot address and reset mtvec address feeded into the core
localparam logic [31:0] core_mtvec_addr = MEM_START;
localparam logic [31:0] core_boot_addr  = MEM_START + 32'h80;

// Definition of the interrupt mode of the core. Hardwired to work in CLINT mode.
localparam CORE_CLIC = 0;
localparam CORE_CLIC_ID_WIDTH = 5;

// Debug module addresses feeded into the core
localparam logic [31:0] CORE_DM_REGION_START   = 32'hF0000000;
localparam logic [31:0] CORE_DM_REGION_END     = 32'hF0003FFF;
localparam logic [31:0] core_dm_halt_addr      = DEBUG_START + dm::HaltAddress[31:0];
localparam logic [31:0] core_dm_exception_addr = DEBUG_START + dm::ExceptionAddress[31:0];

/*
  ___ _   _ ___   ___ ___ ___ _  _   _   _    ___ 
 | _ ) | | / __| / __|_ _/ __| \| | /_\ | |  / __|
 | _ \ |_| \__ \ \__ \| | (_ | .` |/ _ \| |__\__ \
 |___/\___/|___/ |___/___\___|_|\_/_/ \_\____|___/
                                                  
*/
// typedefs for indexing arrays of bus hosts and slaves (devices)
typedef enum int { CoreD, DbgHost } bus_host_e;
typedef enum int { Ram, Gpio, Uart, Timer, Spi, SimCtrl, DbgDev } bus_device_e;

// Host signals
logic        host_req      [NR_HOSTS];
logic        host_gnt      [NR_HOSTS];
logic [31:0] host_addr     [NR_HOSTS];
logic        host_we       [NR_HOSTS];
logic [ 3:0] host_be       [NR_HOSTS];
logic [31:0] host_wdata    [NR_HOSTS];
logic        host_rvalid   [NR_HOSTS];
logic [31:0] host_rdata    [NR_HOSTS];
logic        host_err      [NR_HOSTS];

// Device signals
logic        device_req    [NR_DEVICES];
logic [31:0] device_addr   [NR_DEVICES];
logic        device_we     [NR_DEVICES];
logic [ 3:0] device_be     [NR_DEVICES];
logic [31:0] device_wdata  [NR_DEVICES];
logic        device_rvalid [NR_DEVICES];
logic [31:0] device_rdata  [NR_DEVICES];
logic        device_err    [NR_DEVICES];

// Device address mapping
logic [31:0] cfg_device_addr_base [NR_DEVICES];
logic [31:0] cfg_device_addr_mask [NR_DEVICES];

assign cfg_device_addr_base[Ram]     = MEM_START;
assign cfg_device_addr_mask[Ram]     = MEM_MASK;
assign cfg_device_addr_base[Gpio]    = GPIO_START;
assign cfg_device_addr_mask[Gpio]    = GPIO_MASK;
assign cfg_device_addr_base[Uart]    = UART_START;
assign cfg_device_addr_mask[Uart]    = UART_MASK;
assign cfg_device_addr_base[Timer]   = TIMER_START;
assign cfg_device_addr_mask[Timer]   = TIMER_MASK;
assign cfg_device_addr_base[Spi]     = SPI_START;
assign cfg_device_addr_mask[Spi]     = SPI_MASK;
assign cfg_device_addr_base[SimCtrl] = SIM_CTRL_START;
assign cfg_device_addr_mask[SimCtrl] = SIM_CTRL_MASK;

// If CORE_DEBUG == 1, configure address and mask for the debug module
if (CORE_DEBUG == 1) begin : g_dbg_device_cfg
    assign cfg_device_addr_base[DbgDev] = DEBUG_START;
    assign cfg_device_addr_mask[DbgDev] = DEBUG_MASK;
    assign device_err[DbgDev]           = 1'b0;
end

// These devices don't implement error signaling so we tie-off them
assign device_err[Ram]     = 1'b0;
assign device_err[Gpio]    = 1'b0;
assign device_err[Uart]    = 1'b0;
assign device_err[Spi]     = 1'b0;
assign device_err[SimCtrl] = 1'b0;

/*
  ___ _  _ _____ ___ ___ ___ _   _ ___ _____   ___ ___ ___ _  _   _   _    ___ 
 |_ _| \| |_   _| __| _ \ _ \ | | | _ \_   _| / __|_ _/ __| \| | /_\ | |  / __|
  | || .` | | | | _||   /   / |_| |  _/ | |   \__ \| | (_ | .` |/ _ \| |__\__ \
 |___|_|\_| |_| |___|_|_\_|_\\___/|_|   |_|   |___/___\___|_|\_/_/ \_\____|___/
                                                                               
*/
// TODO: Understand the HW/SW protocol to make the devices de-assert the irq signals
logic timer_irq;
logic uart_irq;
logic [31:0] core_irq;

assign core_irq = {15'b0 ,uart_irq, 8'b0, timer_irq, 7'b0};

/*
  ___   ___   ___ ___ ___ _  _   _   _    ___ 
 |_ _| | __| / __|_ _/ __| \| | /_\ | |  / __|
  | | _| _|  \__ \| | (_ | .` |/ _ \| |__\__ \
 |___(_)_(_) |___/___\___|_|\_/_/ \_\____|___/
                                              
 Istruction Fetch: core <-> instr. memory / core <-> debug module
*/
// Core IF interface signals
logic        core_instr_req;
logic        core_instr_reqpar;
logic        core_instr_gnt;
logic [31:0] core_instr_addr;
logic [1:0]  core_instr_memtype;
logic [2:0]  core_instr_prot;
logic [12:0] core_instr_achk;
logic        core_instr_dbg;
logic        core_instr_rvalid;
logic [31:0] core_instr_rdata;

// Memory IF interface signals
logic [31:0] mem_instr_rdata;

// Debug module IF interface signals
// logic [31:0] dbg_device_rdata; deprecated, now we use device_rdata[DbgDev] directly

// Mem and dbg instruction req signals
logic mem_instr_req;
logic dbg_instr_req;

// Selection signal for choosing if mem or debug module is delivering the instruction
logic core_instr_sel_dbg;

// IF from memory
assign mem_instr_req =
    core_instr_req & ((core_instr_addr & cfg_device_addr_mask[Ram]) == cfg_device_addr_base[Ram]);

// IF from debug module. If CORE_DEBUG != 1 the fetches from debug module are inhibited
assign dbg_instr_req = (CORE_DEBUG == 1) ? 
    core_instr_req & ((core_instr_addr & cfg_device_addr_mask[DbgDev]) == cfg_device_addr_base[DbgDev]) : 1'b0;

// Grant to core
assign core_instr_gnt = mem_instr_req | (dbg_instr_req & ~device_req[DbgDev]);

// Two flip-flop to emulate 1 cycle delay in core_instr_rvalid
// TODO: Why we need this emulation? Are the memory / debug module not able to provide such signal?
always @(posedge clk_sys_i or negedge rst_sys_ni) begin
    if (!rst_sys_ni) begin
        core_instr_rvalid  <= 1'b0;
        core_instr_sel_dbg <= 1'b0;
    end else begin
        core_instr_rvalid  <= core_instr_gnt;
        core_instr_sel_dbg <= dbg_instr_req;
    end
end

// Finally here we link the core_instr_rdata with the correct source
assign core_instr_rdata = core_instr_sel_dbg ? device_rdata[DbgDev] : mem_instr_rdata;

/*
  _      ___  _   _    ___ ___ ___ _  _   _   _    ___ 
 | |    / __|| | | |  / __|_ _/ __| \| | /_\ | |  / __|
 | |__ _\__ \| |_| |  \__ \| | (_ | .` |/ _ \| |__\__ \
 |____(_)___(_)___(_) |___/___\___|_|\_/_/ \_\____|___/
                                                       
 Data fetch: core <-> data memory
*/
logic        core_data_reqpar_o;
logic [1:0]  core_data_memtype_o;
logic [2:0]  core_data_prot_o;
logic        core_data_dbg_o;
logic [12:0] core_data_achk_o;

/*
   ___ ___  ___ ___   ___  ___ ___ _   _  ___   ___ ___ ___ _  _   _   _    ___ 
  / __/ _ \| _ \ __| |   \| __| _ ) | | |/ __| / __|_ _/ __| \| | /_\ | |  / __|
 | (_| (_) |   / _|  | |) | _|| _ \ |_| | (_ | \__ \| | (_ | .` |/ _ \| |__\__ \
  \___\___/|_|_\___| |___/|___|___/\___/ \___| |___/___\___|_|\_/_/ \_\____|___/
                                                                                
*/
logic        rst_core_n;
logic        ndmreset_req;
logic        core_debug_req;

logic        dm_device_req;
logic        dm_device_we;
logic [31:0] dm_device_addr;

logic        core_debug_havereset; // Unused
logic        core_debug_running;   // Unused
logic        core_debug_halted;    // Unused
logic        core_debug_pc_valid;  // Unused
logic [31:0] core_debug_pc;        // Unused

// reset signal for the core: the core is resetted if whole SoC is or if the DM decides so. He's the boss!
assign rst_core_n      = rst_sys_ni & ~ndmreset_req;

// DM receive a request if it is accessed for instruction or data
assign dm_device_req   = device_req[DbgDev] | dbg_instr_req;

// DM receives a write request only if DM is accessed for data writing and so through the bus
assign dm_device_we    = device_req[DbgDev] & device_we[DbgDev];

// Address is multiplexed from data bus or core IF interface
assign dm_device_addr  = device_req[DbgDev] ? device_addr[DbgDev] : core_instr_addr;

/*
  __  __ ___ ___  ___      ___ ___  ___ ___   ___ ___ ___ _  _   _   _    ___ 
 |  \/  |_ _/ __|/ __|    / __/ _ \| _ \ __| / __|_ _/ __| \| | /_\ | |  / __|
 | |\/| || |\__ \ (__ _  | (_| (_) |   / _|  \__ \| | (_ | .` |/ _ \| |__\__ \
 |_|  |_|___|___/\___(_)  \___\___/|_|_\___| |___/___\___|_|\_/_/ \_\____|___/
                                                                              
*/
logic [63:0] core_mcycle;
logic core_fencei_flush_req;
logic core_alert_major;
logic core_alert_minor;
logic core_core_sleep;
logic core_wu_wfe;

assign core_wu_wfe = '0;

/*
  ___  __  __ ___         ___  __  __   ___ ___ ___ _  _   _   _    ___ 
 |   \|  \/  |_ _|  ___  |   \|  \/  | / __|_ _/ __| \| | /_\ | |  / __|
 | |) | |\/| || |  |___| | |) | |\/| | \__ \| | (_ | .` |/ _ \| |__\__ \
 |___/|_|  |_|___|       |___/|_|  |_| |___/___\___|_|\_/_/ \_\____|___/
                                                                        
*/
logic          dmi_rst_n;
dm::dmi_req_t  dmi_req;
logic          dmi_req_valid;
logic          dmi_req_ready;
dm::dmi_resp_t dmi_resp;
logic          dmi_resp_ready;
logic          dmi_resp_valid;

/*
  ___ _   _ ___ 
 | _ ) | | / __|
 | _ \ |_| \__ \
 |___/\___/|___/
                
*/
bus #(
    .NrDevices            (           NR_DEVICES ),
    .NrHosts              (             NR_HOSTS ),
    .DataWidth            (                   32 ),
    .AddressWidth         (                   32 )
  ) u_bus (
    .clk_i                (            clk_sys_i ),
    .rst_ni               (           rst_sys_ni ),

    .host_req_i           (             host_req ),
    .host_gnt_o           (             host_gnt ),
    .host_addr_i          (            host_addr ),
    .host_we_i            (              host_we ),
    .host_be_i            (              host_be ),
    .host_wdata_i         (           host_wdata ),
    .host_rvalid_o        (          host_rvalid ),
    .host_rdata_o         (           host_rdata ),
    .host_err_o           (             host_err ),

    .device_req_o         (           device_req ),
    .device_addr_o        (          device_addr ),
    .device_we_o          (            device_we ),
    .device_be_o          (            device_be ),
    .device_wdata_o       (         device_wdata ),
    .device_rvalid_i      (        device_rvalid ),
    .device_rdata_i       (         device_rdata ),
    .device_err_i         (           device_err ),

    .cfg_device_addr_base ( cfg_device_addr_base ),
    .cfg_device_addr_mask ( cfg_device_addr_mask )
  );

/*
   _____   _________ ___ _ _   __  ___ 
  / __\ \ / /__ /_  ) __| | | /  \/ __|
 | (__ \ V / |_ \/ /| _||_  _| () \__ \
  \___| \_/ |___/___|___| |_| \__/|___/
                                       
*/
cv32e40s_core #(
    .LIB                      (                        0 ),
    .RV32                     (                    RV32I ),
    .B_EXT                    (                   B_NONE ),
    .M_EXT                    (                        M ),
    .DEBUG                    (               CORE_DEBUG ),
    .DM_REGION_START          (     CORE_DM_REGION_START ),
    .DM_REGION_END            (       CORE_DM_REGION_END ),
    .DBG_NUM_TRIGGERS         (    CORE_DBG_NUM_TRIGGERS ),
    .PMP_GRANULARITY          (     CORE_PMP_GRANULARITY ),
    .PMP_NUM_REGIONS          (     CORE_PMP_NUM_REGIONS ),
    .PMP_PMPNCFG_RV           (      CORE_PMP_PMPNCFG_RV ),
    .PMP_PMPADDR_RV           (      CORE_PMP_PMPADDR_RV ),
    .PMP_MSECCFG_RV           (      CORE_PMP_MSECCFG_RV ),
    .PMA_NUM_REGIONS          (     CORE_PMA_NUM_REGIONS ),
    .PMA_CFG                  (             CORE_PMA_CFG ),
    .CLIC                     (                CORE_CLIC ),
    .CLIC_ID_WIDTH            (       CORE_CLIC_ID_WIDTH ),
    .LFSR0_CFG                (           CORE_LFSR0_CFG ),
    .LFSR1_CFG                (           CORE_LFSR1_CFG ),
    .LFSR2_CFG                (           CORE_LFSR2_CFG )
) u_core (
    // Clock and reset
    .clk_i                    (                clk_sys_i ),
    .rst_ni                   (               rst_core_n ),
    .scan_cg_en_i             (                       '0 ), // Tied to 0 because we are in normal/functional operation

    // Configuration
    .boot_addr_i              (           core_boot_addr ),
    .mtvec_addr_i             (          core_mtvec_addr ),
    .dm_halt_addr_i           (        core_dm_halt_addr ),
    .dm_exception_addr_i      (   core_dm_exception_addr ),
    .mhartid_i                (           core_mhartid_i ),
    .mimpid_patch_i           (      core_mimpid_patch_i ),

    // Instruction memory interface
    .instr_req_o              (           core_instr_req ), // Going to Memory or Debug Module
    .instr_reqpar_o           (        core_instr_reqpar ), // Unused
    .instr_gnt_i              (           core_instr_gnt ), // Combinatorial logic to enable on instruction request
    .instr_gntpar_i           (          ~core_instr_gnt ), // Negation of the previous for odd parity
    .instr_addr_o             (          core_instr_addr ), // Going to Memory or Debug Module
    .instr_memtype_o          (       core_instr_memtype ), // Unused
    .instr_prot_o             (          core_instr_prot ), // Unused
    .instr_achk_o             (          core_instr_achk ), // Unused
    .instr_dbg_o              (           core_instr_dbg ), // Unused
    .instr_rvalid_i           (        core_instr_rvalid ), // 1 cycle-delay of core_instr_gnt, see in IF SIGNALS section
    .instr_rvalidpar_i        (       ~core_instr_rvalid ), // Negation of the previous for odd parity
    .instr_rdata_i            (         core_instr_rdata ), // Coming instruction word from memory or debug module
    .instr_err_i              (                       '0 ), // Tied to zero as an always-error-free IF interface
    .instr_rchk_i             (                       '0 ), // Tied to zero as an always-error-free core_instr_rdata

    // Data memory interface
    .data_req_o               (          host_req[CoreD] ),
    .data_reqpar_o            (       core_data_reqpar_o ), // Unused
    .data_gnt_i               (          host_gnt[CoreD] ),
    .data_gntpar_i            (         ~host_gnt[CoreD] ), // Negation of the previous for odd parity
    .data_addr_o              (         host_addr[CoreD] ),
    .data_be_o                (           host_be[CoreD] ),
    .data_memtype_o           (      core_data_memtype_o ), // Unused
    .data_prot_o              (         core_data_prot_o ), // Unused
    .data_dbg_o               (          core_data_dbg_o ), // Unused: Debug mode access, sent together with data_req_o.
    .data_wdata_o             (        host_wdata[CoreD] ),
    .data_we_o                (           host_we[CoreD] ),
    .data_achk_o              (         core_data_achk_o ), // Unused
    .data_rvalid_i            (       host_rvalid[CoreD] ),
    .data_rvalidpar_i         (      ~host_rvalid[CoreD] ), // Negation of the previous for odd parity
    .data_rdata_i             (        host_rdata[CoreD] ),
    .data_err_i               (          host_err[CoreD] ),
    .data_rchk_i              (                       '0 ), // Tied to zero as an always-error-free host_rdata[CoreD]

    // Cycle
    .mcycle_o                 (              core_mcycle ), // Unused

     // Interrupt interface
    .irq_i                    (                 core_irq ),

    .clic_irq_i               (                        '0), // Tied to zero because core is set in CLINT mode
    .clic_irq_id_i            (                        '0), // Tied to zero because core is set in CLINT mode
    .clic_irq_level_i         (                        '0), // Tied to zero because core is set in CLINT mode
    .clic_irq_priv_i          (                        '0), // Tied to zero because core is set in CLINT mode
    .clic_irq_shv_i           (                        '0), // Tied to zero because core is set in CLINT mode

    // Fencei flush handshake
    .fencei_flush_req_o       (    core_fencei_flush_req ), // Unused
    .fencei_flush_ack_i       (                       '1 ), // Tied to 1 to emulate flush protocol termination

    // Debug interface
    .debug_req_i              (           core_debug_req ),
    .debug_havereset_o        (     core_debug_havereset ), // Unused
    .debug_running_o          (       core_debug_running ), // Unused
    .debug_halted_o           (        core_debug_halted ), // Unused
    .debug_pc_valid_o         (      core_debug_pc_valid ), // Unused
    .debug_pc_o               (            core_debug_pc ), // Unused

     // Alert interface
    .alert_major_o            (         core_alert_major ), // Unused
    .alert_minor_o            (         core_alert_minor ), // Unused

    // Special control signals
    .fetch_enable_i           (                       '1 ),
    .core_sleep_o             (          core_core_sleep ),
    .wu_wfe_i                 (               core_wu_wfe)
);

/*
  ___    _   __  __ 
 | _ \  /_\ |  \/  |
 |   / / _ \| |\/| |
 |_|_\/_/ \_\_|  |_|
                    
*/
ram_2p #(
    .Depth       (      MEM_SIZE / 4 ),
    .MemInitFile (     RAM_INIT_FILE )
) u_ram (
    .clk_i  (              clk_sys_i ),
    .rst_ni (             rst_sys_ni ),
    
    .a_req_i    (    device_req[Ram] ),
    .a_we_i     (     device_we[Ram] ),
    .a_be_i     (     device_be[Ram] ),
    .a_addr_i   (   device_addr[Ram] ),
    .a_wdata_i  (  device_wdata[Ram] ),
    .a_rvalid_o ( device_rvalid[Ram] ),
    .a_rdata_o  (  device_rdata[Ram] ),

    .b_req_i    (      mem_instr_req ),
    .b_we_i     (                 '0 ),
    .b_be_i     (                 '0 ),
    .b_addr_i   (    core_instr_addr ),
    .b_wdata_i  (                 '0 ),
    .b_rvalid_o (                    ), // Unused: Why not connecting this to core_instr_rvalid?
    .b_rdata_o  (    mem_instr_rdata )
  );

/*
   ___ ___ ___ ___  
  / __| _ \_ _/ _ \ 
 | (_ |  _/| | (_) |
  \___|_| |___\___/ 
                    
*/
gpio #(
    .GpiWidth        (     GPIO_INPUT_SIZE ),
    .GpoWidth        (    GPIO_OUTPUT_SIZE )
) u_gpio (
    .clk_i           (           clk_sys_i ),
    .rst_ni          (          rst_sys_ni ),
    
    .device_req_i    (    device_req[Gpio] ),
    .device_addr_i   (   device_addr[Gpio] ),
    .device_we_i     (     device_we[Gpio] ),
    .device_be_i     (     device_be[Gpio] ),
    .device_wdata_i  (  device_wdata[Gpio] ),
    .device_rvalid_o ( device_rvalid[Gpio] ),
    .device_rdata_o  (  device_rdata[Gpio] ),
    
    .gp_i            (              gpio_i ),
    .gp_o            (              gpio_o )
);

/*
  _   _  _   ___ _____ 
 | | | |/_\ | _ \_   _|
 | |_| / _ \|   / | |  
  \___/_/ \_\_|_\ |_|  
                       
*/
uart #(
    .ClockFrequency  ( SOC_CLOCK_FREQUENCY ),
    .BaudRate        (      UART_BAUD_RATE )
) u_uart (
    .clk_i           (           clk_sys_i ),
    .rst_ni          (          rst_sys_ni ),
    
    .device_req_i    (    device_req[Uart] ),
    .device_addr_i   (   device_addr[Uart] ),
    .device_we_i     (     device_we[Uart] ),
    .device_be_i     (     device_be[Uart] ),
    .device_wdata_i  (  device_wdata[Uart] ),
    .device_rvalid_o ( device_rvalid[Uart] ),
    .device_rdata_o  (  device_rdata[Uart] ),
    
    .uart_rx_i       (           uart_rx_i ),
    .uart_irq_o      (            uart_irq ),
    .uart_tx_o       (           uart_tx_o )
);

/*
  ___ ___ ___ 
 / __| _ \_ _|
 \__ \  _/| | 
 |___/_| |___|
              
*/
spi_top #(
    .ClockFrequency  ( SOC_CLOCK_FREQUENCY ),
    .CPOL            (            SPI_CPOL ),
    .CPHA            (            SPI_CPHA )
) u_spi (
    .clk_i           (           clk_sys_i ),
    .rst_ni          (          rst_sys_ni ),

    .device_req_i    (     device_req[Spi] ),
    .device_addr_i   (    device_addr[Spi] ),
    .device_we_i     (      device_we[Spi] ),
    .device_be_i     (      device_be[Spi] ),
    .device_wdata_i  (   device_wdata[Spi] ),
    .device_rvalid_o (  device_rvalid[Spi] ),
    .device_rdata_o  (   device_rdata[Spi] ),

    .spi_rx_i        (            spi_rx_i ), // Data received from SPI device.
    .spi_tx_o        (            spi_tx_o ), // Data transmitted to SPI device.
    .sck_o           (           spi_sck_o ), // Serial clock pin.

    .byte_data_o() // Unused.
);

/*
  _____ ___ __  __ ___ ___ 
 |_   _|_ _|  \/  | __| _ \
   | |  | || |\/| | _||   /
   |_| |___|_|  |_|___|_|_\
                           
*/
timer #(
    .DataWidth      (                   32 ),
    .AddressWidth   (                   32 )
) u_timer (
    .clk_i          (            clk_sys_i ),
    .rst_ni         (           rst_sys_ni ),

    .timer_req_i    (    device_req[Timer] ),
    .timer_we_i     (     device_we[Timer] ),
    .timer_be_i     (     device_be[Timer] ),
    .timer_addr_i   (   device_addr[Timer] ),
    .timer_wdata_i  (  device_wdata[Timer] ),
    .timer_rvalid_o ( device_rvalid[Timer] ),
    .timer_rdata_o  (  device_rdata[Timer] ),
    .timer_err_o    (    device_err[Timer] ),
    .timer_intr_o   (            timer_irq )
);

/*
  ___ ___ __  __      ___ _____ ___ _    
 / __|_ _|  \/  |    / __|_   _| _ \ |   
 \__ \| || |\/| |_  | (__  | | |   / |__ 
 |___/___|_|  |_(_)  \___| |_| |_|_\____|
                                         
*/
`ifdef VERILATOR
  simulator_ctrl #(
      .LogName  (      "csoc_system.log" )
    ) u_simulator_ctrl (
      .clk_i    (               clk_sys_i),
      .rst_ni   (              rst_sys_ni),

      .req_i    (    device_req[SimCtrl] ),
      .we_i     (     device_we[SimCtrl] ),
      .be_i     (     device_be[SimCtrl] ),
      .addr_i   (   device_addr[SimCtrl] ),
      .wdata_i  (  device_wdata[SimCtrl] ),
      .rvalid_o ( device_rvalid[SimCtrl] ),
      .rdata_o  (  device_rdata[SimCtrl] )
    );
`endif

if (CORE_DEBUG == 1) begin : gen_dm_dmi

/*
  ___  __  __ ___ 
 |   \|  \/  |_ _|
 | |) | |\/| || | 
 |___/|_|  |_|___|
                  
*/
dmi_jtag u_dmi (
  .clk_i            (      clk_sys_i ),
  .rst_ni           (     rst_sys_ni ),
  .testmode_i       (             '0 ), // Tied to zero because Ibex do so and I don't know nothing more

  // active-low glitch free reset signal. Is asserted for one dmi clock cycle
  // (clk_i) whenever the dmi_jtag is reset (POR or functional reset).
  .dmi_rst_no       (      dmi_rst_n ),
  .dmi_req_o        (        dmi_req ),
  .dmi_req_valid_o  (  dmi_req_valid ),
  .dmi_req_ready_i  (  dmi_req_ready ),

  .dmi_resp_i       (       dmi_resp ),
  .dmi_resp_ready_o ( dmi_resp_ready ),
  .dmi_resp_valid_i ( dmi_resp_valid ),

  .tck_i            (      dtm_tck_i ), // JTAG test clock pad
  .tms_i            (      dtm_tms_i ), // JTAG test mode select pad
  .trst_ni          (    dtm_trst_ni ), // JTAG test reset pad
  .td_i             (       dtm_td_i ), // JTAG test data input pad
  .td_o             (       dtm_td_o ), // JTAG test data output pad
  .tdo_oe_o         (   dtm_tdo_oe_o )  // Data out output enable
);

/*
  ___  __  __ 
 |   \|  \/  |
 | |) | |\/| |
 |___/|_|  |_|
              
*/
dm_obi_top #(
  .NrHarts            (                     1 )
) dm_obi_top ( 
  .clk_i              (             clk_sys_i ),
  .rst_ni             (            rst_sys_ni ),
  .testmode_i         (                    '0 ), // Tied to zero because Ibex do so and I don't know nothing more
  .ndmreset_o         (          ndmreset_req ),
  .dmactive_o         (                       ), // Unused
  .debug_req_o        (        core_debug_req ),
  .unavailable_i      (                    '0 ), // Communicate whether the hart is unavailable (e.g.: power down)
  .hartinfo_i         (                    '0 ), // Tied to zero because I have no such informations

  // Device (slave) interface. I assume this is used when:
  // - DN is accessed to fetch an istruction by the core
  // - DM is accessed to fetch a data by the core
  .slave_req_i        (         dm_device_req ),
  .slave_gnt_o        (                       ), // Unused
  .slave_we_i         (          dm_device_we ),
  .slave_addr_i       (        dm_device_addr ),
  .slave_be_i         (     device_be[DbgDev] ),
  .slave_wdata_i      (  device_wdata[DbgDev] ),
  .slave_aid_i        (                       ), // Unused
  .slave_rvalid_o     ( device_rvalid[DbgDev] ),
  .slave_rdata_o      (  device_rdata[DbgDev] ),
  .slave_rid_o        (                       ), // Unused

  // Host interface to the bus. I assume this is used when:
  // - DM access memory regions or I/O regions
  .master_req_o       (     host_req[DbgHost] ),
  .master_addr_o      (    host_addr[DbgHost] ),
  .master_we_o        (      host_we[DbgHost] ),
  .master_wdata_o     (   host_wdata[DbgHost] ),
  .master_be_o        (      host_be[DbgHost] ),
  .master_gnt_i       (     host_gnt[DbgHost] ),
  .master_rvalid_i    (  host_rvalid[DbgHost] ),
  .master_err_i       (     host_err[DbgHost] ),
  .master_other_err_i (                    '0 ), // Tied to zero because it's not supported
  .master_rdata_i     (   host_rdata[DbgHost] ),

  // Connection to DTM - compatible to RocketChip Debug Module
  .dmi_rst_ni         (             dmi_rst_n ),
  .dmi_req_valid_i    (         dmi_req_valid ),
  .dmi_req_ready_o    (         dmi_req_ready ),
  .dmi_req_i          (               dmi_req ),

  .dmi_resp_valid_o   (        dmi_resp_valid ),
  .dmi_resp_ready_i   (        dmi_resp_ready ),
  .dmi_resp_o         (              dmi_resp )
);

end else begin : gen_no_dm_dmi
    assign core_debug_req = '0;
    assign ndmreset_req   = '0;
end

endmodule

