`include "para.sv"
module switch#(
    parameter M_IN = VC_NUM * PORT_NUM,
    parameter N_OUT= PORT_NUM
)
(
    input clk,
    input rst,
    input [M_IN * FLIT_SIZE - 1 : 0] in,
    input [M_IN * ROUTE_LEN - 1 : 0] route_in,
    input [M_IN - 1 : 0] in_valid,
	input [N_OUT - 1 : 0] out_avail,
	
    output [M_IN - 1 : 0] in_avail,
    output [N_OUT - 1 : 0] out_valid,
    output [N_OUT * FLIT_SIZE - 1 : 0] out    
);

    wire [M_IN - 1 : 0] xpos_in_valid;
    wire [M_IN - 1 : 0] ypos_in_valid;
    wire [M_IN - 1 : 0] zpos_in_valid;
    wire [M_IN - 1 : 0] xneg_in_valid;
    wire [M_IN - 1 : 0] yneg_in_valid;
    wire [M_IN - 1 : 0] zneg_in_valid;

    wire [M_IN - 1 : 0] xpos_in_avail;
    wire [M_IN - 1 : 0] ypos_in_avail;
    wire [M_IN - 1 : 0] zpos_in_avail;
    wire [M_IN - 1 : 0] xneg_in_avail;
    wire [M_IN - 1 : 0] yneg_in_avail;
    wire [M_IN - 1 : 0] zneg_in_avail;

    genvar i;
    generate 
        for(i = 0; i < M_IN; i = i + 1) begin: dst_checker
            assign xpos_in_valid[i] = route_in[i * ROUTE_LEN + ROUTE_LEN - 1: i * ROUTE_LEN] == DIR_XPOS;
            assign ypos_in_valid[i] = route_in[i * ROUTE_LEN + ROUTE_LEN - 1: i * ROUTE_LEN] == DIR_YPOS;
            assign zpos_in_valid[i] = route_in[i * ROUTE_LEN + ROUTE_LEN - 1: i * ROUTE_LEN] == DIR_ZPOS;
            assign xneg_in_valid[i] = route_in[i * ROUTE_LEN + ROUTE_LEN - 1: i * ROUTE_LEN] == DIR_XNEG;
            assign yneg_in_valid[i] = route_in[i * ROUTE_LEN + ROUTE_LEN - 1: i * ROUTE_LEN] == DIR_YNEG;
            assign zneg_in_valid[i] = route_in[i * ROUTE_LEN + ROUTE_LEN - 1: i * ROUTE_LEN] == DIR_ZNEG;
        end
    endgenerate

	// Generate the output signal to let the upper level know if the input for switch is available
	// Only set a certain input slot as available if the related input for the selected reduction tree is available 
    generate
        for(i = 0; i < M_IN; i = i + 1) begin: in_avail_gen
            assign in_avail[i] = ({zneg_in_avail[i], yneg_in_avail[i], xneg_in_avail[i], zpos_in_avail[i], ypos_in_avail[i], xpos_in_avail[i]} & (1 << ( route_in[i * ROUTE_LEN + ROUTE_LEN - 1: i * ROUTE_LEN] - 1))) != 0;
        end
    endgenerate


    reduction_tree#(
        .FAN_IN(M_IN)
    )xpos_reduction(
        .clk(clk),
        .rst(rst),
        .in(in),
        .in_valid(in_valid & xpos_in_valid),
        .in_avail(xpos_in_avail),
        .out_valid(out_valid[0]),
        .out_avail(out_avail[0]),
        .out(out[FLIT_SIZE - 1 : 0])
    );
     
    reduction_tree#(
        .FAN_IN(M_IN)
    )ypos_reduction(
        .clk(clk),
        .rst(rst),
        .in(in),
        .in_valid(in_valid & ypos_in_valid),
        .in_avail(ypos_in_avail),
        .out_valid(out_valid[1]),
        .out_avail(out_avail[1]),
        .out(out[FLIT_SIZE * 2 - 1 : FLIT_SIZE])
    );
      
    reduction_tree#(
        .FAN_IN(M_IN)
    )zpos_reduction(
        .clk(clk),
        .rst(rst),
        .in(in),
        .in_valid(in_valid & zpos_in_valid),
        .in_avail(zpos_in_avail),
        .out_valid(out_valid[2]),
        .out_avail(out_avail[2]),
        .out(out[FLIT_SIZE * 3 - 1 : FLIT_SIZE * 2])
    ); 

    reduction_tree#(
        .FAN_IN(M_IN)
    )xneg_reduction(
        .clk(clk),
        .rst(rst),
        .in(in),
        .in_valid(in_valid & xneg_in_valid),
        .in_avail(xneg_in_avail),
        .out_valid(out_valid[3]),
        .out_avail(out_avail[3]),
        .out(out[FLIT_SIZE * 4 - 1 : FLIT_SIZE * 3])
    ); 

    reduction_tree#(
        .FAN_IN(M_IN)
    )yneg_reduction(
        .clk(clk),
        .rst(rst),
        .in(in),
        .in_valid(in_valid & yneg_in_valid),
        .in_avail(yneg_in_avail),
        .out_valid(out_valid[4]),
        .out_avail(out_avail[4]),
        .out(out[FLIT_SIZE * 5 - 1 : FLIT_SIZE * 4])
    ); 

    reduction_tree#(
        .FAN_IN(M_IN)
    )zneg_reduction(
        .clk(clk),
        .rst(rst),
        .in(in),
        .in_valid(in_valid & zneg_in_valid),
        .in_avail(zneg_in_avail),
        .out_valid(out_valid[5]),
        .out_avail(out_avail[5]),
        .out(out[FLIT_SIZE * 6 - 1 : FLIT_SIZE * 5])
    );
   
   
   
endmodule
    
