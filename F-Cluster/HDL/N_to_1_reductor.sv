
`include "para.sv"
module N_to_1_reductor#(
    parameter N = 6
)(
    input clk,
    input rst,
    input [FLIT_SIZE * N - 1 : 0] in,
    input [N - 1 : 0] in_valid,
    input out_avail,
    output reg [N - 1 : 0] in_avail,
    output [FLIT_SIZE - 1 : 0] out,
    output out_valid
);

    reg [FLIT_SIZE - 1 : 0] in_slot [N - 1 : 0];
    
    wire [N - 1 : 0] slot_is_head;

    reg [N - 1 : 0] slot_valid;

    reg [N - 1 : 0] selector;

    genvar i;

    generate 
        for(i = 0; i < N; i = i + 1) begin: head_detect
            assign slot_is_head[i] = (in_slot[i][FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) || (in_slot[i][FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT);
        end
    endgenerate

    assign out = in_slot[selector];

    assign out_valid = slot_valid[selector];

    always@(*) begin
        if(out_avail) 
            in_avail = (1 << selector) | (~ slot_valid);
        else 
            in_avail = (~slot_valid);
    end

    generate 
    for(i = 0; i < N; i = i + 1) begin: slot_write
        always@(posedge clk) begin
            if(rst) begin
                in_slot[i] <= 0;
                slot_valid[i] <= 0;
            end
            else begin
                if(in_avail[i]) begin
                    in_slot[i] <= in[FLIT_SIZE * i + FLIT_SIZE - 1: FLIT_SIZE * i];
                    slot_valid[i] <= in_valid[i];
                end
            end
        end
    end
    endgenerate


    logic [CMP_LEN - 1 : 0] max;
    integer index = 0;
	 
	 reg [N - 1 : 0] pre_sel;
	 

     reg occupy;

	 always@(posedge clk) begin
	     pre_sel <= selector;
	 end

    always@(posedge clk) begin
        if(out_valid && ((out[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) || (out[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == BODY_FLIT))) begin
            occupy <= 1;
        end
        else if(out_valid && out[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT && out_avail) begin
            occupy <= 0;
        end

    end
    

    always_comb begin
        max = 0;
        selector = 0; 
        if(occupy) begin
            selector = pre_sel;
        end
        else begin
            for(index = 0; index < N; index = index + 1) begin
                if(in_slot[index][CMP_POS : CMP_POS - CMP_LEN + 1] > max && slot_is_head[index] && slot_valid[index]) begin
                    selector = index;
                    max = in_slot[index][CMP_POS : CMP_POS - CMP_LEN + 1];
                end
            end
        end
    end

    
endmodule
