`include "para.sv"
module VC
(
    input clk,
    input rst,
    
    output reg [2:0] G, //Global: state Either idle (I), routing (R), waiting for an output VC (V), active (A), or waiting for credits (C).
    output [ROUTE_LEN - 1 : 0] R, //Route: After routing is completed for a packet, this field holds the output port selected for the packet.
    output reg [3:0] O, //Output VC: After virtual-channel allocation is completed for a packet, this field holds the output virtual channel of port R assigned to the packet.
    output reg [FLIT_SIZE - 1:0] P, //the number of the empty slots in VC
    output vc_full,
	 input C,//Credit count: The number of credits (available downstream flit buffers) for output virtual channel O on output port R.
    
    input [FLIT_SIZE-1:0] flit_in,
    input valid_in,
    input [ROUTE_LEN - 1 : 0] route_in, //the output port number of current flit_in
    
    input grant, //granted by switch, the last flit in the VC is allowed to exit from the VC
    output [FLIT_SIZE-1:0] flit_out,
    output valid_out,

    output vc_idle
    
);



    wire buffer_produce;
    wire buffer_consume;
    wire buffer_full;
    wire buffer_empty;
    wire [FLIT_SIZE + ROUTE_LEN - 1 : 0] buffer_usedw;
    
    wire in_is_head;

    wire out_is_head;

    wire out_is_tail;

    wire in_is_single;
    wire out_is_single;





//VC states list
    parameter IDLE                = 3'd0;
    parameter ROUTING             = 3'd1;
    parameter WAITING_FOR_OVC      = 3'd2;
    parameter ACTIVE              = 3'd3;
    parameter WAITING_FOR_CREDITS = 3'd4;


    assign vc_idle = (G == IDLE);

    always@(posedge clk) begin
        if(rst) begin
            G <= IDLE;
        end
        else begin
            case(G)
                IDLE: begin  //empty buffer and No packet is occupying VC  
                    if(valid_in) begin  
                        if(~ buffer_full) begin
                            if(in_is_head || in_is_single) begin
                                G <= WAITING_FOR_OVC;
                            end
                            else begin
                                G <= ACTIVE;
                            end
                        end
                        else begin
                            G <= WAITING_FOR_CREDITS;
                        end
                    end
                    else begin
                        G <= IDLE;
                    end
                end
                ACTIVE: begin  // the flit_out has a granted OVC, buffer might be empty
                    if(C) begin
                        if(buffer_usedw == 1) begin
                            if(in_is_head || in_is_single) begin
                                G <= WAITING_FOR_OVC;
                            end
                            else if(valid_in) begin
                                G <= ACTIVE;
                            end
                            else if(out_is_tail || out_is_single) begin
                                G <= IDLE;
                            end
                            else begin
                                G <= ACTIVE;
                            end
                        end
                        else begin //there are at least 2 flits in the VC
                            if(out_is_tail || out_is_single) begin // the next out flit will be a head flit
                                G <= WAITING_FOR_OVC;
                            end
                            else begin
                                G <= ACTIVE;
                            end
                        end
                    end
                    else begin
                        G <= ACTIVE;//downstream OVC has no credits
                    end
                end
                WAITING_FOR_OVC: begin
                    if(grant) begin
                        G <= ACTIVE;
                    end
                    else begin
                        G <= WAITING_FOR_OVC;
                    end
                end
                default begin
                    G <= G;
                end
            endcase
        end
    end


    always@(posedge clk) begin
        if(rst) begin
            O <= 4'hf; //f means no granted OVC
        end
        else begin
            if(G == WAITING_FOR_OVC && grant) begin
                O <= {1'b1, R};
            end
        end
    end

                



            

    assign in_is_head = valid_in && (flit_in[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT); 


    assign in_is_single = valid_in && (flit_in[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT); 

    assign out_is_head = (~ buffer_empty) && (flit_out[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT);

    assign out_is_tail = (~ buffer_empty) && (flit_out[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT);

    assign out_is_single = (~ buffer_empty) && (flit_out[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT);
 
 
    wire [FLIT_SIZE + ROUTE_LEN - 1 : 0] buffer_out;

    buffer#(
        .buffer_depth_log(4),
        .buffer_width(FLIT_SIZE + ROUTE_LEN),
        .buffer_depth(VC_SIZE)
    )VC_buffer(
        .clk        (clk),
        .rst        (rst),
        .in         ({route_in, flit_in}),
        .produce    (buffer_produce),
        .consume    (buffer_consume),
        .full       (buffer_full),
        .empty      (buffer_empty),
        .out        (buffer_out),
        .usedw      (buffer_usedw)
    );

    assign buffer_consume = (G == ACTIVE) && (C);

    assign buffer_produce = valid_in;

	 assign R = buffer_out[FLIT_SIZE + ROUTE_LEN - 1 : FLIT_SIZE];
	 
	 assign flit_out = buffer_out[FLIT_SIZE - 1 : 0];

    assign P = VC_SIZE - 1 - buffer_usedw;

    assign vc_full = buffer_full;
    
    assign valid_out = ~buffer_empty && (G == ACTIVE);
    

    
endmodule


    
