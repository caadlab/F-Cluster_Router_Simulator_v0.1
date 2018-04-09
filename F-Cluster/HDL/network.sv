`include "para.sv"
    module network(
    input clk,
    input rst
);
	reg[15:0] xpos_link_sum, xneg_link_sum, ypos_link_sum, yneg_link_sum, zpos_link_sum, zneg_link_sum;
	reg[15:0] xpos_ClockwiseUtil_sum, xneg_ClockwiseUtil_sum, ypos_ClockwiseUtil_sum, yneg_ClockwiseUtil_sum, zpos_ClockwiseUtil_sum, zneg_ClockwiseUtil_sum;
	reg[15:0] xpos_CounterClockwiseUtil_sum, xneg_CounterClockwiseUtil_sum, ypos_CounterClockwiseUtil_sum, yneg_CounterClockwiseUtil_sum, zpos_CounterClockwiseUtil_sum, zneg_CounterClockwiseUtil_sum;
	reg[15:0] xpos_InjectUtil_sum, xneg_InjectUtil_sum, ypos_InjectUtil_sum, yneg_InjectUtil_sum, zpos_InjectUtil_sum, zneg_InjectUtil_sum;
	reg[15:0] link_sum,Clockwise_sum,CounterClockwise_sum,Inject_sum,port_sum;
	reg[15:0] counter, time_counter;
	wire[FLIT_SIZE : 0] in_xpos_ser_0_0_0, out_xpos_ser_0_0_0;
	wire[FLIT_SIZE : 0] in_xneg_ser_0_0_0, out_xneg_ser_0_0_0;
	wire[FLIT_SIZE : 0] in_ypos_ser_0_0_0, out_ypos_ser_0_0_0;
	wire[FLIT_SIZE : 0] in_yneg_ser_0_0_0, out_yneg_ser_0_0_0;
	wire[FLIT_SIZE : 0] in_zpos_ser_0_0_0, out_zpos_ser_0_0_0;
	wire[FLIT_SIZE : 0] in_zneg_ser_0_0_0, out_zneg_ser_0_0_0;
	wire[7:0] xpos_ClockwiseUtil_0_0_0,xpos_CounterClockwiseUtil_0_0_0,xpos_InjectUtil_0_0_0;
	wire[7:0] xneg_ClockwiseUtil_0_0_0,xneg_CounterClockwiseUtil_0_0_0,xneg_InjectUtil_0_0_0;
	wire[7:0] ypos_ClockwiseUtil_0_0_0,ypos_CounterClockwiseUtil_0_0_0,ypos_InjectUtil_0_0_0;
	wire[7:0] yneg_ClockwiseUtil_0_0_0,yneg_CounterClockwiseUtil_0_0_0,yneg_InjectUtil_0_0_0;
	wire[7:0] zpos_ClockwiseUtil_0_0_0,zpos_CounterClockwiseUtil_0_0_0,zpos_InjectUtil_0_0_0;
	wire[7:0] zneg_ClockwiseUtil_0_0_0,zneg_CounterClockwiseUtil_0_0_0,zneg_InjectUtil_0_0_0;

	wire[FLIT_SIZE : 0] in_xpos_ser_0_1_0, out_xpos_ser_0_1_0;
	wire[FLIT_SIZE : 0] in_xneg_ser_0_1_0, out_xneg_ser_0_1_0;
	wire[FLIT_SIZE : 0] in_ypos_ser_0_1_0, out_ypos_ser_0_1_0;
	wire[FLIT_SIZE : 0] in_yneg_ser_0_1_0, out_yneg_ser_0_1_0;
	wire[FLIT_SIZE : 0] in_zpos_ser_0_1_0, out_zpos_ser_0_1_0;
	wire[FLIT_SIZE : 0] in_zneg_ser_0_1_0, out_zneg_ser_0_1_0;
	wire[7:0] xpos_ClockwiseUtil_0_1_0,xpos_CounterClockwiseUtil_0_1_0,xpos_InjectUtil_0_1_0;
	wire[7:0] xneg_ClockwiseUtil_0_1_0,xneg_CounterClockwiseUtil_0_1_0,xneg_InjectUtil_0_1_0;
	wire[7:0] ypos_ClockwiseUtil_0_1_0,ypos_CounterClockwiseUtil_0_1_0,ypos_InjectUtil_0_1_0;
	wire[7:0] yneg_ClockwiseUtil_0_1_0,yneg_CounterClockwiseUtil_0_1_0,yneg_InjectUtil_0_1_0;
	wire[7:0] zpos_ClockwiseUtil_0_1_0,zpos_CounterClockwiseUtil_0_1_0,zpos_InjectUtil_0_1_0;
	wire[7:0] zneg_ClockwiseUtil_0_1_0,zneg_CounterClockwiseUtil_0_1_0,zneg_InjectUtil_0_1_0;

	wire[FLIT_SIZE : 0] in_xpos_ser_1_0_0, out_xpos_ser_1_0_0;
	wire[FLIT_SIZE : 0] in_xneg_ser_1_0_0, out_xneg_ser_1_0_0;
	wire[FLIT_SIZE : 0] in_ypos_ser_1_0_0, out_ypos_ser_1_0_0;
	wire[FLIT_SIZE : 0] in_yneg_ser_1_0_0, out_yneg_ser_1_0_0;
	wire[FLIT_SIZE : 0] in_zpos_ser_1_0_0, out_zpos_ser_1_0_0;
	wire[FLIT_SIZE : 0] in_zneg_ser_1_0_0, out_zneg_ser_1_0_0;
	wire[7:0] xpos_ClockwiseUtil_1_0_0,xpos_CounterClockwiseUtil_1_0_0,xpos_InjectUtil_1_0_0;
	wire[7:0] xneg_ClockwiseUtil_1_0_0,xneg_CounterClockwiseUtil_1_0_0,xneg_InjectUtil_1_0_0;
	wire[7:0] ypos_ClockwiseUtil_1_0_0,ypos_CounterClockwiseUtil_1_0_0,ypos_InjectUtil_1_0_0;
	wire[7:0] yneg_ClockwiseUtil_1_0_0,yneg_CounterClockwiseUtil_1_0_0,yneg_InjectUtil_1_0_0;
	wire[7:0] zpos_ClockwiseUtil_1_0_0,zpos_CounterClockwiseUtil_1_0_0,zpos_InjectUtil_1_0_0;
	wire[7:0] zneg_ClockwiseUtil_1_0_0,zneg_CounterClockwiseUtil_1_0_0,zneg_InjectUtil_1_0_0;

	wire[FLIT_SIZE : 0] in_xpos_ser_1_1_0, out_xpos_ser_1_1_0;
	wire[FLIT_SIZE : 0] in_xneg_ser_1_1_0, out_xneg_ser_1_1_0;
	wire[FLIT_SIZE : 0] in_ypos_ser_1_1_0, out_ypos_ser_1_1_0;
	wire[FLIT_SIZE : 0] in_yneg_ser_1_1_0, out_yneg_ser_1_1_0;
	wire[FLIT_SIZE : 0] in_zpos_ser_1_1_0, out_zpos_ser_1_1_0;
	wire[FLIT_SIZE : 0] in_zneg_ser_1_1_0, out_zneg_ser_1_1_0;
	wire[7:0] xpos_ClockwiseUtil_1_1_0,xpos_CounterClockwiseUtil_1_1_0,xpos_InjectUtil_1_1_0;
	wire[7:0] xneg_ClockwiseUtil_1_1_0,xneg_CounterClockwiseUtil_1_1_0,xneg_InjectUtil_1_1_0;
	wire[7:0] ypos_ClockwiseUtil_1_1_0,ypos_CounterClockwiseUtil_1_1_0,ypos_InjectUtil_1_1_0;
	wire[7:0] yneg_ClockwiseUtil_1_1_0,yneg_CounterClockwiseUtil_1_1_0,yneg_InjectUtil_1_1_0;
	wire[7:0] zpos_ClockwiseUtil_1_1_0,zpos_CounterClockwiseUtil_1_1_0,zpos_InjectUtil_1_1_0;
	wire[7:0] zneg_ClockwiseUtil_1_1_0,zneg_CounterClockwiseUtil_1_1_0,zneg_InjectUtil_1_1_0;

	assign in_xpos_ser_0_0_0=out_xneg_ser_1_0_0;
	assign in_xneg_ser_0_0_0=out_xpos_ser_1_0_0;
	assign in_xpos_ser_0_1_0=out_xneg_ser_1_1_0;
	assign in_xneg_ser_0_1_0=out_xpos_ser_1_1_0;
	assign in_xpos_ser_1_0_0=out_xneg_ser_0_0_0;
	assign in_xneg_ser_1_0_0=out_xpos_ser_0_0_0;
	assign in_xpos_ser_1_1_0=out_xneg_ser_0_1_0;
	assign in_xneg_ser_1_1_0=out_xpos_ser_0_1_0;
	assign in_ypos_ser_0_0_0=out_yneg_ser_0_1_0;
	assign in_yneg_ser_0_0_0=out_ypos_ser_0_1_0;
	assign in_ypos_ser_0_1_0=out_yneg_ser_0_0_0;
	assign in_yneg_ser_0_1_0=out_ypos_ser_0_0_0;
	assign in_ypos_ser_1_0_0=out_yneg_ser_1_1_0;
	assign in_yneg_ser_1_0_0=out_ypos_ser_1_1_0;
	assign in_ypos_ser_1_1_0=out_yneg_ser_1_0_0;
	assign in_yneg_ser_1_1_0=out_ypos_ser_1_0_0;
	assign in_zpos_ser_0_0_0=out_zneg_ser_0_0_0;
	assign in_zneg_ser_0_0_0=out_zpos_ser_0_0_0;
	assign in_zpos_ser_0_1_0=out_zneg_ser_0_1_0;
	assign in_zneg_ser_0_1_0=out_zpos_ser_0_1_0;
	assign in_zpos_ser_1_0_0=out_zneg_ser_1_0_0;
	assign in_zneg_ser_1_0_0=out_zpos_ser_1_0_0;
	assign in_zpos_ser_1_1_0=out_zneg_ser_1_1_0;
	assign in_zneg_ser_1_1_0=out_zpos_ser_1_1_0;
    node#(
        .cur_x(3'd0),
        .cur_y(3'd0),
        .cur_z(3'd0)
        )n_0_0_0(
        .clk(clk),
        .rst(rst),
        .in_xpos_ser(in_xpos_ser_0_0_0),
        .out_xpos_ser(out_xpos_ser_0_0_0),
        .in_xneg_ser(in_xneg_ser_0_0_0),
        .out_xneg_ser(out_xneg_ser_0_0_0),
        .in_ypos_ser(in_ypos_ser_0_0_0),
        .out_ypos_ser(out_ypos_ser_0_0_0),
        .in_yneg_ser(in_yneg_ser_0_0_0),
        .out_yneg_ser(out_yneg_ser_0_0_0),
        .in_zpos_ser(in_zpos_ser_0_0_0),
        .out_zpos_ser(out_zpos_ser_0_0_0),
        .in_zneg_ser(in_zneg_ser_0_0_0),
        .out_zneg_ser(out_zneg_ser_0_0_0)
      );
    node#(
        .cur_x(3'd0),
        .cur_y(3'd1),
        .cur_z(3'd0)
        )n_0_1_0(
        .clk(clk),
        .rst(rst),
        .in_xpos_ser(in_xpos_ser_0_1_0),
        .out_xpos_ser(out_xpos_ser_0_1_0),
        .in_xneg_ser(in_xneg_ser_0_1_0),
        .out_xneg_ser(out_xneg_ser_0_1_0),
        .in_ypos_ser(in_ypos_ser_0_1_0),
        .out_ypos_ser(out_ypos_ser_0_1_0),
        .in_yneg_ser(in_yneg_ser_0_1_0),
        .out_yneg_ser(out_yneg_ser_0_1_0),
        .in_zpos_ser(in_zpos_ser_0_1_0),
        .out_zpos_ser(out_zpos_ser_0_1_0),
        .in_zneg_ser(in_zneg_ser_0_1_0),
        .out_zneg_ser(out_zneg_ser_0_1_0)
      );
    node#(
        .cur_x(3'd1),
        .cur_y(3'd0),
        .cur_z(3'd0)
        )n_1_0_0(
        .clk(clk),
        .rst(rst),
        .in_xpos_ser(in_xpos_ser_1_0_0),
        .out_xpos_ser(out_xpos_ser_1_0_0),
        .in_xneg_ser(in_xneg_ser_1_0_0),
        .out_xneg_ser(out_xneg_ser_1_0_0),
        .in_ypos_ser(in_ypos_ser_1_0_0),
        .out_ypos_ser(out_ypos_ser_1_0_0),
        .in_yneg_ser(in_yneg_ser_1_0_0),
        .out_yneg_ser(out_yneg_ser_1_0_0),
        .in_zpos_ser(in_zpos_ser_1_0_0),
        .out_zpos_ser(out_zpos_ser_1_0_0),
        .in_zneg_ser(in_zneg_ser_1_0_0),
        .out_zneg_ser(out_zneg_ser_1_0_0)
      );
    node#(
        .cur_x(3'd1),
        .cur_y(3'd1),
        .cur_z(3'd0)
        )n_1_1_0(
        .clk(clk),
        .rst(rst),
        .in_xpos_ser(in_xpos_ser_1_1_0),
        .out_xpos_ser(out_xpos_ser_1_1_0),
        .in_xneg_ser(in_xneg_ser_1_1_0),
        .out_xneg_ser(out_xneg_ser_1_1_0),
        .in_ypos_ser(in_ypos_ser_1_1_0),
        .out_ypos_ser(out_ypos_ser_1_1_0),
        .in_yneg_ser(in_yneg_ser_1_1_0),
        .out_yneg_ser(out_yneg_ser_1_1_0),
        .in_zpos_ser(in_zpos_ser_1_1_0),
        .out_zpos_ser(out_zpos_ser_1_1_0),
        .in_zneg_ser(in_zneg_ser_1_1_0),
        .out_zneg_ser(out_zneg_ser_1_1_0)
      );
endmodule
