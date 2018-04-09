`include "para.sv"
module xbar#(
    parameter M_IN = VC_NUM * PORT_NUM, 
    parameter N_OUT = PORT_NUM
)(
    input [M_IN * FLIT_SIZE - 1 : 0] inputs,
    input [M_IN * N_OUT -1 : 0] xpoints_enable,
    output [N_OUT * FLIT_SIZE -1 : 0] outputs
);

    genvar i, j;

    wire [FLIT_SIZE - 1 : 0] h_in_array [M_IN * N_OUT - 1 : 0];
    wire [FLIT_SIZE - 1 : 0] v_in_array [M_IN * N_OUT - 1 : 0];
    wire [FLIT_SIZE - 1 : 0] h_out_array [M_IN * N_OUT - 1 : 0];
    wire [FLIT_SIZE - 1 : 0] v_out_array [M_IN * N_OUT - 1 : 0];




    generate
        for(i = 0; i < M_IN; i = i + 1) 
        begin: CONNECT_V0
            assign v_in_array[i * N_OUT] = inputs[(i + 1) * FLIT_SIZE - 1 : i * FLIT_SIZE];
        end
    endgenerate

    generate
        for(i = 0; i < N_OUT; i = i + 1)
        begin: CONNECT_H0
            assign outputs[(i + 1) * FLIT_SIZE - 1 : i * FLIT_SIZE] = h_out_array[i];
        end
    endgenerate


    generate
        for(i = 0; i < N_OUT; i = i + 1) 
        begin: CONNECT_HLAST
            assign h_in_array[(M_IN - 1) * N_OUT + i] = 0;
        end
    endgenerate



    generate
        for(i = 1; i < M_IN; i = i + 1)
        begin: CONNECT_V
            for(j = 1; j < N_OUT; j = j + 1)
            begin: CONNECT_H
                assign h_in_array[i * N_OUT + j] = h_out_array[(i - 1) * N_OUT + j];
                assign v_in_array[(i - 1) * N_OUT + j] = v_out_array[i * N_OUT + j];
            end
        end
    endgenerate

    generate 
        for(i = 0; i < M_IN; i = i + 1)
        begin: V
            for(j = 0 ; j < N_OUT; j = j + 1) 
            begin: H
                xpoint (
                    .h_in(h_in_array[i * N_OUT + j]),
                    .v_in(v_in_array[i * N_OUT + j]),
                    .cross_enable(xpoints_enable[i * N_OUT + j]),
                    .h_out(h_out_array[i * N_OUT + j]),
                    .v_out(v_out_array[i * N_OUT + j])
                );
            end
        end
    endgenerate


endmodule
