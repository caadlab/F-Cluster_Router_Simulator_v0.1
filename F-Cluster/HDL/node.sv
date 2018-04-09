`include "para.sv"

module node
#(
    parameter cur_x = 0,
    parameter cur_y = 0,
    parameter cur_z = 0
)(
	input clk,
    input rst,
	input [FLIT_SIZE : 0] in_xpos_ser,	
    output [FLIT_SIZE : 0] out_xpos_ser,
    
    input [FLIT_SIZE : 0] in_xneg_ser,	
    output [FLIT_SIZE : 0] out_xneg_ser,
	
    input [FLIT_SIZE : 0] in_ypos_ser,	
    output [FLIT_SIZE : 0] out_ypos_ser,
    
    input [FLIT_SIZE : 0] in_yneg_ser,	
    output [FLIT_SIZE : 0] out_yneg_ser,
 
    input [FLIT_SIZE : 0] in_zpos_ser,	
    output [FLIT_SIZE : 0] out_zpos_ser,
    
    input [FLIT_SIZE : 0] in_zneg_ser,	
    output [FLIT_SIZE : 0] out_zneg_ser
);


    wire [FLIT_SIZE - 1 : 0] in_xpos;
    wire [FLIT_SIZE - 1 : 0] in_ypos;
    wire [FLIT_SIZE - 1 : 0] in_zpos;
    wire [FLIT_SIZE - 1 : 0] in_xneg;
    wire [FLIT_SIZE - 1 : 0] in_yneg;
    wire [FLIT_SIZE - 1 : 0] in_zneg;
    
    wire in_xpos_valid;
    wire in_ypos_valid;
    wire in_zpos_valid;
    wire in_xneg_valid;
    wire in_yneg_valid;
    wire in_zneg_valid;

    wire [FLIT_SIZE - 1 : 0] out_xpos;
    wire [FLIT_SIZE - 1 : 0] out_ypos;
    wire [FLIT_SIZE - 1 : 0] out_zpos;
    wire [FLIT_SIZE - 1 : 0] out_xneg;
    wire [FLIT_SIZE - 1 : 0] out_yneg;
    wire [FLIT_SIZE - 1 : 0] out_zneg;
    
    wire out_xpos_valid;
    wire out_ypos_valid;
    wire out_zpos_valid;
    wire out_xneg_valid;
    wire out_yneg_valid;
    wire out_zneg_valid;

    wire [FLIT_SIZE - 1 : 0] inject_xpos;
    wire [FLIT_SIZE - 1 : 0] inject_ypos;
    wire [FLIT_SIZE - 1 : 0] inject_zpos;
    wire [FLIT_SIZE - 1 : 0] inject_xneg;
    wire [FLIT_SIZE - 1 : 0] inject_yneg;
    wire [FLIT_SIZE - 1 : 0] inject_zneg;
    
    wire inject_xpos_valid;
    wire inject_ypos_valid;
    wire inject_zpos_valid;
    wire inject_xneg_valid;
    wire inject_yneg_valid;
    wire inject_zneg_valid;

    wire inject_xpos_avail;
    wire inject_ypos_avail;
    wire inject_zpos_avail;
    wire inject_xneg_avail;
    wire inject_yneg_avail;
    wire inject_zneg_avail;
 
    wire [FLIT_SIZE - 1 : 0] eject_xpos;
    wire [FLIT_SIZE - 1 : 0] eject_ypos;
    wire [FLIT_SIZE - 1 : 0] eject_zpos;
    wire [FLIT_SIZE - 1 : 0] eject_xneg;
    wire [FLIT_SIZE - 1 : 0] eject_yneg;
    wire [FLIT_SIZE - 1 : 0] eject_zneg;
    
    wire eject_xpos_valid;
    wire eject_ypos_valid;
    wire eject_zpos_valid;
    wire eject_xneg_valid;
    wire eject_yneg_valid;
    wire eject_zneg_valid;




    
    router#(
        .cur_x(cur_x),
        .cur_y(cur_y),
        .cur_z(cur_z),
        .input_Q_size(256),
        .credit_back_period(100),
        .credit_threshold(160)
    )
    switch_inst(
        .clk(clk),
        .rst(rst),

    //interface from 6 MGTs
        .in_xpos(in_xpos),
        .in_ypos(in_ypos),
        .in_zpos(in_zpos),
        .in_xneg(in_xneg),
        .in_yneg(in_yneg),
        .in_zneg(in_zneg),
        .in_xpos_valid(in_xpos_valid),
        .in_ypos_valid(in_ypos_valid),
        .in_zpos_valid(in_zpos_valid),
        .in_xneg_valid(in_xneg_valid),
        .in_yneg_valid(in_yneg_valid),
        .in_zneg_valid(in_zneg_valid),
    //interface to 6 MGTs
        .out_xpos(out_xpos),
        .out_ypos(out_ypos),
        .out_zpos(out_zpos),
        .out_xneg(out_xneg),
        .out_yneg(out_yneg),
        .out_zneg(out_zneg),
        .out_xpos_valid(out_xpos_valid),
        .out_ypos_valid(out_ypos_valid),
        .out_zpos_valid(out_zpos_valid),
        .out_xneg_valid(out_xneg_valid),
        .out_yneg_valid(out_yneg_valid),
        .out_zneg_valid(out_zneg_valid),
    //interface to application kernel
    //inputs 
        .eject_xpos(eject_xpos),
        .eject_ypos(eject_ypos),
        .eject_zpos(eject_zpos),
        .eject_xneg(eject_xneg),
        .eject_yneg(eject_yneg),
        .eject_zneg(eject_zneg),
        .eject_xpos_valid(eject_xpos_valid),
        .eject_ypos_valid(eject_ypos_valid),
        .eject_zpos_valid(eject_zpos_valid),
        .eject_xneg_valid(eject_xneg_valid),
        .eject_yneg_valid(eject_yneg_valid),
        .eject_zneg_valid(eject_zneg_valid),

        .inject_xpos(inject_xpos),
        .inject_ypos(inject_ypos),
        .inject_zpos(inject_zpos),
        .inject_xneg(inject_xneg),
        .inject_yneg(inject_yneg),
        .inject_zneg(inject_zneg),
        .inject_xpos_valid(inject_xpos_valid),
        .inject_ypos_valid(inject_ypos_valid),
        .inject_zpos_valid(inject_zpos_valid),
        .inject_xneg_valid(inject_xneg_valid),
        .inject_yneg_valid(inject_yneg_valid),
        .inject_zneg_valid(inject_zneg_valid),
        .inject_xpos_avail(inject_xpos_avail),
        .inject_ypos_avail(inject_ypos_avail),
        .inject_zpos_avail(inject_zpos_avail),
        .inject_xneg_avail(inject_xneg_avail),
        .inject_yneg_avail(inject_yneg_avail),
        .inject_zneg_avail(inject_zneg_avail)



    );
        
//xpos link
    internode_link#(
        .DELAY(LinkDelay)
    )
    xpos_link_inst(
        .rst(rst),
        .clk(clk),
        .tx_par_data_valid(out_xpos_valid),
        .tx_par_data(out_xpos),
        .tx_ser_data(out_xpos_ser),
        .tx_ready(),
        .rx_par_data(in_xpos),
        .rx_par_data_valid(in_xpos_valid),
        .rx_ser_data(in_xpos_ser),
        .rx_ready()
    );

//ypos link
    internode_link#(
        .DELAY(LinkDelay)
    )
    ypos_link_inst(
        .rst(rst),
        .clk(clk),
        .tx_par_data_valid(out_ypos_valid),
        .tx_par_data(out_ypos),
        .tx_ser_data(out_ypos_ser),
        .tx_ready(),
        .rx_par_data(in_ypos),
        .rx_par_data_valid(in_ypos_valid),
        .rx_ser_data(in_ypos_ser),
        .rx_ready()
    );

    //zpos link
    internode_link#(
        .DELAY(LinkDelay)
    )
    zpos_link_inst(
        .rst(rst),
        .clk(clk),
        .tx_par_data_valid(out_zpos_valid),
        .tx_par_data(out_zpos),
        .tx_ser_data(out_zpos_ser),
        .tx_ready(),
        .rx_par_data(in_zpos),
        .rx_par_data_valid(in_zpos_valid),
        .rx_ser_data(in_zpos_ser),
        .rx_ready()
    );

//xneg link
    internode_link#(
        .DELAY(LinkDelay)
    )
    xneg_link_inst(
        .rst(rst),
        .clk(clk),
        .tx_par_data_valid(out_xneg_valid),
        .tx_par_data(out_xneg),
        .tx_ser_data(out_xneg_ser),
        .tx_ready(),
        .rx_par_data(in_xneg),
        .rx_par_data_valid(in_xneg_valid),
        .rx_ser_data(in_xneg_ser),
        .rx_ready()
    );
//yneg link
    internode_link#(
        .DELAY(LinkDelay)
    )
    yneg_link_inst(
        .rst(rst),
        .clk(clk),
        .tx_par_data_valid(out_yneg_valid),
        .tx_par_data(out_yneg),
        .tx_ser_data(out_yneg_ser),
        .tx_ready(),
        .rx_par_data(in_yneg),
        .rx_par_data_valid(in_yneg_valid),
        .rx_ser_data(in_yneg_ser),
        .rx_ready()
    );
//zneg link
    internode_link#(
        .DELAY(LinkDelay)
    )
    zneg_link_inst(
        .rst(rst),
        .clk(clk),
        .tx_par_data_valid(out_zneg_valid),
        .tx_par_data(out_zneg),
        .tx_ser_data(out_zneg_ser),
        .tx_ready(),
        .rx_par_data(in_zneg),
        .rx_par_data_valid(in_zneg_valid),
        .rx_ser_data(in_zneg_ser),
        .rx_ready()
    );

//local unit 
//
    local_unit#(
        .cur_x(cur_x),
        .cur_y(cur_y),
        .cur_z(cur_z)
    )
    local_unit_inst(
        .clk(clk),
        .rst(rst),
        .eject_xpos(eject_xpos),
        .eject_xpos_valid(eject_xpos_valid),
        .eject_ypos(eject_ypos),
        .eject_ypos_valid(eject_ypos_valid),
        .eject_zpos(eject_zpos),
        .eject_zpos_valid(eject_zpos_valid),
        .eject_xneg(eject_xneg),
        .eject_xneg_valid(eject_xneg_valid),
        .eject_yneg(eject_yneg),
        .eject_yneg_valid(eject_yneg_valid),
        .eject_zneg(eject_zneg),
        .eject_zneg_valid(eject_zneg_valid),

        .inject_xpos(inject_xpos),
        .inject_xpos_valid(inject_xpos_valid),
        .inject_ypos(inject_ypos),
        .inject_ypos_valid(inject_ypos_valid),
        .inject_zpos(inject_zpos),
        .inject_zpos_valid(inject_zpos_valid),
        .inject_xneg(inject_xneg),
        .inject_xneg_valid(inject_xneg_valid),
        .inject_yneg(inject_yneg),
        .inject_yneg_valid(inject_yneg_valid),
        .inject_zneg(inject_zneg),
        .inject_zneg_valid(inject_zneg_valid),
        .inject_xpos_avail(inject_xpos_avail),
        .inject_ypos_avail(inject_ypos_avail),
        .inject_zpos_avail(inject_zpos_avail),
        .inject_xneg_avail(inject_xneg_avail),
        .inject_yneg_avail(inject_yneg_avail),
        .inject_zneg_avail(inject_zneg_avail)
    );
    

endmodule
