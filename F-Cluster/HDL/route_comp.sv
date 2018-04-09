
`include "para.sv"
//the packet format:
//head flit   |type|VC class|dst|cmp|payload| 
//body flits  |type|                 payload|
//tail flit   |type|                 payload|
//
//the |cmp| field will be the priority field for the switch allocation
//if the SA policy is farthest first, this will be the distance from the destination node to the current node, if the SA policy is the oldest first, this will be the time stamp when this packet is sent
//in order to prevent deadlock, there are 6 turns should be forbidden if it is not xyz routing
//zneg -> xpos
//zneg -> xneg
//zneg -> ypos
//zneg -> yneg
//yneg -> xpos
//yneg -> xneg
`define FARTHEST_FIRST
//`define OLDEST_FIRST


`define DOR_XYZ
module route_comp
#(
    parameter cur_x = 0,
    parameter cur_y = 0,
    parameter cur_z = 0
)(
    input clk,
    input rst,
    input flit_valid_in,
    input [FLIT_SIZE - 1 : 0] flit_before_RC, 
    input stall,
    input [2 : 0] dir_in,
    output reg [FLIT_SIZE - 1 : 0] flit_after_RC,
    output flit_valid_out,
    output reg [2 : 0] dir_out, //this is going to hold until next head
    output eject_enable
);


    wire [XW - 1 : 0] dst_x;
    wire [YW - 1 : 0] dst_y;
    wire [ZW - 1 : 0] dst_z;
    reg [2:0] dir;

    reg new_VC_class; 
    wire old_VC_class;

    reg turn;

    reg flit_out_tmp;
    wire ejecting;


    assign dst_z = flit_before_RC[DST_ZPOS : DST_ZPOS - ZW + 1];
    assign dst_y = flit_before_RC[DST_YPOS : DST_YPOS - YW + 1];
    assign dst_x = flit_before_RC[DST_XPOS : DST_XPOS - XW + 1];

    assign old_VC_class = flit_before_RC[VC_CLASS_POS];

    //the VC_class needs to be changed when either crossing the dateline or changing dimension

    always@(*) begin
        if(dir_in != dir + 3 && dir != dir_in + 3) begin
            if(dir == DIR_XPOS || dir == DIR_YPOS || dir == DIR_ZPOS) begin
                new_VC_class = 0;
            end
            else begin 
                new_VC_class = 1;
            end
        end
        else begin 
            case(dir)
                DIR_XPOS: begin
                    if(cur_x == 0) begin
                        new_VC_class = 1;
                    end
                    else begin
                        new_VC_class = old_VC_class;
                    end
                end
                DIR_XNEG: begin
                    if(cur_x == XSIZE - 1) begin
                        new_VC_class = 0;
                    end
                    else begin
                        new_VC_class = old_VC_class;
                    end
                
                end
                DIR_YPOS: begin
                    if(cur_y == 0) begin
                        new_VC_class = 1;
                    end
                    else begin
                        new_VC_class = old_VC_class;
                    end
                end
                DIR_YNEG: begin
                    if(cur_y == YSIZE - 1) begin
                        new_VC_class = 0;
                    end
                    else begin
                        new_VC_class = old_VC_class;
                    end
                
                end
                DIR_ZPOS: begin
                    if(cur_z == 0) begin
                        new_VC_class = 1;
                    end
                    else begin
                        new_VC_class = old_VC_class;
                    end
                end
                DIR_ZNEG: begin
                    if(cur_z == ZSIZE - 1) begin
                        new_VC_class = 0;
                    end
                    else begin
                        new_VC_class = old_VC_class;
                    end
                
                end
                default:
                    new_VC_class = old_VC_class;


                
            endcase
        end
    end



    reg ejecting_started;

    wire start_ejecting;

    wire single_ejecting;

    wire stop_ejecting;
    reg stop_ejecting_reg;

    reg ejecting_delay;

    assign start_ejecting = flit_valid_in && (flit_before_RC[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) && dir == DIR_EJECT;

    assign single_ejecting = flit_valid_in && (flit_before_RC[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT) && dir == DIR_EJECT;


    assign stop_ejecting = flit_valid_in && ((flit_before_RC[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT));

    always@(posedge clk) begin
        stop_ejecting_reg <= stop_ejecting;
    end

    assign ejecting = single_ejecting || (start_ejecting || (ejecting_started && ~stop_ejecting_reg));

    always@(posedge clk) begin
        ejecting_delay <= ejecting;
    end
    reg flit_valid_in_reg;
    assign eject_enable = ejecting_delay && flit_valid_in_reg;


    always@(posedge clk) begin
        if(rst) begin 
            ejecting_started <= 0;
        end
        else begin
            if(start_ejecting) begin
                ejecting_started <= 1;
            end
            else begin
                if(ejecting_started) begin
                    if(stop_ejecting) begin
                        ejecting_started <= 0;
                    end
                end
            end
        end
    end



    always@(posedge clk) begin
        if(~stall) begin
            flit_valid_in_reg <= flit_valid_in;
        end
    end

    assign flit_valid_out = flit_valid_in_reg && ~eject_enable;


`ifdef FARTHEST_FIRST
    wire [CMP_LEN - 1 : 0] nxt_priority_field;
    assign nxt_priority_field = (flit_before_RC[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT || flit_before_RC[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN]) ? (flit_before_RC[CMP_POS : CMP_POS - CMP_LEN + 1] - 1) : flit_before_RC[CMP_POS : CMP_POS - CMP_LEN];
`endif
    

    always@(posedge clk) begin
        if(rst) begin
            dir_out<=0;
        end
        else if(~ejecting) begin
            if(~ stall) begin
`ifdef FARTHEST_FIRST
                flit_after_RC <= {flit_before_RC[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN], new_VC_class, flit_before_RC[DST_ZPOS : CMP_POS + 1], nxt_priority_field, flit_before_RC[CMP_POS - CMP_LEN : 0]};
`else
                flit_after_RC <= {flit_before_RC[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN], new_VC_class, flit_before_RC[FLIT_SIZE - HEADER_LEN - 2  : 0]};
`endif
                if((flit_before_RC[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) || (flit_before_RC[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT))begin
                    dir_out<=dir;
                end
            end
        end
        else begin //dir equals to eject port
            flit_after_RC <= {flit_before_RC[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN], new_VC_class, flit_before_RC[FLIT_SIZE - HEADER_LEN - 2  : 0]};
            dir_out <= DIR_EJECT;
        end
    end



`ifdef DOR_XYZ
    //for simplicity start with xyz routing first
    always@(*) begin
        if(cur_x != dst_x) begin
            if(cur_x > dst_x) begin
                if(cur_x - dst_x >= XSIZE/2) begin
                    dir = DIR_XPOS;
                end
                else begin
                    dir = DIR_XNEG;
                end
            end
            else begin
                if(dst_x - cur_x <= XSIZE/2) begin
                    dir = DIR_XPOS;
                end
                else begin
                    dir = DIR_XNEG;
                end
            end
        end
        else if(cur_y != dst_y) begin
            if(cur_y > dst_y) begin
                if(cur_y - dst_y >= YSIZE/2) begin
                    dir = DIR_YPOS;
                end
                else begin
                    dir = DIR_YNEG;
                end
            end
            else begin
                if(dst_y - cur_y <= YSIZE/2) begin
                    dir = DIR_YPOS;
                end
                else begin
                    dir = DIR_YNEG;
                end
            end
        end
        else if(cur_z != dst_z) begin
            if(cur_z > dst_z) begin
                if(cur_z - dst_z >= ZSIZE/2) begin
                    dir = DIR_ZPOS;
                end
                else begin
                    dir = DIR_ZNEG;
                end
            end
            else begin
                if(dst_z - cur_z <= ZSIZE/2) begin
                    dir = DIR_ZPOS;
                end 
                else begin
                    dir = DIR_ZNEG;
                end
            end
        end
        else begin
            dir = DIR_EJECT;
        end
    end
`endif
endmodule
        
