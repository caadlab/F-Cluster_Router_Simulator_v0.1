`include "para.sv"
module router#(
    parameter cur_x = 0,
    parameter cur_y = 0,
    parameter cur_z = 0,
    parameter input_Q_size = 256,
    parameter credit_back_period = 100,
    parameter credit_threshold = 160
)(
    input clk,
    input rst,

    //interface from 6 MGTs
    input [FLIT_SIZE - 1 : 0] in_xpos,
    input [FLIT_SIZE - 1 : 0] in_ypos,
    input [FLIT_SIZE - 1 : 0] in_zpos,
    input [FLIT_SIZE - 1 : 0] in_xneg,
    input [FLIT_SIZE - 1 : 0] in_yneg,
    input [FLIT_SIZE - 1 : 0] in_zneg,
    input in_xpos_valid,
    input in_ypos_valid,
    input in_zpos_valid,
    input in_xneg_valid,
    input in_yneg_valid,
    input in_zneg_valid,
    //interface to 6 MGTs
    output [FLIT_SIZE - 1 : 0] out_xpos,
    output [FLIT_SIZE - 1 : 0] out_ypos,
    output [FLIT_SIZE - 1 : 0] out_zpos,
    output [FLIT_SIZE - 1 : 0] out_xneg,
    output [FLIT_SIZE - 1 : 0] out_yneg,
    output [FLIT_SIZE - 1 : 0] out_zneg,
    output out_xpos_valid,
    output out_ypos_valid,
    output out_zpos_valid,
    output out_xneg_valid,
    output out_yneg_valid,
    output out_zneg_valid,
    //interface to application kernel
    //inputs 
    output [FLIT_SIZE - 1 : 0] eject_xpos,
    output [FLIT_SIZE - 1 : 0] eject_ypos,
    output [FLIT_SIZE - 1 : 0] eject_zpos,
    output [FLIT_SIZE - 1 : 0] eject_xneg,
    output [FLIT_SIZE - 1 : 0] eject_yneg,
    output [FLIT_SIZE - 1 : 0] eject_zneg,
    output eject_xpos_valid,
    output eject_ypos_valid,
    output eject_zpos_valid,
    output eject_xneg_valid,
    output eject_yneg_valid,
    output eject_zneg_valid,

    input [FLIT_SIZE - 1 : 0] inject_xpos,
    input [FLIT_SIZE - 1 : 0] inject_ypos,
    input [FLIT_SIZE - 1 : 0] inject_zpos,
    input [FLIT_SIZE - 1 : 0] inject_xneg,
    input [FLIT_SIZE - 1 : 0] inject_yneg,
    input [FLIT_SIZE - 1 : 0] inject_zneg,
    input inject_xpos_valid,
    input inject_ypos_valid,
    input inject_zpos_valid,
    input inject_xneg_valid,
    input inject_yneg_valid,
    input inject_zneg_valid,
    output inject_xpos_avail,
    output inject_ypos_avail,
    output inject_zpos_avail,
    output inject_xneg_avail,
    output inject_yneg_avail,
    output inject_zneg_avail
);
    
    //instantiate route computation components
    //
    wire in_xpos_is_credit;
    wire in_ypos_is_credit;
    wire in_zpos_is_credit;
    wire in_xneg_is_credit;
    wire in_yneg_is_credit;
    wire in_zneg_is_credit;

    wire [FLIT_SIZE - 1 : 0] flit_xpos_VA;
    wire [FLIT_SIZE - 1 : 0] flit_ypos_VA;
    wire [FLIT_SIZE - 1 : 0] flit_zpos_VA;
    wire [FLIT_SIZE - 1 : 0] flit_xneg_VA;
    wire [FLIT_SIZE - 1 : 0] flit_yneg_VA;
    wire [FLIT_SIZE - 1 : 0] flit_zneg_VA;

    wire flit_xpos_VA_valid;
    wire flit_ypos_VA_valid;
    wire flit_zpos_VA_valid;
    wire flit_xneg_VA_valid;
    wire flit_yneg_VA_valid;
    wire flit_zneg_VA_valid;
 
    wire [ROUTE_LEN - 1 : 0] flit_xpos_VA_route;
    wire [ROUTE_LEN - 1 : 0] flit_ypos_VA_route;
    wire [ROUTE_LEN - 1 : 0] flit_zpos_VA_route;
    wire [ROUTE_LEN - 1 : 0] flit_xneg_VA_route;
    wire [ROUTE_LEN - 1 : 0] flit_yneg_VA_route;
    wire [ROUTE_LEN - 1 : 0] flit_zneg_VA_route;

    wire VA_stall_xpos;
    wire VA_stall_ypos;
    wire VA_stall_zpos;
    wire VA_stall_xneg;
    wire VA_stall_yneg;
    wire VA_stall_zneg;
 
    reg [FLIT_SIZE - 1 : 0] xpos_downstream_credits;
    reg [FLIT_SIZE - 1 : 0] ypos_downstream_credits;
    reg [FLIT_SIZE - 1 : 0] zpos_downstream_credits;
    reg [FLIT_SIZE - 1 : 0] xneg_downstream_credits;
    reg [FLIT_SIZE - 1 : 0] yneg_downstream_credits;
    reg [FLIT_SIZE - 1 : 0] zneg_downstream_credits;

    wire [FLIT_SIZE - 1 : 0] xpos_upstream_credits;
    wire [FLIT_SIZE - 1 : 0] ypos_upstream_credits;
    wire [FLIT_SIZE - 1 : 0] zpos_upstream_credits;
    wire [FLIT_SIZE - 1 : 0] xneg_upstream_credits;
    wire [FLIT_SIZE - 1 : 0] yneg_upstream_credits;
    wire [FLIT_SIZE - 1 : 0] zneg_upstream_credits;
    


    assign in_xpos_is_credit = in_xpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == CREDIT_FLIT;
    assign in_ypos_is_credit = in_ypos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == CREDIT_FLIT;
    assign in_zpos_is_credit = in_zpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == CREDIT_FLIT;
    assign in_xneg_is_credit = in_xneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == CREDIT_FLIT;
    assign in_yneg_is_credit = in_yneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == CREDIT_FLIT;
    assign in_zneg_is_credit = in_zneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == CREDIT_FLIT;

    //instantiate 6 big input buffers

    wire xpos_input_queue_empty;
    wire ypos_input_queue_empty;
    wire zpos_input_queue_empty;
    wire xneg_input_queue_empty;
    wire yneg_input_queue_empty;
    wire zneg_input_queue_empty;
 
    wire in_xpos_valid_RC;
    wire in_ypos_valid_RC;
    wire in_zpos_valid_RC;
    wire in_xneg_valid_RC;
    wire in_yneg_valid_RC;
    wire in_zneg_valid_RC;

    wire [FLIT_SIZE - 1 : 0] in_xpos_RC;
    wire [FLIT_SIZE - 1 : 0] in_ypos_RC;
    wire [FLIT_SIZE - 1 : 0] in_zpos_RC;
    wire [FLIT_SIZE - 1 : 0] in_xneg_RC;
    wire [FLIT_SIZE - 1 : 0] in_yneg_RC;
    wire [FLIT_SIZE - 1 : 0] in_zneg_RC;

    wire [FLIT_SIZE - 1 : 0] in_xpos_usedw;
    wire [FLIT_SIZE - 1 : 0] in_ypos_usedw;
    wire [FLIT_SIZE - 1 : 0] in_zpos_usedw;
    wire [FLIT_SIZE - 1 : 0] in_xneg_usedw;
    wire [FLIT_SIZE - 1 : 0] in_yneg_usedw;
    wire [FLIT_SIZE - 1 : 0] in_zneg_usedw;


    

    always@(posedge clk) begin
        if(rst) begin
            xpos_downstream_credits <= input_Q_size - 1;
            ypos_downstream_credits <= input_Q_size - 1;
            zpos_downstream_credits <= input_Q_size - 1; 
            xneg_downstream_credits <= input_Q_size - 1;
            yneg_downstream_credits <= input_Q_size - 1;
            zneg_downstream_credits <= input_Q_size - 1;
        end
        else begin
            if(in_xpos_valid && in_xpos_is_credit) begin
                xpos_downstream_credits <= in_xpos[FLIT_SIZE / 2 - 1 : 0]; //only need the payload
            end
            if(in_ypos_valid && in_ypos_is_credit) begin
                ypos_downstream_credits <= in_ypos[FLIT_SIZE / 2 - 1 : 0]; //only need the payload
            end
            if(in_zpos_valid && in_zpos_is_credit) begin
                zpos_downstream_credits <= in_zpos[FLIT_SIZE / 2 - 1 : 0]; //only need the payload
            end
            if(in_xneg_valid && in_xneg_is_credit) begin
                xneg_downstream_credits <= in_xneg[FLIT_SIZE / 2 - 1 : 0]; //only need the payload
            end
            if(in_yneg_valid && in_yneg_is_credit) begin
                yneg_downstream_credits <= in_yneg[FLIT_SIZE / 2 - 1 : 0]; //only need the payload
            end
            if(in_zneg_valid && in_zneg_is_credit) begin
                zneg_downstream_credits <= in_zneg[FLIT_SIZE / 2 - 1 : 0]; //only need the payload
            end
        end
    end









    large_buffer#(
        .buffer_depth(input_Q_size),
        .buffer_width(FLIT_SIZE)
    )
    xpos_input_queue(
        .clk(clk),
        .rst(rst),
        .in(in_xpos),
        .produce(in_xpos_valid && (~in_xpos_is_credit)),
        .consume(eject_xpos_valid || ~VA_stall_xpos),
        .full(),
        .empty(xpos_input_queue_empty),
        .out(in_xpos_RC),
        .usedw(in_xpos_usedw)
    );

    assign in_xpos_valid_RC = ~xpos_input_queue_empty;

    large_buffer#(
        .buffer_depth(input_Q_size),
        .buffer_width(FLIT_SIZE)
    )
    ypos_input_queue(
        .clk(clk),
        .rst(rst),
        .in(in_ypos),
        .produce(in_ypos_valid && (~in_ypos_is_credit)),
        .consume(eject_ypos_valid || ~VA_stall_ypos),
        .full(),
        .empty(ypos_input_queue_empty),
        .out(in_ypos_RC),
        .usedw(in_ypos_usedw)
    );

    assign in_ypos_valid_RC = ~ypos_input_queue_empty;

    large_buffer#(
        .buffer_depth(input_Q_size),
        .buffer_width(FLIT_SIZE)
    )
    zpos_input_queue(
        .clk(clk),
        .rst(rst),
        .in(in_zpos),
        .produce(in_zpos_valid && (~in_zpos_is_credit)),
        .consume(eject_zpos_valid || ~VA_stall_zpos),
        .full(),
        .empty(zpos_input_queue_empty),
        .out(in_zpos_RC),
        .usedw(in_zpos_usedw)
    );

    assign in_zpos_valid_RC = ~zpos_input_queue_empty;
 
    large_buffer#(
        .buffer_depth(input_Q_size),
        .buffer_width(FLIT_SIZE)
    )
    xneg_input_queue(
        .clk(clk),
        .rst(rst),
        .in(in_xneg),
        .produce(in_xneg_valid && (~in_xneg_is_credit)),
        .consume(eject_xneg_valid || ~VA_stall_xneg),
        .full(),
        .empty(xneg_input_queue_empty),
        .out(in_xneg_RC),
        .usedw(in_xneg_usedw)
    );

    assign in_xneg_valid_RC = ~xneg_input_queue_empty;

    large_buffer#(
        .buffer_depth(input_Q_size),
        .buffer_width(FLIT_SIZE)
    )
    yneg_input_queue(
        .clk(clk),
        .rst(rst),
        .in(in_yneg),
        .produce(in_yneg_valid && (~in_yneg_is_credit)),
        .consume(eject_yneg_valid || ~VA_stall_yneg),
        .full(),
        .empty(yneg_input_queue_empty),
        .out(in_yneg_RC),
        .usedw(in_yneg_usedw)
    );

    assign in_yneg_valid_RC = ~yneg_input_queue_empty;

    large_buffer#(
        .buffer_depth(input_Q_size),
        .buffer_width(FLIT_SIZE)
    )
    zneg_input_queue(
        .clk(clk),
        .rst(rst),
        .in(in_zneg),
        .produce(in_zneg_valid && (~in_zneg_is_credit)),
        .consume(eject_zneg_valid || ~VA_stall_zneg),
        .full(),
        .empty(zneg_input_queue_empty),
        .out(in_zneg_RC),
        .usedw(in_zneg_usedw)
    );

    assign in_zneg_valid_RC = ~zneg_input_queue_empty;

    assign xpos_upstream_credits = input_Q_size - 1 - in_xpos_usedw;
    assign ypos_upstream_credits = input_Q_size - 1 - in_ypos_usedw;
    assign zpos_upstream_credits = input_Q_size - 1 - in_zpos_usedw;
    assign xneg_upstream_credits = input_Q_size - 1 - in_xneg_usedw;
    assign yneg_upstream_credits = input_Q_size - 1 - in_yneg_usedw;
    assign zneg_upstream_credits = input_Q_size - 1 - in_zneg_usedw;


    

    route_comp#(
        .cur_x(cur_x),
        .cur_y(cur_y),
        .cur_z(cur_z)
    )xpos_route_comp(
        .clk(clk),
        .rst(rst),
        .flit_valid_in(in_xpos_valid_RC),
        .flit_before_RC(in_xpos_RC),
        .stall(VA_stall_xpos),
        .dir_in(DIR_XPOS),
        .flit_after_RC(flit_xpos_VA),
        .flit_valid_out(flit_xpos_VA_valid),
        .dir_out(flit_xpos_VA_route),
        .eject_enable(eject_xpos_valid)
    );
    assign eject_xpos = flit_xpos_VA;

    route_comp#(
        .cur_x(cur_x),
        .cur_y(cur_y),
        .cur_z(cur_z)
    )ypos_route_comp(
        .clk(clk),
        .rst(rst),
        .flit_valid_in(in_ypos_valid_RC),
        .flit_before_RC(in_ypos_RC),
        .stall(VA_stall_ypos),
        .dir_in(DIR_YPOS),
        .flit_after_RC(flit_ypos_VA),
        .flit_valid_out(flit_ypos_VA_valid),
        .dir_out(flit_ypos_VA_route),
        .eject_enable(eject_ypos_valid)
    ); 
    assign eject_ypos = flit_ypos_VA;

    route_comp#(
        .cur_x(cur_x),
        .cur_y(cur_y),
        .cur_z(cur_z)
    )zpos_route_comp(
        .clk(clk),
        .rst(rst),
        .flit_valid_in(in_zpos_valid_RC),
        .flit_before_RC(in_zpos_RC),
        .stall(VA_stall_zpos),
        .dir_in(DIR_ZPOS),
        .flit_after_RC(flit_zpos_VA),
        .flit_valid_out(flit_zpos_VA_valid),
        .dir_out(flit_zpos_VA_route),
        .eject_enable(eject_zpos_valid)
    );
    assign eject_zpos = flit_zpos_VA;

    route_comp#(
        .cur_x(cur_x),
        .cur_y(cur_y),
        .cur_z(cur_z)
    )xneg_route_comp(
        .clk(clk),
        .rst(rst),
        .flit_valid_in(in_xneg_valid_RC),
        .flit_before_RC(in_xneg_RC),
        .stall(VA_stall_xneg),
        .dir_in(DIR_XNEG),
        .flit_after_RC(flit_xneg_VA),
        .flit_valid_out(flit_xneg_VA_valid),
        .dir_out(flit_xneg_VA_route),
        .eject_enable(eject_xneg_valid)
    );
    assign eject_xneg = flit_xneg_VA;

    route_comp#(
        .cur_x(cur_x),
        .cur_y(cur_y),
        .cur_z(cur_z)
    )yneg_route_comp(
        .clk(clk),
        .rst(rst),
        .flit_valid_in(in_yneg_valid_RC),
        .flit_before_RC(in_yneg_RC),
        .stall(VA_stall_yneg),
        .dir_in(DIR_YNEG),
        .flit_after_RC(flit_yneg_VA),
        .flit_valid_out(flit_yneg_VA_valid),
        .dir_out(flit_yneg_VA_route),
        .eject_enable(eject_yneg_valid)
    );
    assign eject_yneg = flit_yneg_VA;

    route_comp#(
        .cur_x(cur_x),
        .cur_y(cur_y),
        .cur_z(cur_z)
    )zneg_route_comp(
        .clk(clk),
        .rst(rst),
        .flit_valid_in(in_zneg_valid_RC),
        .flit_before_RC(in_zneg_RC),
        .stall(VA_stall_zneg),
        .dir_in(DIR_ZNEG),
        .flit_after_RC(flit_zneg_VA),
        .flit_valid_out(flit_zneg_VA_valid),
        .dir_out(flit_zneg_VA_route),
        .eject_enable(eject_zneg_valid)
    );
    assign eject_zneg = flit_zneg_VA;


    wire [VC_NUM - 1 : 0] VC_full_xpos;
    wire [VC_NUM - 1 : 0] VC_full_ypos;
    wire [VC_NUM - 1 : 0] VC_full_zpos;
    wire [VC_NUM - 1 : 0] VC_full_xneg;
    wire [VC_NUM - 1 : 0] VC_full_yneg;
    wire [VC_NUM - 1 : 0] VC_full_zneg;

    wire [VC_NUM - 1 : 0] VC_idle_xpos;
    wire [VC_NUM - 1 : 0] VC_idle_ypos;
    wire [VC_NUM - 1 : 0] VC_idle_zpos;
    wire [VC_NUM - 1 : 0] VC_idle_xneg;
    wire [VC_NUM - 1 : 0] VC_idle_yneg;
    wire [VC_NUM - 1 : 0] VC_idle_zneg;

    wire [VC_NUM - 1 : 0] VC_grant_xpos;
    wire [VC_NUM - 1 : 0] VC_grant_ypos;
    wire [VC_NUM - 1 : 0] VC_grant_zpos;
    wire [VC_NUM - 1 : 0] VC_grant_xneg;
    wire [VC_NUM - 1 : 0] VC_grant_yneg;
    wire [VC_NUM - 1 : 0] VC_grant_zneg;







    //instantiate VC allocators
    VC_allocator  VC_allocator_inst(
        .clk(clk),
        .rst(rst),
        .full_xpos(VC_full_xpos),
        .full_ypos(VC_full_ypos),
        .full_zpos(VC_full_zpos),
        .full_xneg(VC_full_xneg),
        .full_yneg(VC_full_yneg),
        .full_zneg(VC_full_zneg),
        .idle_xpos(VC_idle_xpos),
        .idle_ypos(VC_idle_ypos),
        .idle_zpos(VC_idle_zpos),
        .idle_xneg(VC_idle_xneg),
        .idle_yneg(VC_idle_yneg),
        .idle_zneg(VC_idle_zneg),
        .port_valid({flit_zneg_VA_valid, flit_yneg_VA_valid, flit_xneg_VA_valid, flit_zpos_VA_valid, flit_ypos_VA_valid, flit_xpos_VA_valid}), //xpos 0th bit, ypos 1st bit, zpos 2nd bit, xneg 3rd bit, yneg 4th bit, zneg 5th bit
        .route_in_xpos(flit_xpos_VA_route),
        .route_in_ypos(flit_ypos_VA_route),
        .route_in_zpos(flit_zpos_VA_route),
        .route_in_xneg(flit_xneg_VA_route),
        .route_in_yneg(flit_yneg_VA_route),
        .route_in_zneg(flit_zneg_VA_route),
        .flit_xpos(flit_xpos_VA),
        .flit_ypos(flit_ypos_VA),
        .flit_zpos(flit_zpos_VA),
        .flit_xneg(flit_xneg_VA),
        .flit_yneg(flit_yneg_VA),
        .flit_zneg(flit_zneg_VA),
        .grant_xpos(VC_grant_xpos), // a one-hot coding grant
        .grant_ypos(VC_grant_ypos),
        .grant_zpos(VC_grant_zpos),
        .grant_xneg(VC_grant_xneg),
        .grant_yneg(VC_grant_yneg),
        .grant_zneg(VC_grant_zneg),
        .stall_xpos(VA_stall_xpos),
        .stall_ypos(VA_stall_ypos),
        .stall_zpos(VA_stall_zpos),
        .stall_xneg(VA_stall_xneg),
        .stall_yneg(VA_stall_yneg),
        .stall_zneg(VA_stall_zneg)
    );

    wire [VC_NUM * FLIT_SIZE - 1 : 0] flit_xpos_SA;
    wire [VC_NUM * FLIT_SIZE - 1 : 0] flit_ypos_SA;
    wire [VC_NUM * FLIT_SIZE - 1 : 0] flit_zpos_SA;
    wire [VC_NUM * FLIT_SIZE - 1 : 0] flit_xneg_SA;
    wire [VC_NUM * FLIT_SIZE - 1 : 0] flit_yneg_SA;
    wire [VC_NUM * FLIT_SIZE - 1 : 0] flit_zneg_SA;

    wire [VC_NUM - 1 : 0] flit_xpos_SA_valid;
    wire [VC_NUM - 1 : 0] flit_ypos_SA_valid;
    wire [VC_NUM - 1 : 0] flit_zpos_SA_valid;
    wire [VC_NUM - 1 : 0] flit_xneg_SA_valid;
    wire [VC_NUM - 1 : 0] flit_yneg_SA_valid;
    wire [VC_NUM - 1 : 0] flit_zneg_SA_valid;

    wire [VC_NUM - 1 : 0] flit_xpos_SA_grant;
    wire [VC_NUM - 1 : 0] flit_ypos_SA_grant;
    wire [VC_NUM - 1 : 0] flit_zpos_SA_grant;
    wire [VC_NUM - 1 : 0] flit_xneg_SA_grant;
    wire [VC_NUM - 1 : 0] flit_yneg_SA_grant;
    wire [VC_NUM - 1 : 0] flit_zneg_SA_grant;

    wire [VC_NUM * ROUTE_LEN - 1 : 0] flit_xpos_SA_route;
    wire [VC_NUM * ROUTE_LEN - 1 : 0] flit_ypos_SA_route;
    wire [VC_NUM * ROUTE_LEN - 1 : 0] flit_zpos_SA_route;
    wire [VC_NUM * ROUTE_LEN - 1 : 0] flit_xneg_SA_route;
    wire [VC_NUM * ROUTE_LEN - 1 : 0] flit_yneg_SA_route;
    wire [VC_NUM * ROUTE_LEN - 1 : 0] flit_zneg_SA_route;


    //instantiate VCs
    // xpos VCs
    genvar i;
    generate 
        for(i = 0; i < VC_NUM; i = i + 1) begin: xpos_VCs
            VC VC_inst(
                .clk(clk),
                .rst(rst),
                .G(),
                .R(flit_xpos_SA_route[i * ROUTE_LEN + ROUTE_LEN - 1 : i * ROUTE_LEN]),
                .O(),
                .P(),
                .vc_full(VC_full_xpos[i]),
                .C(flit_xpos_SA_grant[i]),
                .flit_in(flit_xpos_VA),
                .valid_in(VC_grant_xpos[i] && flit_xpos_VA_valid),
                .route_in(flit_xpos_VA_route),
                .grant(flit_xpos_SA_grant[i]),
                .flit_out(flit_xpos_SA[i * FLIT_SIZE + FLIT_SIZE - 1 : i * FLIT_SIZE]),
                .valid_out(flit_xpos_SA_valid[i]),
                .vc_idle(VC_idle_xpos[i])
            );
        end
    endgenerate
    

    //ypos VCs
    generate 
        for(i = 0; i < VC_NUM; i = i + 1) begin: ypos_VCs
            VC VC_inst(
                .clk(clk),
                .rst(rst),
                .G(),
                .R(flit_ypos_SA_route[i * ROUTE_LEN + ROUTE_LEN - 1 : i * ROUTE_LEN]),
                .O(),
                .P(),
                .vc_full(VC_full_ypos[i]),
                .C(flit_ypos_SA_grant[i]),
                .flit_in(flit_ypos_VA),
                .valid_in(VC_grant_ypos[i] && flit_ypos_VA_valid),
                .route_in(flit_ypos_VA_route),
                .grant(flit_ypos_SA_grant[i]),
                .flit_out(flit_ypos_SA[i * FLIT_SIZE + FLIT_SIZE - 1 : i * FLIT_SIZE]),
                .valid_out(flit_ypos_SA_valid[i]),
                .vc_idle(VC_idle_ypos[i])

            );
        end
    endgenerate    
    
   //zpos VCs 
    generate 
        for(i = 0; i < VC_NUM; i = i + 1) begin: zpos_VCs
            VC VC_inst(
                .clk(clk),
                .rst(rst),
                .G(),
                .R(flit_zpos_SA_route[i * ROUTE_LEN + ROUTE_LEN - 1 : i * ROUTE_LEN]),
                .O(),
                .P(),
                .vc_full(VC_full_zpos[i]),
                .C(flit_zpos_SA_grant[i]),
                .flit_in(flit_zpos_VA),
                .valid_in(VC_grant_zpos[i] && flit_zpos_VA_valid),
                .route_in(flit_zpos_VA_route),
                .grant(flit_zpos_SA_grant[i]),
                .flit_out(flit_zpos_SA[i * FLIT_SIZE + FLIT_SIZE - 1 : i * FLIT_SIZE]),
                .valid_out(flit_zpos_SA_valid[i]),
                .vc_idle(VC_idle_zpos[i])

            );
        end
    endgenerate

    generate 
        for(i = 0; i < VC_NUM; i = i + 1) begin: xneg_VCs
            VC VC_inst(
                .clk(clk),
                .rst(rst),
                .G(),
                .R(flit_xneg_SA_route[i * ROUTE_LEN + ROUTE_LEN - 1 : i * ROUTE_LEN]),
                .O(),
                .P(),
                .vc_full(VC_full_xneg[i]),
                .C(flit_xneg_SA_grant[i]),
                .flit_in(flit_xneg_VA),
                .valid_in(VC_grant_xneg[i] && flit_xneg_VA_valid),
                .route_in(flit_xneg_VA_route),
                .grant(flit_xneg_SA_grant[i]),
                .flit_out(flit_xneg_SA[i * FLIT_SIZE + FLIT_SIZE - 1 : i * FLIT_SIZE]),
                .valid_out(flit_xneg_SA_valid[i]),
                .vc_idle(VC_idle_xneg[i])

            );
        end
    endgenerate
 
    generate 
        for(i = 0; i < VC_NUM; i = i + 1) begin: yneg_VCs
            VC VC_inst(
                .clk(clk),
                .rst(rst),
                .G(),
                .R(flit_yneg_SA_route[i * ROUTE_LEN + ROUTE_LEN - 1 : i * ROUTE_LEN]),
                .O(),
                .P(),
                .vc_full(VC_full_yneg[i]),
                .C(flit_yneg_SA_grant[i]),
                .flit_in(flit_yneg_VA),
                .valid_in(VC_grant_yneg[i] && flit_yneg_VA_valid),
                .route_in(flit_yneg_VA_route),
                .grant(flit_yneg_SA_grant[i]),
                .flit_out(flit_yneg_SA[i * FLIT_SIZE + FLIT_SIZE - 1 : i * FLIT_SIZE]),
                .valid_out(flit_yneg_SA_valid[i]),
                .vc_idle(VC_idle_yneg[i])

            );
        end
    endgenerate 
    
    generate 
        for(i = 0; i < VC_NUM; i = i + 1) begin: zneg_VCs
            VC VC_inst(
                .clk(clk),
                .rst(rst),
                .G(),
                .R(flit_zneg_SA_route[i * ROUTE_LEN + ROUTE_LEN - 1 : i * ROUTE_LEN]),
                .O(),
                .P(),
                .vc_full(VC_full_zneg[i]),
                .C(flit_zneg_SA_grant[i]),
                .flit_in(flit_zneg_VA),
                .valid_in(VC_grant_zneg[i] && flit_zneg_VA_valid),
                .route_in(flit_zneg_VA_route),
                .grant(flit_zneg_SA_grant[i]),
                .flit_out(flit_zneg_SA[i * FLIT_SIZE + FLIT_SIZE - 1 : i * FLIT_SIZE]),
                .valid_out(flit_zneg_SA_valid[i]),
                .vc_idle(VC_idle_zneg[i])

            );
        end
    endgenerate

//instantiate switch

    wire [PORT_NUM - 1 : 0] flit_valid_ST;
    wire [FLIT_SIZE - 1 : 0] flit_xpos_ST;
    wire [FLIT_SIZE - 1 : 0] flit_ypos_ST;
    wire [FLIT_SIZE - 1 : 0] flit_zpos_ST;
    wire [FLIT_SIZE - 1 : 0] flit_xneg_ST;
    wire [FLIT_SIZE - 1 : 0] flit_yneg_ST;
    wire [FLIT_SIZE - 1 : 0] flit_zneg_ST;

    wire xpos_downstream_avail;
    wire ypos_downstream_avail;
    wire zpos_downstream_avail;
    wire xneg_downstream_avail;
    wire yneg_downstream_avail;
    wire zneg_downstream_avail;

    wire zneg_avail_ST;
    wire yneg_avail_ST;
    wire xneg_avail_ST;
    wire zpos_avail_ST;
    wire ypos_avail_ST;
    wire xpos_avail_ST;

    wire [PORT_NUM -1 : 0] out_avail_ST;
    wire [PORT_NUM * FLIT_SIZE - 1 : 0] out_ST;

    wire [FLIT_SIZE - 1 : 0] zneg_out_ST;
    wire [FLIT_SIZE - 1 : 0] yneg_out_ST;
    wire [FLIT_SIZE - 1 : 0] xneg_out_ST;
    wire [FLIT_SIZE - 1 : 0] zpos_out_ST;
    wire [FLIT_SIZE - 1 : 0] ypos_out_ST;
    wire [FLIT_SIZE - 1 : 0] xpos_out_ST;

    assign xpos_out_ST = out_ST[FLIT_SIZE - 1 : 0];
    assign ypos_out_ST = out_ST[2 * FLIT_SIZE - 1 : FLIT_SIZE];
    assign zpos_out_ST = out_ST[3 * FLIT_SIZE - 1 : 2 * FLIT_SIZE];
    assign xneg_out_ST = out_ST[4 * FLIT_SIZE - 1 : 3 * FLIT_SIZE];
    assign yneg_out_ST = out_ST[5 * FLIT_SIZE - 1 : 4 * FLIT_SIZE];
    assign zneg_out_ST = out_ST[6 * FLIT_SIZE - 1 : 5 * FLIT_SIZE];



    assign  xpos_downstream_avail = (xpos_downstream_credits >= credit_threshold);
    assign  ypos_downstream_avail = (ypos_downstream_credits >= credit_threshold);
    assign  zpos_downstream_avail = (zpos_downstream_credits >= credit_threshold);
    assign  xneg_downstream_avail = (xneg_downstream_credits >= credit_threshold);
    assign  yneg_downstream_avail = (yneg_downstream_credits >= credit_threshold);
    assign  zneg_downstream_avail = (zneg_downstream_credits >= credit_threshold);


    switch#(
        .M_IN(VC_NUM * PORT_NUM),
        .N_OUT(PORT_NUM)
    )sw_inst(
        .clk(clk),
        .rst(rst),
        .in({flit_zneg_SA, flit_yneg_SA, flit_xneg_SA, flit_zpos_SA, flit_ypos_SA, flit_xpos_SA}),
        .route_in({flit_zneg_SA_route, flit_yneg_SA_route, flit_xneg_SA_route, flit_zpos_SA_route, flit_ypos_SA_route, flit_xpos_SA_route}),
        .in_valid({flit_zneg_SA_valid, flit_yneg_SA_valid, flit_xneg_SA_valid, flit_zpos_SA_valid, flit_ypos_SA_valid, flit_xpos_SA_valid}),
        .in_avail({flit_zneg_SA_grant, flit_yneg_SA_grant, flit_xneg_SA_grant, flit_zpos_SA_grant, flit_ypos_SA_grant, flit_xpos_SA_grant}),
        
        .out_valid(flit_valid_ST),
        .out_avail({zneg_avail_ST, yneg_avail_ST, xneg_avail_ST, zpos_avail_ST, ypos_avail_ST, xpos_avail_ST}),
        .out(out_ST)
    );

    reg [FLIT_SIZE - 1 : 0] credit_period_counter;

    always@(posedge clk) begin
        if(rst) begin
            credit_period_counter <= 0;
        end
        else begin
            credit_period_counter <= (credit_period_counter < credit_back_period ? credit_period_counter + 1 : 0);
        end
    end
    
    wire ST_or_inject_xpos;
    wire ST_or_inject_ypos;
    wire ST_or_inject_zpos;
    wire ST_or_inject_xneg;
    wire ST_or_inject_yneg;
    wire ST_or_inject_zneg;
 
    reg xpos_occupy_by_thru; //the xpos is occupyied by the thru traffic
    reg ypos_occupy_by_thru;
    reg zpos_occupy_by_thru;
    reg xneg_occupy_by_thru;
    reg yneg_occupy_by_thru;
    reg zneg_occupy_by_thru;

    reg xpos_occupy_by_inject;
    reg ypos_occupy_by_inject;
    reg zpos_occupy_by_inject;
    reg xneg_occupy_by_inject;
    reg yneg_occupy_by_inject;
    reg zneg_occupy_by_inject;


    assign zneg_avail_ST = zneg_downstream_avail && (credit_period_counter != credit_back_period - 1) && (~zneg_occupy_by_inject);
    assign yneg_avail_ST = yneg_downstream_avail && (credit_period_counter != credit_back_period - 1) && (~yneg_occupy_by_inject);
    assign xneg_avail_ST = xneg_downstream_avail && (credit_period_counter != credit_back_period - 1) && (~xneg_occupy_by_inject);
    assign zpos_avail_ST = zpos_downstream_avail && (credit_period_counter != credit_back_period - 1) && (~zpos_occupy_by_inject);
    assign ypos_avail_ST = ypos_downstream_avail && (credit_period_counter != credit_back_period - 1) && (~ypos_occupy_by_inject);
    assign xpos_avail_ST = xpos_downstream_avail && (credit_period_counter != credit_back_period - 1) && (~xpos_occupy_by_inject);

   // the pass through traffic has higher priority than injection traffic. But if injection ports can only be preempted when it is not in the middle of injecting of one packet. 





    assign ST_or_inject_xpos = flit_valid_ST[0] && (~xpos_occupy_by_inject);
    assign ST_or_inject_ypos = flit_valid_ST[1] && (~ypos_occupy_by_inject);
    assign ST_or_inject_zpos = flit_valid_ST[2] && (~zpos_occupy_by_inject);
    assign ST_or_inject_xneg = flit_valid_ST[3] && (~xneg_occupy_by_inject);
    assign ST_or_inject_yneg = flit_valid_ST[4] && (~yneg_occupy_by_inject);
    assign ST_or_inject_zneg = flit_valid_ST[5] && (~zneg_occupy_by_inject);
        
    always@(posedge clk) begin
        if(rst) begin
            xpos_occupy_by_inject <= 0;

        end
        else if(inject_xpos_valid && ~flit_valid_ST[0] && inject_xpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) begin
            xpos_occupy_by_inject <= 1;
        end
        else if(inject_xpos_valid && inject_xpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT) begin
            xpos_occupy_by_inject <= 0;
        end
        
    end 
    
    always@(posedge clk) begin
        if(rst) begin
            ypos_occupy_by_inject <= 0;

        end
        else if(inject_ypos_valid && ~flit_valid_ST[1] && inject_ypos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) begin
            ypos_occupy_by_inject <= 1;
        end
        else if(inject_ypos_valid && inject_ypos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT) begin
            ypos_occupy_by_inject <= 0;
        end
        
    end
 
    always@(posedge clk) begin
        if(rst) begin
            zpos_occupy_by_inject <= 0;

        end
        else if(inject_zpos_valid && ~flit_valid_ST[2] && inject_zpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) begin
            zpos_occupy_by_inject <= 1;
        end
        else if(inject_zpos_valid && inject_zpos[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT) begin
            zpos_occupy_by_inject <= 0;
        end
        
    end
 
    always@(posedge clk) begin
        if(rst) begin
            xneg_occupy_by_inject <= 0;

        end
        else if(inject_xneg_valid && ~flit_valid_ST[3] && inject_xneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) begin
            xneg_occupy_by_inject <= 1;
        end
        else if(inject_xneg_valid && inject_xneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT) begin
            xneg_occupy_by_inject <= 0;
        end
        
    end
 
    always@(posedge clk) begin
        if(rst) begin
            yneg_occupy_by_inject <= 0;

        end
        else if(inject_yneg_valid && ~flit_valid_ST[4] && inject_yneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) begin
            yneg_occupy_by_inject <= 1;
        end
        else if(inject_yneg_valid && inject_yneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT) begin
            yneg_occupy_by_inject <= 0;
        end
        
    end
 
    always@(posedge clk) begin
        if(rst) begin
            zneg_occupy_by_inject <= 0;

        end
        else if(inject_zneg_valid && ~flit_valid_ST[5] && inject_zneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == HEAD_FLIT) begin
            zneg_occupy_by_inject <= 1;
        end
        else if(inject_zneg_valid && inject_zneg[FLIT_SIZE - 1 : FLIT_SIZE - HEADER_LEN] == TAIL_FLIT) begin
            zneg_occupy_by_inject <= 0;
        end
        
    end


    assign out_xpos_valid = (credit_period_counter == credit_back_period - 1) || flit_valid_ST[0] || inject_xpos_valid;
    assign out_ypos_valid = (credit_period_counter == credit_back_period - 1) || flit_valid_ST[1] || inject_ypos_valid;
    assign out_zpos_valid = (credit_period_counter == credit_back_period - 1) || flit_valid_ST[2] || inject_zpos_valid;
    assign out_xneg_valid = (credit_period_counter == credit_back_period - 1) || flit_valid_ST[3] || inject_xneg_valid;
    assign out_yneg_valid = (credit_period_counter == credit_back_period - 1) || flit_valid_ST[4] || inject_yneg_valid;
    assign out_zneg_valid = (credit_period_counter == credit_back_period - 1) || flit_valid_ST[5] || inject_zneg_valid;



    assign out_xpos = (credit_period_counter == credit_back_period - 1) ? {CREDIT_FLIT, xpos_upstream_credits[FLIT_SIZE - HEADER_LEN - 1 : 0]} : (ST_or_inject_xpos ? xpos_out_ST : inject_xpos);
    assign out_ypos = (credit_period_counter == credit_back_period - 1) ? {CREDIT_FLIT, ypos_upstream_credits[FLIT_SIZE - HEADER_LEN - 1 : 0]} : (ST_or_inject_ypos ? ypos_out_ST : inject_ypos);
    assign out_zpos = (credit_period_counter == credit_back_period - 1) ? {CREDIT_FLIT, zpos_upstream_credits[FLIT_SIZE - HEADER_LEN - 1 : 0]} : (ST_or_inject_zpos ? zpos_out_ST : inject_zpos);
    assign out_xneg = (credit_period_counter == credit_back_period - 1) ? {CREDIT_FLIT, xneg_upstream_credits[FLIT_SIZE - HEADER_LEN - 1 : 0]} : (ST_or_inject_xneg ? xneg_out_ST : inject_xneg);
    assign out_yneg = (credit_period_counter == credit_back_period - 1) ? {CREDIT_FLIT, yneg_upstream_credits[FLIT_SIZE - HEADER_LEN - 1 : 0]} : (ST_or_inject_yneg ? yneg_out_ST : inject_yneg);
    assign out_zneg = (credit_period_counter == credit_back_period - 1) ? {CREDIT_FLIT, zneg_upstream_credits[FLIT_SIZE - HEADER_LEN - 1 : 0]} : (ST_or_inject_zneg ? zneg_out_ST : inject_zneg);

    
    assign inject_xpos_avail = xpos_downstream_avail && (credit_period_counter != credit_back_period - 1) && (xpos_occupy_by_inject || (~flit_valid_ST[0]));
    assign inject_ypos_avail = ypos_downstream_avail && (credit_period_counter != credit_back_period - 1) && (ypos_occupy_by_inject || (~flit_valid_ST[1]));
    assign inject_zpos_avail = zpos_downstream_avail && (credit_period_counter != credit_back_period - 1) && (zpos_occupy_by_inject || (~flit_valid_ST[2]));
    assign inject_xneg_avail = xneg_downstream_avail && (credit_period_counter != credit_back_period - 1) && (xneg_occupy_by_inject || (~flit_valid_ST[3]));
    assign inject_yneg_avail = yneg_downstream_avail && (credit_period_counter != credit_back_period - 1) && (yneg_occupy_by_inject || (~flit_valid_ST[4]));
    assign inject_zneg_avail = zneg_downstream_avail && (credit_period_counter != credit_back_period - 1) && (zneg_occupy_by_inject || (~flit_valid_ST[5]));



    
    
endmodule
    
    

    

    
