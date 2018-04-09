`ifndef PARA_SV
`define PARA_SV
parameter FLIT_SIZE=128;
parameter PHIT_SIZE=256;
parameter IN_Q_SIZE=512;
parameter VC_SIZE=16; //VC size should be bigger than the largest possible packet size
parameter VC_NUM=9;
parameter XSIZE=4'd4;  //the number of nodes in x dimension
parameter YSIZE=4'd4;  //the number of nodes in y dimension
parameter ZSIZE=4'd4;  //the number of nodes in z dimension
parameter XW=3;   //the width of the x coordinates
parameter YW=3;   //the width of the y coordinates
parameter ZW=3;   //the width of the z coordinates
parameter DSTW=9; //the width of the destination field
parameter DIR_INJECT=3'd0;
parameter DIR_XPOS=3'd1;
parameter DIR_YPOS=3'd2;
parameter DIR_ZPOS=3'd3;
parameter DIR_XNEG=3'd4;
parameter DIR_YNEG=3'd5;
parameter DIR_ZNEG=3'd6;
parameter DIR_EJECT=3'd7;
parameter LinkDelay = 50;

/* packet format
* head flit
|FLIT type (3 bits)| VC class (1 bit) | dst z (3 bits) | dst y (3 bits) | dst x (3 bits) | priority field (4 bits) | src z (3 bits)| src y (3 bits)| src x (3 bits)| packet id (16 bits)| payload (66 bits)|
body flit
|FLIT type (3 bits)| payload|
tail flit
|FLIT type (3 bits)| payload|
*/


parameter HEADER_LEN=3;
parameter ROUTE_LEN = 3;
parameter HEAD_FLIT=3'b000;
parameter BODY_FLIT=3'b001;
parameter TAIL_FLIT=3'b010;
parameter SINGLE_FLIT=3'b011;
parameter CREDIT_FLIT=3'b100;


parameter VC_CLASS_POS = FLIT_SIZE - HEADER_LEN - 1;
parameter PORT_NUM = 6;
parameter DST_ZPOS = VC_CLASS_POS - 1;
parameter DST_YPOS = DST_ZPOS - ZW;
parameter DST_XPOS = DST_YPOS - YW;
parameter CMP_POS = DST_XPOS - XW;
parameter CMP_LEN = 4;


parameter ERR_NO_ERROR = 3'd0;
parameter ERR_FLIT_WRONG = 3'd1;
parameter ERR_FLIT_MISSING = 3'd2;
parameter ERR_FLIT_TIMEOUT = 3'd3;
parameter ERR_PCKT_TIMEOUT = 3'd4;
parameter ERR_PCKT_WRONG = 3'd5;
`endif

