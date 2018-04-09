//input unit for dynamic router
//packet format
//|        |        |         |             |
//|  TYPE  |   DST  |  CHECK  |   PAYLOAD   |
//|        |        |         |             |
//
`include "para.sv"
module input_unit
#(
    parameter X=3'd0,
    parameter Y=3'd0,
    parameter Z=3'd0
)(
    input clk,
    input rst,
    input [FLIT_SIZE-1:0] data_in,
    input valid_in,
    
    output [FLIT_SIZE-1:0] credit_out
);
    
    wire [FLIT_SIZE-1:0] data_deQ;
    wire dequeue;
    wire inQ_full;
    wire inQ_empty;

    wire [DSTW-1:0] dst;



    //input queue
    buffer#(
        .buffer_width(FLIT_SIZE),
        .buffer_depth(IN_Q_SIZE)
    )in_queue(
        .clk(clk),
        .rst(rst),
        .in(data_in),
        .produce(valid_in),
        .consume(dequeue),
        .full(inQ_full),
        .empty(inQ_empty),
        .out(data_deQ),
        .usedw(credit_out)
    );

    //route computation unit
    
    



    //input virtual channels
    //


    //crossbar switch 
    //
    //


    //output unit 
    

endmodule
    



    

    

    


