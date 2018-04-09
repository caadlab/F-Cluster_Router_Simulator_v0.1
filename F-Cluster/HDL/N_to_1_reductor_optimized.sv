
`include "para.sv"
module N_to_1_reductor_optimized#(
    parameter N = 3
)(
    input clk,
    input rst,
    input [FLIT_SIZE * N - 1 : 0] in,
    input [N - 1 : 0] in_valid,
    input out_avail,
    output [N - 1 : 0] in_avail,
    output [FLIT_SIZE - 1 : 0] out,
    output out_valid
);

    reg [N - 1 : 0] selector;
    wire [N - 1 : 0] selector_one_hot;
    wire [N - 1 : 0] consume;
    wire [N - 1 : 0] q_empty;
    wire [N - 1 : 0] q_full;

    reg [FLIT_SIZE - 1 : 0] in_slot [N - 1 : 0];
    
    wire [N - 1 : 0] slot_is_head;

    reg [N - 1 : 0] slot_valid;


    assign slot_valid = ~q_empty;

    assign in_avail = ~q_full;

    //instantiate N tiny fifos
    genvar i;
    generate 
        for(i = 0; i < N; i = i + 1) begin: in_Q
            buffer#(
                .buffer_depth_log(1),
                .buffer_depth(2),
                .buffer_width(FLIT_SIZE)
            )in_Q_inst(
                .clk(clk),
                .rst(rst),
                .in(in[FLIT_SIZE * i + FLIT_SIZE - 1 : FLIT_SIZE * i]),
                .produce(in_valid[i]),
                .consume(out_avail && selector_one_hot[i]),				// Only when this input select and the output is available, the read out the data and send to output port
                .full(q_full[i]),
                .empty(q_empty[i]),
                .out(in_slot[i]),
                .usedw()
            );
        end
    endgenerate

	// Set only the slected port as 1, for example, if selector == 3, then the mask will be 3'b100
    assign selector_one_hot = (1 << selector);


	// determine if the current flit is HEADER_FLIT or SINGLE_FLIT
    generate 
        for(i = 0; i < N; i = i + 1) begin: head_detect
            assign slot_is_head[i] = (in_slot[i][FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) || (in_slot[i][FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT);
        end
    endgenerate

    assign out = in_slot[selector];

    assign out_valid = slot_valid[selector];
	
	// fetch the stored manhatton distance as the value to determine priority
    wire [CMP_LEN - 1 : 0] out_cmp_array[N - 1 : 0];
    generate 
        for(i = 0; i < N; i = i + 1) begin: cmp_array
            assign out_cmp_array[i] = in_slot[i][CMP_POS : CMP_POS - CMP_LEN + 1];
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


    generate
        case(N)
            1: 
            always_comb begin
                max = 0;
                selector = 0; 
            end
            2:
            always_comb begin
                if(occupy) begin
                    selector = pre_sel;
                end
                else begin
                    if(out_cmp_array[1] >= out_cmp_array[0] && slot_is_head[1] && slot_valid[1])
                        selector = 1;
                    else
                        selector = 0;
                end
            end
            3:
            always_comb begin
                if(occupy) begin
                    selector = pre_sel;
                end
                else begin
                    if(out_cmp_array[2] >= out_cmp_array[0] && out_cmp_array[2] >= out_cmp_array[1] && slot_is_head[2] && slot_valid[2])
                        selector = 2;
                    else if(out_cmp_array[1] >= out_cmp_array[0] && slot_is_head[1] && slot_valid[1])
                        selector = 1;
                    else
                        selector = 0;
                end
            end
            default:
            always_comb begin
                if(occupy) begin
                    selector = pre_sel;
                end
                else begin
                    max = 0;
                    selector = 0; 
                    for(index = 0; index <= N - 1; index = index + 1) begin
                        if(out_cmp_array[index] > max && slot_is_head[index] && slot_valid[index]) begin
                            selector = index;
                            max = out_cmp_array[index] ;
                        end
                        else begin
                            selector = index;
                            max = max;
                        end
                    end
                end
            end
            
        
        
        endcase
        
    endgenerate
    

 /*   always_comb begin

        if(occupy) begin
            selector = pre_sel;
        end
        else begin
            generate
                case(N)
                    1: selector = 0;
                    2: begin
                        if(out_cmp_array[1] >= out_cmp_array[0] && slot_is_head[1] && slot_valid[1])
                            selector = 1;
                        else
                            selector = 0;
                    end
                    3: begin
                        if(out_cmp_array[2] >= out_cmp_array[0] && out_cmp_array[1] >= out_cmp_array[0] && slot_is_head[2] && slot_valid[2])
                            selector = 2;
                        else if(out_cmp_array[1] >= out_cmp_array[0] && slot_is_head[1] && slot_valid[1])
                            selector = 1;
                        else
                            selector = 0;
                    end
                    default: begin
                        max = 0;
                        selector = 0; 
                        for(index = 0; index <= N - 1; index = index + 1) begin
                            if(out_cmp_array[index] > max && slot_is_head[index] && slot_valid[index]) begin
                                selector = index;
                                max = out_cmp_array[index] ;
                            end
                            else begin
                                selector = index;
                                max = max;
                            end
                        end
                    end
                endcase
            endgenerate
        end
    end*/

    
endmodule
