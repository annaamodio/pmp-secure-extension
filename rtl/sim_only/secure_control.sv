
/**
 * MOCK; To be removed
 * Module to change security level from outside.
 * 
 * contains just one flip flop. When you write 1, security_lvl is set to SEC_LVL_S
 * otherwise is SEC_LVL_S
 *
 */

module secure_control (
  input               clk_i,
  input               rst_ni,

  input               req_i,
  input               we_i,
  input        [ 3:0] be_i,
  input        [31:0] addr_i,
  input               wdata_i,
  output logic        rvalid_o,
  output logic        rdata_o
);

  logic security_level;

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (~rst_ni) begin
      rvalid_o <= 0;
        end else begin
        // Immeditely respond to any request
        rvalid_o <= req_i;
        if (req_i & we_i) begin
            if (be_i[0]) begin
            security_level <= wdata_i;
                end
            end
        end
    end
  assign rdata_o = security_level;

endmodule
