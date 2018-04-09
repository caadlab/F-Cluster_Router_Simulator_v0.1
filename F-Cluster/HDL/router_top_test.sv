`include "para.sv"
module router_top_test(
    input clk,
    input rst,
    input [FLIT_SIZE - 1 : 0] in,
    input in_valid,
    output [FLIT_SIZE - 1 : 0] out,
    output out_valid,

    output [FLIT_SIZE - 1 : 0] eject,
    output eject_valid,
    input [FLIT_SIZE - 1 : 0] inject,
    input inect_valid
);
    
	 
	 wire [FLIT_SIZE - 1 : 0] out_xpos;
	 wire [FLIT_SIZE - 1 : 0] out_ypos;
	 wire [FLIT_SIZE - 1 : 0] out_zpos;
	 wire [FLIT_SIZE - 1 : 0] out_xneg;
	 wire [FLIT_SIZE - 1 : 0] out_yneg;
	 wire [FLIT_SIZE - 1 : 0] out_zneg;
	 
	 wire out_xpos_valid;
	 wire out_ypos_valid;
	 wire out_zpos_valid;
	 wire out_xneg_valid;
	 wire out_yneg_valid;
	 wire out_zneg_valid;
	 
	 wire [FLIT_SIZE - 1 : 0] eject_xpos;
	 wire [FLIT_SIZE - 1 : 0] eject_ypos;
	 wire [FLIT_SIZE - 1 : 0] eject_zpos;
	 wire [FLIT_SIZE - 1 : 0] eject_xneg;
	 wire [FLIT_SIZE - 1 : 0] eject_yneg;
	 wire [FLIT_SIZE - 1 : 0] eject_zneg;
	 
	 wire eject_xpos_valid;
	 wire eject_ypos_valid;
	 wire eject_zpos_valid;
	 wire eject_xneg_valid;
	 wire eject_yneg_valid;
	 wire eject_zneg_valid;
	 
    router#(
        .cur_x(0),
        .cur_y(0),
        .cur_z(0),
        .input_Q_size(256),
        .credit_back_period(100),
        .credit_threshold(160)
    )router_inst(
        .clk(clk),
        .rst(rst),
        .in_xpos(in),
        .in_ypos(in),
        .in_zpos(in),
        .in_xneg(in),
        .in_yneg(in),
        .in_zneg(in),
        .in_xpos_valid(in_valid),
        .in_ypos_valid(in_valid),
        .in_zpos_valid(in_valid),
        .in_xneg_valid(in_valid),
        .in_yneg_valid(in_valid),
        .in_zneg_valid(in_valid),
        .out_xpos(out_xpos),
        .out_ypos(out_ypos),
        .out_zpos(out_zpos),
        .out_xneg(out_xneg),
        .out_yneg(out_yneg),
        .out_zneg(out_zneg),
        .out_xpos_valid(out_xpos_valid),
        .out_ypos_valid(out_ypos_valid),
        .out_zpos_valid(out_zpos_valid),
        .out_xneg_valid(out_xneg_valid),
        .out_yneg_valid(out_yneg_valid),
        .out_zneg_valid(out_zneg_valid),
        
        .eject_xpos(eject_xpos),
        .eject_ypos(eject_ypos),
        .eject_zpos(eject_zpos),
        .eject_xneg(eject_xneg),
        .eject_yneg(eject_yneg),
        .eject_zneg(eject_zneg),
        .eject_xpos_valid(eject_xpos_valid),
        .eject_ypos_valid(eject_ypos_valid),
        .eject_zpos_valid(eject_zpos_valid),
        .eject_xneg_valid(eject_xneg_valid),
        .eject_yneg_valid(eject_yneg_valid),
        .eject_zneg_valid(eject_zneg_valid),
        
        .inject_xpos(inject),
        .inject_ypos(inject),
        .inject_zpos(inject),
        .inject_xneg(inject),
        .inject_yneg(inject),
        .inject_zneg(inject),

        .inject_xpos_valid(inject_valid),
        .inject_ypos_valid(inject_valid),
        .inject_zpos_valid(inject_valid),
        .inject_xneg_valid(inject_valid),
        .inject_yneg_valid(inject_valid),
        .inject_zneg_valid(inject_valid)
    );
	 
	 assign out = out_xpos & out_ypos & out_zpos & out_xneg & out_yneg & out_zneg;
	 
	 assign out_valid = out_xpos_valid & out_ypos_valid & out_zpos_valid & out_xneg_valid & out_yneg_valid & out_zneg_valid;
	 
	 assign eject = eject_xpos & eject_ypos & eject_zpos & eject_xneg & eject_yneg & eject_zneg;

	 assign eject_valid = eject_xpos_valid & eject_ypos_valid & eject_zpos_valid & eject_xneg_valid & eject_yneg_valid & eject_zneg_valid;
endmodule
