`include "para.sv"


module reduction_tree#(
    parameter FAN_IN = 54,
    parameter L1_N = 18,
    parameter L2_N = 6,
	parameter L3_N = 2,
    parameter L4_N = 1 
)
(
    input clk,
    input rst,
    input [FLIT_SIZE * FAN_IN  - 1 : 0] in,
    input [FAN_IN - 1 : 0] in_valid,
    input out_avail,
    output reg [FAN_IN - 1 : 0] in_avail,
    output [FLIT_SIZE - 1 : 0] out,
    output out_valid
);
    parameter L1_W = FAN_IN / L1_N;
    parameter L2_W = L1_N / L2_N;
    parameter L3_W = L2_N / L3_N;
    parameter L4_W = L3_N / L4_N;


    wire [L1_N - 1 : 0] out_avail_L2_to_L1;
    wire [L1_N - 1 : 0] out_valid_L1_to_L2;
    wire [FLIT_SIZE - 1 : 0] out_L1_to_L2[L1_N - 1 : 0];
//we need four levels of reductors
    //instantiate first level
    genvar i;

    generate
        for(i = 0; i < L1_N; i = i + 1) begin: L1_reductors
            N_to_1_reductor_optimized#(
                .N(L1_W)
            )level1(
                .clk(clk),
                .rst(rst),
                .in(in[i * L1_W * FLIT_SIZE + L1_W * FLIT_SIZE - 1 : i * L1_W * FLIT_SIZE]),
                .in_valid(in_valid[i * L1_W + L1_W - 1 : i * L1_W]),
                .out_avail(out_avail_L2_to_L1[i]),
                .in_avail(in_avail[i * L1_W + L1_W - 1 : i * L1_W]),
                .out(out_L1_to_L2[i]),
                .out_valid(out_valid_L1_to_L2[i])
            );
        end
    endgenerate
	 
	wire [FLIT_SIZE * L1_N - 1 : 0] out_L1_to_L2_packed;
	generate 
	    for(i = 0; i < L1_N; i = i + 1) begin: pack_out_l1_to_l2
		    assign out_L1_to_L2_packed[i * FLIT_SIZE + FLIT_SIZE - 1 : i* FLIT_SIZE] = out_L1_to_L2[i];	  
		end
	endgenerate
	 
	wire [L2_N - 1 : 0] out_avail_L3_to_L2;
    wire [L2_N - 1 : 0] out_valid_L2_to_L3;
    wire [FLIT_SIZE - 1 : 0] out_L2_to_L3[L2_N - 1 : 0];
	 


    //instantiate second level
	 
    generate
        for(i = 0; i < L2_N; i = i + 1) begin: L2_reductors
            N_to_1_reductor_optimized#(
                .N(L2_W)
            )L2_reductor(
                .clk(clk),
                .rst(rst),
                .in(out_L1_to_L2_packed[i * L2_W * FLIT_SIZE + L2_W * FLIT_SIZE - 1 : i * L2_W * FLIT_SIZE]),
                .in_valid(out_valid_L1_to_L2[i * L2_W + L2_W - 1 : i * L2_W]),
                .out_avail(out_avail_L3_to_L2[i]),
                .in_avail(out_avail_L2_to_L1[i * L2_W + L2_W - 1 : i * L2_W]),
                .out(out_L2_to_L3[i]),
                .out_valid(out_valid_L2_to_L3[i])
            );
        end
    endgenerate
    
    wire [FLIT_SIZE * L2_N - 1 : 0] out_L2_to_L3_packed;
	generate 
	    for(i = 0; i < L2_N; i = i + 1) begin: pack_out_l2_to_l3
		    assign out_L2_to_L3_packed[i * FLIT_SIZE + FLIT_SIZE - 1 : i* FLIT_SIZE] = out_L2_to_L3[i];	  
		end
	endgenerate
    
    wire [L3_N - 1 : 0] out_avail_L4_to_L3;
    wire [L3_N - 1 : 0] out_valid_L3_to_L4;
    wire [FLIT_SIZE - 1 : 0] out_L3_to_L4[L3_N - 1 : 0];
           
	//instantiate third level
    
    
    generate
        for(i = 0; i < L3_N; i = i + 1) begin: L3_reductors
            N_to_1_reductor_optimized#(
                .N(L3_W)
            )L3_reductor(
                .clk(clk),
                .rst(rst),
                .in(out_L2_to_L3_packed[i * L3_W * FLIT_SIZE + L3_W * FLIT_SIZE - 1 : i * L3_W * FLIT_SIZE]),
                .in_valid(out_valid_L2_to_L3[i * L3_W + L3_W - 1 : i * L3_W]),
                .out_avail(out_avail_L4_to_L3[i]),
                .in_avail(out_avail_L3_to_L2[i * L3_W + L3_W - 1 : i * L3_W]),
                .out(out_L3_to_L4[i]),
                .out_valid(out_valid_L3_to_L4[i])
            );
        end
    endgenerate
    
    wire [FLIT_SIZE * L3_N - 1 : 0] out_L3_to_L4_packed;
	generate 
	    for(i = 0; i < L3_N; i = i + 1) begin: pack_out_l3_to_l4
		    assign out_L3_to_L4_packed[i * FLIT_SIZE + FLIT_SIZE - 1 : i* FLIT_SIZE] = out_L3_to_L4[i];	  
		end
	endgenerate
            
		
    N_to_1_reductor_optimized#(
        .N(L4_W)
    )L4_reductor(
        .clk(clk),
        .rst(rst),
        .in(out_L3_to_L4_packed),
        .in_valid(out_valid_L3_to_L4),
        .out_avail(out_avail),
        .in_avail(out_avail_L4_to_L3),
        .out(out),
        .out_valid(out_valid)
    );


endmodule
