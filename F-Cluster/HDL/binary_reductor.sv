`include "para.sv"
`define FARTHEST_FIRST
module binary_reductor(
    input clk,
    input rst,
    input [FLIT_SIZE - 1 : 0] in0,
    input [FLIT_SIZE - 1 : 0] in1,
    input in0_valid,
    input in1_valid,
    input out_avail,
    output reg in0_avail,
    output reg in1_avail,
    output [FLIT_SIZE - 1 : 0] out,
    output out_valid
);

    reg [FLIT_SIZE - 1 : 0] in_slot0;
    reg [FLIT_SIZE - 1 : 0] in_slot1;

    wire slot0_is_header;
    wire slot1_is_header;

    reg slot0_valid;
    reg slot1_valid;

    reg selector;

    reg pre_sel;

    assign slot0_is_header = (in_slot0[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) || (in_slot0[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT); 

    assign slot1_is_header = (in_slot1[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) || (in_slot1[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT); 

    assign out = selector ? in1 : in0;

    assign out_valid = selector ? slot1_valid : slot0_valid;

    always@(*) begin
        if(out_avail && selector) begin
            in1_avail = 1;
        end
        else begin
            in1_avail = ~slot1_valid;
        end
    end

    always@(*) begin
        if(out_avail && (~selector))begin
            in0_avail = 1;
        end
        else begin
            in0_avail = ~slot0_valid;
        end
    end

    always@(posedge clk) begin
        if(rst) begin
            in_slot0 <= 0;
            in_slot1 <= 0;
            slot0_valid <= 0;
            slot1_valid <= 1;
        end
        else begin
            if(in0_avail) begin
                in_slot0 <= in0;
                slot0_valid <= in0_valid;
            end
            if(in1_avail) begin
                in_slot1 <= in1;
                slot1_valid <= in1_valid;
            end
        end
    end
            

    

    //selector
    //
    always@(posedge clk) begin
        pre_sel <= selector;
    end


    always@(*) begin
        if(slot0_valid && ~slot1_valid) begin
            selector = 0;
        end
        else if(~slot0_valid && slot1_valid) begin
            selector = 1;
        end
        else if(~slot0_valid && ~slot1_valid) begin
            selector = 0;
        end
        else begin
            if(slot0_is_header && slot1_is_header) begin
            `ifdef FARTHEST
                selector = in_slot0[CMP_POS : CMP_POS - CMP_LEN + 1] < in_slot1[CMP_POS : CMP_POS - CMP_LEN + 1];
            `endif
            end
            else begin
                selector = pre_sel;
            end
        end
    end
            
        
endmodule
