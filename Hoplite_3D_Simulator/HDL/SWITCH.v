////////////////////////////////////////////////////////////////////////////////////////////////
// Moduel: SWITCH
// By: Chen Yang
// Organization: CAAD Lab @ Boston University
// 03/28/2018
// 
// 4*3 switch, handles x,y,z passthrough and local injection, and ejection (ejection control signal is controlled by ARBITER)
// ejection is handled in the upper router level to reduce switch size
// All combinational logic
//
// Schematic: 
//																							 / XPOS
//		ZNEG -> ZPOS				YNEG - YPOS							XNEG - YPOS
//												\ ZPOS									 \ ZPOS
////////////////////////////////////////////////////////////////////////////////////////////////

module SWITCH
#(
	parameter FLIT_SIZE = 128
)
(
	////////////////////////////////////////////////////
	// Connect to ROUTER input & output
	////////////////////////////////////////////////////
	// Input for passthrough
	input x_in_valid,
	input y_in_valid,
	input z_in_valid,
	input [FLIT_SIZE-1:0] x_input,
	input [FLIT_SIZE-1:0] y_input,
	input [FLIT_SIZE-1:0] z_input,
	// Output for passthrough and ejection
	output [FLIT_SIZE-1:0] x_output,
	output [FLIT_SIZE-1:0] y_output,
	output [FLIT_SIZE-1:0] z_output,
	output x_out_valid,
	output y_out_valid,
	output z_out_valid,
	
	////////////////////////////////////////////////////
	// Connect to ROUTER PE input & output
	////////////////////////////////////////////////////
	// Injection
	input pe_in_valid,
	input [FLIT_SIZE-1:0] pe_input,
	// Ejection
	input x_input_eject_valid,
	input y_input_eject_valid,
	input z_input_eject_valid,
	output [FLIT_SIZE-1:0] x_eject,
	output [FLIT_SIZE-1:0] y_eject,
	output [FLIT_SIZE-1:0] z_eject,
	output x_eject_valid,
	output y_eject_valid,
	output z_eject_valid,
	
	////////////////////////////////////////////////////
	// Connect to ARBITER output
	////////////////////////////////////////////////////	
	// Arbitration signal
	input [1:0] x_sel,				// MSB indicate whether injection is valid if the 1 LSB == 1'b0
	input [2:0] y_sel,				// MSB indicate whether injection is valid if the 2 LSB == 2'd0
	input [2:0] z_sel					// MSB indicate whether injection is valid if the 2 LSB == 2'd0
);

// x_sel:   0: injection   1: x_input
assign x_output = (x_sel[0]==1'd0)? pe_input : x_input;
// y_sel:   0: injection   1: x_input   2or3: y_input
assign y_output = (y_sel[1:0]==2'd0)? pe_input : (y_sel[1:0]==2'd1)? x_input : y_input;
// z_sel:   0: injection   1: x_input   2: y_input   3: z_input
assign z_output = (z_sel[1:0]==2'd0)? pe_input : (z_sel[1:0]==2'd1)? x_input : (z_sel[1:0]==2'd2)? y_input : z_input;

assign x_out_valid = (x_sel[0]==1'd0)? x_sel[1]&&pe_in_valid : x_in_valid;
assign y_out_valid = (y_sel[1:0]==2'd0)? y_sel[2]&&pe_in_valid : (y_sel[1:0]==2'd1)? x_in_valid : y_in_valid;
assign z_out_valid = (z_sel[1:0]==2'd0)? z_sel[2]&&pe_in_valid : (z_sel[1:0]==2'd1)? x_in_valid : (z_sel[1:0]==2'd2)? y_in_valid : z_in_valid;

// Ejection related
assign x_eject_valid = x_input_eject_valid;
assign y_eject_valid = y_input_eject_valid;
assign z_eject_valid = z_input_eject_valid;
assign x_eject = (x_input_eject_valid) ? x_input : 0;
assign y_eject = (y_input_eject_valid) ? y_input : 0;
assign z_eject = (z_input_eject_valid) ? z_input : 0;

endmodule