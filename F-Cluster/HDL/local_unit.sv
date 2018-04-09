
`include "para.sv"
//`define NEAREST_NEIGHBOR
//`define TORUS_LOOP_TEST
//`define THREE_HOP_DIAGONAL
//`define X_Y_CORNER_TURN
`define BOARD_TEST // board test is in a 2x2x1 torus, no traffic should be sent on z dimension
module local_unit#(
    parameter cur_x = 3'd0,
    parameter cur_y = 3'd0,
    parameter cur_z = 3'd0
)(
    input clk,
    input rst,
    input [FLIT_SIZE - 1 : 0] eject_xpos,
    input eject_xpos_valid,
    input [FLIT_SIZE - 1 : 0] eject_ypos,
    input eject_ypos_valid,
    input [FLIT_SIZE - 1 : 0] eject_zpos,
    input eject_zpos_valid,
    input [FLIT_SIZE - 1 : 0] eject_xneg,
    input eject_xneg_valid,
    input [FLIT_SIZE - 1 : 0] eject_yneg,
    input eject_yneg_valid,
    input [FLIT_SIZE - 1 : 0] eject_zneg,
    input eject_zneg_valid,

    output  [FLIT_SIZE - 1 : 0] inject_xpos,
    output inject_xpos_valid,
    output [FLIT_SIZE - 1 : 0] inject_ypos,
    output inject_ypos_valid,
    output [FLIT_SIZE - 1 : 0] inject_zpos,
    output inject_zpos_valid,
    output [FLIT_SIZE - 1 : 0] inject_xneg,
    output inject_xneg_valid,
    output [FLIT_SIZE - 1 : 0] inject_yneg,
    output inject_yneg_valid,
    output [FLIT_SIZE - 1 : 0] inject_zneg,
    output inject_zneg_valid,
    input inject_xpos_avail,
    input inject_ypos_avail,
    input inject_zpos_avail,
    input inject_xneg_avail,
    input inject_yneg_avail,
    input inject_zneg_avail

);

    parameter injection_rate=10'd0; //injection rate means inject one packet per injection_rate cycles

    parameter packet_size = 1; //number of flits in one packet
    
    parameter packet_num = 100;

    reg [15 : 0] packet_counter; 


    reg [9:0] injection_control_counter;
    reg [9 : 0 ] flit_counter;

    wire injection_enable;

    reg [FLIT_SIZE - 1 : 0] app_xpos_inject;
    reg [FLIT_SIZE - 1 : 0] app_ypos_inject;
    reg [FLIT_SIZE - 1 : 0] app_zpos_inject;
    reg [FLIT_SIZE - 1 : 0] app_xneg_inject;
    reg [FLIT_SIZE - 1 : 0] app_yneg_inject;
    reg [FLIT_SIZE - 1 : 0] app_zneg_inject;

    reg app_xpos_inject_valid;
    reg app_ypos_inject_valid;
    reg app_zpos_inject_valid;
    reg app_xneg_inject_valid;
    reg app_yneg_inject_valid;
    reg app_zneg_inject_valid;


    reg [3 : 0] xpos_check_state;
    reg [3 : 0] ypos_check_state;
    reg [3 : 0] zpos_check_state;
    reg [3 : 0] xneg_check_state;
    reg [3 : 0] yneg_check_state;
    reg [3 : 0] zneg_check_state;

    parameter IDLE = 4'd0;
    parameter PACKET_RCVING = 4'd1;
    parameter WAITING_FOR_PACKET = 4'd2;
    parameter ERROR = 4'd3;
    


    
    //packet checker
    //expect packets from 6 nearest neighbors

    reg [15 : 0] xpos_flit_counter;
    reg [15 : 0] xpos_pckt_counter;
    reg [15 : 0] xpos_flit_timeout_counter;
    

    reg [3 : 0] xpos_error_code;
    reg [8 : 0] xpos_src_id;

    always@(posedge clk) begin
        if(rst) begin
            xpos_check_state <= IDLE;
            xpos_flit_counter <= 0;
            xpos_pckt_counter <= 0;
            xpos_flit_timeout_counter <= 0;
            xpos_error_code <= ERR_NO_ERROR;
        end
        else begin
            case(xpos_check_state)
                IDLE: begin
                    if(eject_xpos_valid) begin
                        if(eject_xpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) begin 
                            xpos_check_state <= PACKET_RCVING;
                            xpos_flit_counter <= 1;
                        end
                        else if(eject_xpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT) begin
                            xpos_check_state <= IDLE;
                            xpos_pckt_counter <= xpos_pckt_counter + 1;
                        end
                        else begin
                            xpos_check_state <= ERROR;
                            xpos_error_code <= ERR_FLIT_WRONG;
                        end
                    end
                end
                PACKET_RCVING: begin
                    if(eject_xpos_valid) begin
                        xpos_flit_timeout_counter <= 0;
                        if(eject_xpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == BODY_FLIT) begin
                            xpos_flit_counter <= xpos_flit_counter + 1;
                        end
                        else if(eject_xpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT) begin
                            xpos_flit_counter <= 0;
                            if(xpos_flit_counter < packet_size - 1) begin
                                xpos_check_state <= ERROR;
                                xpos_error_code <= ERR_FLIT_MISSING;
                            end
                            else if(xpos_flit_counter >= packet_size) begin
                                xpos_check_state <= ERROR;
                                xpos_error_code <= ERR_FLIT_WRONG;
                            end
                            else begin
                                xpos_pckt_counter <= xpos_pckt_counter + 1;
                                xpos_check_state <= IDLE;
                            end
                        end
                        else begin
                            xpos_check_state <= ERROR;
                            xpos_error_code <= ERR_FLIT_MISSING;
                        end
                    end
                    else begin
                        xpos_flit_timeout_counter <= xpos_flit_timeout_counter + 1;
                        if(xpos_flit_timeout_counter > 1000) begin
                            xpos_check_state <= ERROR;
                            xpos_error_code <= ERR_FLIT_TIMEOUT;
                        end
                    end
                end
                ERROR: begin
                    xpos_check_state <= ERROR;
                end
            endcase
        end
    end


    reg [15 : 0] ypos_flit_counter;
    reg [15 : 0] ypos_pckt_counter;
    reg [15 : 0] ypos_flit_timeout_counter;
    

    reg [3 : 0] ypos_error_code;
    reg [8 : 0] ypos_src_id;

    always@(posedge clk) begin
        if(rst) begin
            ypos_check_state <= IDLE;
            ypos_flit_counter <= 0;
            ypos_pckt_counter <= 0;
            ypos_flit_timeout_counter <= 0;
            ypos_error_code <= ERR_NO_ERROR;
        end
        else begin
            case(ypos_check_state)
                IDLE: begin
                    if(eject_ypos_valid) begin
                        if(eject_ypos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) begin 
                            ypos_check_state <= PACKET_RCVING;
                            ypos_flit_counter <= 1;
                        end
                        else if(eject_ypos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT) begin
                            ypos_check_state <= IDLE;
                            ypos_pckt_counter <= ypos_pckt_counter + 1;
                        end
                        else begin
                            ypos_check_state <= ERROR;
                            ypos_error_code <= ERR_FLIT_WRONG;
                        end
                    end
                end
                PACKET_RCVING: begin
                    if(eject_ypos_valid) begin
                        ypos_flit_timeout_counter <= 0;
                        if(eject_ypos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == BODY_FLIT) begin
                            ypos_flit_counter <= ypos_flit_counter + 1;
                        end
                        else if(eject_ypos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT) begin
                            ypos_flit_counter <= 0;
                            if(ypos_flit_counter < packet_size - 1) begin
                                ypos_check_state <= ERROR;
                                ypos_error_code <= ERR_FLIT_MISSING;
                            end
                            else if(ypos_flit_counter >= packet_size) begin
                                ypos_check_state <= ERROR;
                                ypos_error_code <= ERR_FLIT_WRONG;
                            end
                            else begin
                                ypos_pckt_counter <= ypos_pckt_counter + 1;
                                ypos_check_state <= IDLE;
                            end
                        end
                        else begin
                            ypos_check_state <= ERROR;
                            ypos_error_code <= ERR_FLIT_MISSING;
                        end
                    end
                    else begin
                        ypos_flit_timeout_counter <= ypos_flit_timeout_counter + 1;
                        if(ypos_flit_timeout_counter > 1000) begin
                            ypos_check_state <= ERROR;
                            ypos_error_code <= ERR_FLIT_TIMEOUT;
                        end
                    end
                end
                ERROR: begin
                    ypos_check_state <= ERROR;
                end
            endcase
        end
    end



    reg [15 : 0] zpos_flit_counter;
    reg [15 : 0] zpos_pckt_counter;
    reg [15 : 0] zpos_flit_timeout_counter;
    

    reg [3 : 0] zpos_error_code;
    reg [8 : 0] zpos_src_id;

    always@(posedge clk) begin
        if(rst) begin
            zpos_check_state <= IDLE;
            zpos_flit_counter <= 0;
            zpos_pckt_counter <= 0;
            zpos_flit_timeout_counter <= 0;
            zpos_error_code <= ERR_NO_ERROR;
        end
        else begin
            case(zpos_check_state)
                IDLE: begin
                    if(eject_zpos_valid) begin
                        if(eject_zpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) begin 
                            zpos_check_state <= PACKET_RCVING;
                            zpos_flit_counter <= 1;
                        end
                        else if(eject_zpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT) begin
                            zpos_check_state <= IDLE;
                            zpos_pckt_counter <= zpos_pckt_counter + 1;
                        end
                        else begin
                            zpos_check_state <= ERROR;
                            zpos_error_code <= ERR_FLIT_WRONG;
                        end
                    end
                end
                PACKET_RCVING: begin
                    if(eject_zpos_valid) begin
                        zpos_flit_timeout_counter <= 0;
                        if(eject_zpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == BODY_FLIT) begin
                            zpos_flit_counter <= zpos_flit_counter + 1;
                        end
                        else if(eject_zpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT) begin
                            zpos_flit_counter <= 0;
                            if(zpos_flit_counter < packet_size - 1) begin
                                zpos_check_state <= ERROR;
                                zpos_error_code <= ERR_FLIT_MISSING;
                            end
                            else if(zpos_flit_counter >= packet_size) begin
                                zpos_check_state <= ERROR;
                                zpos_error_code <= ERR_FLIT_WRONG;
                            end
                            else begin
                                zpos_pckt_counter <= zpos_pckt_counter + 1;
                                zpos_check_state <= IDLE;
                            end
                        end
                        else begin
                            zpos_check_state <= ERROR;
                            zpos_error_code <= ERR_FLIT_MISSING;
                        end
                    end
                    else begin
                        zpos_flit_timeout_counter <= zpos_flit_timeout_counter + 1;
                        if(zpos_flit_timeout_counter > 1000) begin
                            zpos_check_state <= ERROR;
                            zpos_error_code <= ERR_FLIT_TIMEOUT;
                        end
                    end
                end
                ERROR: begin
                    zpos_check_state <= ERROR;
                end
            endcase
        end
    end



    reg [15 : 0] xneg_flit_counter;
    reg [15 : 0] xneg_pckt_counter;
    reg [15 : 0] xneg_flit_timeout_counter;
    

    reg [3 : 0] xneg_error_code;
    reg [8 : 0] xneg_src_id;

    always@(posedge clk) begin
        if(rst) begin
            xneg_check_state <= IDLE;
            xneg_flit_counter <= 0;
            xneg_pckt_counter <= 0;
            xneg_flit_timeout_counter <= 0;
            xneg_error_code <= ERR_NO_ERROR;
        end
        else begin
            case(xneg_check_state)
                IDLE: begin
                    if(eject_xneg_valid) begin
                        if(eject_xneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) begin 
                            xneg_check_state <= PACKET_RCVING;
                            xneg_flit_counter <= 1;
                        end
                        else if(eject_xneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT) begin
                            xneg_check_state <= IDLE;
                            xneg_pckt_counter <= xneg_pckt_counter + 1;
                        end
                        else begin
                            xneg_check_state <= ERROR;
                            xneg_error_code <= ERR_FLIT_WRONG;
                        end
                    end
                end
                PACKET_RCVING: begin
                    if(eject_xneg_valid) begin
                        xneg_flit_timeout_counter <= 0;
                        if(eject_xneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == BODY_FLIT) begin
                            xneg_flit_counter <= xneg_flit_counter + 1;
                        end
                        else if(eject_xneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT) begin
                            xneg_flit_counter <= 0;
                            if(xneg_flit_counter < packet_size - 1) begin
                                xneg_check_state <= ERROR;
                                xneg_error_code <= ERR_FLIT_MISSING;
                            end
                            else if(xneg_flit_counter >= packet_size) begin
                                xneg_check_state <= ERROR;
                                xneg_error_code <= ERR_FLIT_WRONG;
                            end
                            else begin
                                xneg_pckt_counter <= xneg_pckt_counter + 1;
                                xneg_check_state <= IDLE;
                            end
                        end
                        else begin
                            xneg_check_state <= ERROR;
                            xneg_error_code <= ERR_FLIT_MISSING;
                        end
                    end
                    else begin
                        xneg_flit_timeout_counter <= xneg_flit_timeout_counter + 1;
                        if(xneg_flit_timeout_counter > 1000) begin
                            xneg_check_state <= ERROR;
                            xneg_error_code <= ERR_FLIT_TIMEOUT;
                        end
                    end
                end
                ERROR: begin
                    xneg_check_state <= ERROR;
                end
            endcase
        end
    end


    reg [15 : 0] yneg_flit_counter;
    reg [15 : 0] yneg_pckt_counter;
    reg [15 : 0] yneg_flit_timeout_counter;
    

    reg [3 : 0] yneg_error_code;
    reg [8 : 0] yneg_src_id;

    always@(posedge clk) begin
        if(rst) begin
            yneg_check_state <= IDLE;
            yneg_flit_counter <= 0;
            yneg_pckt_counter <= 0;
            yneg_flit_timeout_counter <= 0;
            yneg_error_code <= ERR_NO_ERROR;
        end
        else begin
            case(yneg_check_state)
                IDLE: begin
                    if(eject_yneg_valid) begin
                        if(eject_yneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) begin 
                            yneg_check_state <= PACKET_RCVING;
                            yneg_flit_counter <= 1;
                        end
                        else if(eject_yneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT) begin
                            yneg_check_state <= IDLE;
                            yneg_pckt_counter <= yneg_pckt_counter + 1;
                        end
                        else begin
                            yneg_check_state <= ERROR;
                            yneg_error_code <= ERR_FLIT_WRONG;
                        end
                    end
                end
                PACKET_RCVING: begin
                    if(eject_yneg_valid) begin
                        yneg_flit_timeout_counter <= 0;
                        if(eject_yneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == BODY_FLIT) begin
                            yneg_flit_counter <= yneg_flit_counter + 1;
                        end
                        else if(eject_yneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT) begin
                            yneg_flit_counter <= 0;
                            if(yneg_flit_counter < packet_size - 1) begin
                                yneg_check_state <= ERROR;
                                yneg_error_code <= ERR_FLIT_MISSING;
                            end
                            else if(yneg_flit_counter >= packet_size) begin
                                yneg_check_state <= ERROR;
                                yneg_error_code <= ERR_FLIT_WRONG;
                            end
                            else begin
                                yneg_pckt_counter <= yneg_pckt_counter + 1;
                                yneg_check_state <= IDLE;
                            end
                        end
                        else begin
                            yneg_check_state <= ERROR;
                            yneg_error_code <= ERR_FLIT_MISSING;
                        end
                    end
                    else begin
                        yneg_flit_timeout_counter <= yneg_flit_timeout_counter + 1;
                        if(yneg_flit_timeout_counter > 1000) begin
                            yneg_check_state <= ERROR;
                            yneg_error_code <= ERR_FLIT_TIMEOUT;
                        end
                    end
                end
                ERROR: begin
                    yneg_check_state <= ERROR;
                end
            endcase
        end
    end


    reg [15 : 0] zneg_flit_counter;
    reg [15 : 0] zneg_pckt_counter;
    reg [15 : 0] zneg_flit_timeout_counter;
    

    reg [3 : 0] zneg_error_code;
    reg [8 : 0] zneg_src_id;

    always@(posedge clk) begin
        if(rst) begin
            zneg_check_state <= IDLE;
            zneg_flit_counter <= 0;
            zneg_pckt_counter <= 0;
            zneg_flit_timeout_counter <= 0;
            zneg_error_code <= ERR_NO_ERROR;
        end
        else begin
            case(zneg_check_state)
                IDLE: begin
                    if(eject_zneg_valid) begin
                        if(eject_zneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) begin 
                            zneg_check_state <= PACKET_RCVING;
                            zneg_flit_counter <= 1;
                        end
                        else if(eject_zneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == SINGLE_FLIT) begin
                            zneg_check_state <= IDLE;
                            zneg_pckt_counter <= zneg_pckt_counter + 1;
                        end
                        else begin
                            zneg_check_state <= ERROR;
                            zneg_error_code <= ERR_FLIT_WRONG;
                        end
                    end
                end
                PACKET_RCVING: begin
                    if(eject_zneg_valid) begin
                        zneg_flit_timeout_counter <= 0;
                        if(eject_zneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == BODY_FLIT) begin
                            zneg_flit_counter <= zneg_flit_counter + 1;
                        end
                        else if(eject_zneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT) begin
                            zneg_flit_counter <= 0;
                            if(zneg_flit_counter < packet_size - 1) begin
                                zneg_check_state <= ERROR;
                                zneg_error_code <= ERR_FLIT_MISSING;
                            end
                            else if(zneg_flit_counter >= packet_size) begin
                                zneg_check_state <= ERROR;
                                zneg_error_code <= ERR_FLIT_WRONG;
                            end
                            else begin
                                zneg_pckt_counter <= zneg_pckt_counter + 1;
                                zneg_check_state <= IDLE;
                            end
                        end
                        else begin
                            zneg_check_state <= ERROR;
                            zneg_error_code <= ERR_FLIT_MISSING;
                        end
                    end
                    else begin
                        zneg_flit_timeout_counter <= zneg_flit_timeout_counter + 1;
                        if(zneg_flit_timeout_counter > 1000) begin
                            zneg_check_state <= ERROR;
                            zneg_error_code <= ERR_FLIT_TIMEOUT;
                        end
                    end
                end
                ERROR: begin
                    zneg_check_state <= ERROR;
                end
            endcase
        end
    end

    //check whether all the pckts have been received or not
    wire all_pckts_rcvd;
`ifdef NEAREST_NEIGHBOR
    assign all_pckts_rcvd = (xpos_pckt_counter + ypos_pckt_counter + zpos_pckt_counter + xneg_pckt_counter + yneg_pckt_counter + zneg_pckt_counter == packet_num * 6); 
`endif

`ifdef THREE_HOP_DIAGONAL
    assign all_pckts_rcvd = (xpos_pckt_counter + ypos_pckt_counter + zpos_pckt_counter + xneg_pckt_counter + yneg_pckt_counter + zneg_pckt_counter == packet_num * 6); 
`endif

`ifdef TORUS_LOOP_TEST
    assign all_pckts_rcvd = (xpos_pckt_counter + ypos_pckt_counter + zpos_pckt_counter + xneg_pckt_counter + yneg_pckt_counter + zneg_pckt_counter == packet_num * 6); 
`endif

`ifdef BOARD_TEST
    assign all_pckts_rcvd = (xpos_pckt_counter + ypos_pckt_counter + xneg_pckt_counter + yneg_pckt_counter == packet_num * 3); 

`endif

    always@(posedge clk) begin
        if(rst) begin
            packet_counter <= 0;
        end
        else begin
            if(flit_counter == packet_size) begin
                packet_counter <= (packet_counter < packet_num) ? packet_counter + 1 : packet_num;
            end
        end
    end

    always@(posedge clk) begin
        if(rst) begin
            flit_counter <= 0;
        end
        else begin
            if(flit_counter == 0) begin
                if(injection_rate == 0) begin
                    flit_counter <= 1;
                end
                else if(injection_control_counter == injection_rate - 1) begin
                    flit_counter <= 1;
                end
            end
            else begin
                flit_counter <= flit_counter < packet_size ? flit_counter + 1 : (injection_rate == 0);
            end
        end
    end


    always@(posedge clk) begin
        if(rst) begin
            injection_control_counter <= 0;
        end
        else begin
            injection_control_counter <= (injection_control_counter == injection_rate + packet_size - 1) ? 0 : injection_control_counter + 1;
        end
    end

    assign injection_enable = (flit_counter >= 1 && flit_counter <= packet_size && packet_counter < packet_num);


    assign app_xpos_inject_valid = injection_enable;
    assign app_ypos_inject_valid = injection_enable;
    assign app_zpos_inject_valid = injection_enable;
    assign app_xneg_inject_valid = injection_enable;
    assign app_yneg_inject_valid = injection_enable;
    assign app_zneg_inject_valid = injection_enable;





    wire [2 : 0] nxt_x;
    wire [2 : 0] pre_x;
    wire [2 : 0] nxt_y;
    wire [2 : 0] pre_y;

    wire [2 : 0] pre_z;
    wire [2 : 0] nxt_z;

    assign nxt_x = cur_x < XSIZE - 1 ? cur_x + 1 : 0;
    assign nxt_y = cur_y < YSIZE - 1 ? cur_y + 1 : 0;
    assign nxt_z = cur_z < ZSIZE - 1 ? cur_z + 1 : 0;
    assign pre_x = cur_x > 0 ? cur_x - 1 : XSIZE - 1;
    assign pre_y = cur_y > 0 ? cur_y - 1 : YSIZE - 1;
    assign pre_z = cur_z > 0 ? cur_z - 1 : ZSIZE - 1;

    wire [2 : 0] nxtnxt_x;
    wire [2 : 0] nxtnxt_y;
    wire [2 : 0] nxtnxt_z;
    wire [2 : 0] prepre_x;
    wire [2 : 0] prepre_y;
    wire [2 : 0] prepre_z;

    assign nxtnxt_x = cur_x + 2 <= XSIZE - 1 ? cur_x + 2 : cur_x + 2 - XSIZE;
    assign nxtnxt_y = cur_y + 2 <= YSIZE - 1 ? cur_y + 2 : cur_y + 2 - YSIZE;
    assign nxtnxt_z = cur_z + 2 <= ZSIZE - 1 ? cur_z + 2 : cur_z + 2 - ZSIZE;

    assign prepre_x = cur_x >= 2 ? cur_x - 2 : cur_x - 2 + XSIZE;
    assign prepre_y = cur_y >= 2 ? cur_y - 2 : cur_y - 2 + YSIZE;
    assign prepre_z = cur_z >= 2 ? cur_z - 2 : cur_z - 2 + ZSIZE;
//the injection port selection should make sure the turn rules to prevent deadlock
//
//
//
`ifdef BOARD_TEST
    always@(*) begin
        if(packet_size == 1) begin
            app_xpos_inject = {SINGLE_FLIT, 1'b0, cur_z, cur_y, nxt_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hBEAD};
        end
        else if(flit_counter == 1) begin
            app_xpos_inject = {HEAD_FLIT, 1'b0, cur_z, cur_y, nxt_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hBEAD};
        end
        else if(flit_counter == packet_size) begin
            app_xpos_inject = {TAIL_FLIT, 125'hBA11};
        end
        else begin
            app_xpos_inject = {BODY_FLIT, 125'hBEEF};
        end    
    end

    always@(*) begin
       if(packet_size == 1) begin
            app_ypos_inject = {SINGLE_FLIT, 1'b0, cur_z, nxt_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hCEAD};
        end
        else if(flit_counter == 1) begin
            app_ypos_inject = {HEAD_FLIT, 1'b0, cur_z, nxt_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hCEAD};
        end
        else if(flit_counter == packet_size) begin
            app_ypos_inject = {TAIL_FLIT, 125'hCA11};
        end
        else begin
            app_ypos_inject = {BODY_FLIT, 125'hCEEF};
        end
    end  
    
    always@(*) begin
       if(packet_size == 1) begin
            app_xneg_inject = {SINGLE_FLIT, 1'b1, cur_z, nxt_y, nxt_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hDEAD};
        end
        else if(flit_counter == 1) begin
            app_xneg_inject = {HEAD_FLIT, 1'b1, cur_z, nxt_y, nxt_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hDEAD};
        end
        else if(flit_counter == packet_size) begin
            app_xneg_inject = {TAIL_FLIT, 125'hDA11};
        end
        else begin
            app_xneg_inject = {BODY_FLIT, 125'hDEEF};
        end
    end 

`endif

`ifdef TORUS_LOOP_TEST
    always@(*) begin
       if(packet_size == 1) begin
            app_xpos_inject = {SINGLE_FLIT, 1'b0, cur_z, cur_y, nxtnxt_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hBEAD};
        end
        else if(flit_counter == 1) begin
            app_xpos_inject = {HEAD_FLIT, 1'b0, cur_z, cur_y, nxtnxt_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hBEAD};
        end
        else if(flit_counter == packet_size) begin
            app_xpos_inject = {TAIL_FLIT, 125'hBA11};
        end
        else begin
            app_xpos_inject = {BODY_FLIT, 125'hBEEF};
        end
    end 
    always@(*) begin
       if(packet_size == 1) begin
            app_ypos_inject = {SINGLE_FLIT, 1'b0, cur_z, nxtnxt_y, cur_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hCEAD};
        end
        else if(flit_counter == 1) begin
            app_ypos_inject = {HEAD_FLIT, 1'b0, cur_z, nxtnxt_y, cur_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hCEAD};
        end
        else if(flit_counter == packet_size) begin
            app_ypos_inject = {TAIL_FLIT, 125'hCA11};
        end
        else begin
            app_ypos_inject = {BODY_FLIT, 125'hCEEF};
        end
    end 
    always@(*) begin
       if(packet_size == 1) begin
            app_zpos_inject = {SINGLE_FLIT, 1'b0, nxtnxt_z, cur_y, cur_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hDEAD};
        end
        else if(flit_counter == 1) begin
            app_zpos_inject = {HEAD_FLIT, 1'b0, nxtnxt_z, cur_y, cur_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hDEAD};
        end
        else if(flit_counter == packet_size) begin
            app_zpos_inject = {TAIL_FLIT, 125'hDA11};
        end
        else begin
            app_zpos_inject = {BODY_FLIT, 125'hDEEF};
        end
    end

    always@(*) begin
       if(packet_size == 1) begin
            app_xneg_inject = {SINGLE_FLIT, 1'b1, cur_z, cur_y, prepre_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hBEAD};
        end
        else if(flit_counter == 1) begin
            app_xneg_inject = {HEAD_FLIT, 1'b1, cur_z, cur_y, prepre_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hBEAD};
        end
        else if(flit_counter == packet_size) begin
            app_xneg_inject = {TAIL_FLIT, 125'hBA11};
        end
        else begin
            app_xneg_inject = {BODY_FLIT, 125'hBEEF};
        end
    end 
    always@(*) begin
       if(packet_size == 1) begin
            app_yneg_inject = {SINGLE_FLIT, 1'b1, cur_z, prepre_y, cur_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hCEAD};
        end
        else if(flit_counter == 1) begin
            app_yneg_inject = {HEAD_FLIT, 1'b1, cur_z, prepre_y, cur_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hCEAD};
        end
        else if(flit_counter == packet_size) begin
            app_yneg_inject = {TAIL_FLIT, 125'hCA11};
        end
        else begin
            app_yneg_inject = {BODY_FLIT, 125'hCEEF};
        end
    end 
    always@(*) begin
       if(packet_size == 1) begin
            app_zneg_inject = {SINGLE_FLIT, 1'b1, prepre_z, cur_y, cur_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hDEAD};
        end
        else if(flit_counter == 1) begin
            app_zneg_inject = {HEAD_FLIT, 1'b1, prepre_z, cur_y, cur_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hDEAD};
        end
        else if(flit_counter == packet_size) begin
            app_zneg_inject = {TAIL_FLIT, 125'hDA11};
        end
        else begin
            app_zneg_inject = {BODY_FLIT, 125'hDEEF};
        end
    end 
`endif

`ifdef THREE_HOP_DIAGONAL
    always@(*) begin
        if(packet_size == 1) begin
            app_xpos_inject = {SINGLE_FLIT, 1'b0, nxt_z, nxt_y, nxt_x, 4'd3, cur_z, cur_y, cur_x, packet_counter, 86'hBEAD};
        end
        else if(flit_counter == 1) begin
            app_xpos_inject = {HEAD_FLIT, 1'b0, nxt_z, nxt_y, nxt_x, 4'd3, cur_z, cur_y, cur_x, packet_counter, 86'hBEAD};
        end
        else if(flit_counter == packet_size) begin
            app_xpos_inject = {TAIL_FLIT, 125'hBA11};
        end
        else begin
            app_xpos_inject = {BODY_FLIT, 125'hBEEF};
        end
    end 
    always@(*) begin
        if(packet_size == 1) begin
            app_ypos_inject = {SINGLE_FLIT, 1'b0, nxt_z, nxt_y, cur_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == 1) begin
            app_ypos_inject = {HEAD_FLIT, 1'b0, nxt_z, nxt_y, cur_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == packet_size) begin
            app_ypos_inject = {TAIL_FLIT, 125'hA11};
        end
        else begin
            app_ypos_inject = {BODY_FLIT, 125'hBAAF};
        end
    end
    always@(*) begin
        if(packet_size == 1) begin
            app_zpos_inject = {SINGLE_FLIT, 1'b0, nxt_z, cur_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == 1) begin
            app_zpos_inject = {HEAD_FLIT, 1'b0, nxt_z, cur_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == packet_size) begin
            app_zpos_inject = {TAIL_FLIT, 125'hA11};
        end
        else begin
            app_zpos_inject = {BODY_FLIT, 125'hB00F};
        end
    end

    always@(*) begin
        if(packet_size == 1) begin
            app_xneg_inject = {SINGLE_FLIT, 1'b1, pre_z, pre_y, pre_x, 4'd3, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == 1) begin
            app_xneg_inject = {HEAD_FLIT, 1'b1, pre_z, pre_y, pre_x, 4'd3, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == packet_size) begin
            app_xneg_inject = {TAIL_FLIT, 125'hA11};
        end
        else begin
            app_xneg_inject = {BODY_FLIT, 125'hDEEF};
        end
    end 
    always@(*) begin
        if(packet_size == 1) begin
            app_yneg_inject = {SINGLE_FLIT, 1'b1, pre_z, pre_y, cur_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == 1) begin
            app_yneg_inject = {HEAD_FLIT, 1'b1, pre_z, pre_y, cur_x, 4'd2, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};    
        end
        else if(flit_counter == packet_size) begin
            app_yneg_inject = {TAIL_FLIT, 125'hA11};
        end
        else begin
            app_yneg_inject = {BODY_FLIT, 125'hDAAF};
        end
    end 
    always@(*) begin
        if(packet_size == 1) begin
            app_zneg_inject = {SINGLE_FLIT, 1'b1, pre_z, cur_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == 1) begin
            app_zneg_inject = {HEAD_FLIT, 1'b1, pre_z, cur_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == packet_size) begin
            app_zneg_inject = {TAIL_FLIT, 125'hA11};
        end
        else begin
            app_zneg_inject = {BODY_FLIT, 125'hD00F};
        end
    end 
`endif
    
`ifdef NEAREST_NEIGHBOR


    always@(*) begin
        if(packet_size == 1) begin
            app_xpos_inject = {SINGLE_FLIT, 1'b0, cur_z, cur_y, nxt_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == 1) begin
            app_xpos_inject = {HEAD_FLIT, 1'b0, cur_z, cur_y, nxt_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == packet_size) begin
            app_xpos_inject = {TAIL_FLIT, 125'hA11};
        end
        else begin
            app_xpos_inject = {BODY_FLIT, 125'hD};
        end
    end 
    always@(*) begin
        if(packet_size == 1) begin
            app_ypos_inject = {SINGLE_FLIT, 1'b0, cur_z, nxt_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == 1) begin
            app_ypos_inject = {HEAD_FLIT, 1'b0, cur_z, nxt_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == packet_size) begin
            app_ypos_inject = {TAIL_FLIT, 125'hA11};
        end
        else begin
            app_ypos_inject = {BODY_FLIT, 125'hD};
        end
    end
    always@(*) begin
        if(packet_size == 1) begin
            app_zpos_inject = {SINGLE_FLIT, 1'b0, nxt_z, cur_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == 1) begin
            app_zpos_inject = {HEAD_FLIT, 1'b0, nxt_z, cur_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == packet_size) begin
            app_zpos_inject = {TAIL_FLIT, 125'hA11};
        end
        else begin
            app_zpos_inject = {BODY_FLIT, 125'hD};
        end
    end

    always@(*) begin
        if(packet_size == 1) begin
            app_xneg_inject = {SINGLE_FLIT, 1'b1, cur_z, cur_y, pre_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == 1) begin
            app_xneg_inject = {HEAD_FLIT, 1'b1, cur_z, cur_y, pre_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == packet_size) begin
            app_xneg_inject = {TAIL_FLIT, 125'hA11};
        end
        else begin
            app_xneg_inject = {BODY_FLIT, 125'hD};
        end
    end 
    always@(*) begin
        if(packet_size == 1) begin
            app_yneg_inject = {SINGLE_FLIT, 1'b1, cur_z, pre_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == 1) begin
            app_yneg_inject = {HEAD_FLIT, 1'b1, cur_z, pre_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};    
        end
        else if(flit_counter == packet_size) begin
            app_yneg_inject = {TAIL_FLIT, 125'hA11};
        end
        else begin
            app_yneg_inject = {BODY_FLIT, 125'hD};
        end
    end 
    always@(*) begin
        if(packet_size == 1) begin
            app_zneg_inject = {SINGLE_FLIT, 1'b1, pre_z, cur_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == 1) begin
            app_zneg_inject = {HEAD_FLIT, 1'b1, pre_z, cur_y, cur_x, 4'd1, cur_z, cur_y, cur_x, packet_counter, 86'hEAD};
        end
        else if(flit_counter == packet_size) begin
            app_zneg_inject = {TAIL_FLIT, 125'hA11};
        end
        else begin
            app_zneg_inject = {BODY_FLIT, 125'hD};
        end
    end 
`endif


    //instantiate 6 inject buffers
    wire xpos_inject_buffer_empty;
    wire ypos_inject_buffer_empty;
    wire zpos_inject_buffer_empty;
    wire xneg_inject_buffer_empty;
    wire yneg_inject_buffer_empty;
    wire zneg_inject_buffer_empty;

    assign inject_xpos_valid = ~xpos_inject_buffer_empty;
    assign inject_ypos_valid = ~ypos_inject_buffer_empty;
    assign inject_zpos_valid = ~zpos_inject_buffer_empty;
    assign inject_xneg_valid = ~xneg_inject_buffer_empty;
    assign inject_yneg_valid = ~yneg_inject_buffer_empty;
    assign inject_zneg_valid = ~zneg_inject_buffer_empty;



    buffer#(
        .buffer_depth(2560),
        .buffer_width(FLIT_SIZE)
    )xpos_inject_buffer(
        .clk(clk),
        .rst(rst),
        .in(app_xpos_inject),
        .produce(app_xpos_inject_valid),
        .consume(inject_xpos_avail),
        .full(),
        .empty(xpos_inject_buffer_empty),
        .out(inject_xpos),
        .usedw()
    );

    buffer#(
        .buffer_depth(2560),
        .buffer_width(FLIT_SIZE)
    )ypos_inject_buffer(
        .clk(clk),
        .rst(rst),
        .in(app_ypos_inject),
        .produce(app_ypos_inject_valid),
        .consume(inject_ypos_avail),
        .full(),
        .empty(ypos_inject_buffer_empty),
        .out(inject_ypos),
        .usedw()
    );
    buffer#(
        .buffer_depth(2560),
        .buffer_width(FLIT_SIZE)
    )zpos_inject_buffer(
        .clk(clk),
        .rst(rst),
        .in(app_zpos_inject),
        .produce(app_zpos_inject_valid),
        .consume(inject_zpos_avail),
        .full(),
        .empty(zpos_inject_buffer_empty),
        .out(inject_zpos),
        .usedw()
    );
    buffer#(
        .buffer_depth(2560),
        .buffer_width(FLIT_SIZE)
    )xneg_inject_buffer(
        .clk(clk),
        .rst(rst),
        .in(app_xneg_inject),
        .produce(app_xneg_inject_valid),
        .consume(inject_xneg_avail),
        .full(),
        .empty(xneg_inject_buffer_empty),
        .out(inject_xneg),
        .usedw()
    );
    buffer#(
        .buffer_depth(2560),
        .buffer_width(FLIT_SIZE)
    )yneg_inject_buffer(
        .clk(clk),
        .rst(rst),
        .in(app_yneg_inject),
        .produce(app_yneg_inject_valid),
        .consume(inject_yneg_avail),
        .full(),
        .empty(yneg_inject_buffer_empty),
        .out(inject_yneg),
        .usedw()
    );
    buffer#(
        .buffer_depth(2560),
        .buffer_width(FLIT_SIZE)
    )zneg_inject_buffer(
        .clk(clk),
        .rst(rst),
        .in(app_zneg_inject),
        .produce(app_zneg_inject_valid),
        .consume(inject_zneg_avail),
        .full(),
        .empty(zneg_inject_buffer_empty),
        .out(inject_zneg),
        .usedw()
    );


endmodule

    

	 

    
    
