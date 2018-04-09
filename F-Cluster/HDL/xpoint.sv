`include "para.sv"
module xpoint(
    input [FLIT_SIZE - 1 : 0] h_in,
    input [FLIT_SIZE - 1 : 0] v_in,
    input cross_enable,
    output reg [FLIT_SIZE - 1 : 0] h_out,
    output reg [FLIT_SIZE - 1 : 0] v_out
);

    always@(*) begin
        if(cross_enable) begin
            h_out = v_in;
            v_out = h_in;
        end
        else begin
            h_out = h_in;
            v_out = v_in;
        end
    end

endmodule
    
