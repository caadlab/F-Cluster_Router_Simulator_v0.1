////////////////////////////////////////////////////////////////////////////////////////////////
// Moduel: router
// By: Chen Yang
// Organization: CAAD Lab @ Boston University
// 03/29/2018
// 
// Router holds arbiter and switch, expose ports to PE for injection and ejection
//
////////////////////////////////////////////////////////////////////////////////////////////////

module ROUTER
#(
	parameter FLIT_SIZE = 128,
	parameter ADDRESS_WIDTH = 3,
	
	parameter CUR_X = 0,
	parameter CUR_Y = 0,
	parameter CUR_Z = 0
)
(
	input clk,
	input rst,
	////////////////////////////////////////////////////
	// Connect to NODE input & output
	////////////////////////////////////////////////////
	// Input for passthrough
	input x_in_valid,
	input y_in_valid,
	input z_in_valid,
	input [FLIT_SIZE-1:0] x_input,
	input [FLIT_SIZE-1:0] y_input,
	input [FLIT_SIZE-1:0] z_input,
	// Output for passthrough
	output reg [FLIT_SIZE-1:0] x_output,
	output reg [FLIT_SIZE-1:0] y_output,
	output reg [FLIT_SIZE-1:0] z_output,
	output reg x_out_valid,
	output reg y_out_valid,
	output reg z_out_valid,
	////////////////////////////////////////////////////
	// Connect to PE
	////////////////////////////////////////////////////
	// Injection
	input pe_in_valid,
	input [FLIT_SIZE-1:0] pe_input,
	output reg injection_success,
	// Ejection
	output reg [FLIT_SIZE-1:0] x_eject,
	output reg [FLIT_SIZE-1:0] y_eject,
	output reg [FLIT_SIZE-1:0] z_eject,
	output reg x_eject_valid,
	output reg y_eject_valid,
	output reg z_eject_valid
);

	////////////////////////////////////////////////////
	// wires connect to switch output
	////////////////////////////////////////////////////
	wire [FLIT_SIZE-1:0] switch_x_output;
	wire [FLIT_SIZE-1:0] switch_y_output;
	wire [FLIT_SIZE-1:0] switch_z_output;
	wire switch_x_out_valid;
	wire switch_y_out_valid;
	wire switch_z_out_valid;
	wire [FLIT_SIZE-1:0] switch_x_eject;
	wire [FLIT_SIZE-1:0] switch_y_eject;
	wire [FLIT_SIZE-1:0] switch_z_eject;
	wire switch_x_eject_valid;
	wire switch_y_eject_valid;
	wire switch_z_eject_valid;
	
	////////////////////////////////////////////////////
	// wires connect to arbiter output
	////////////////////////////////////////////////////
	wire arbiter_injection_success;
	
	////////////////////////////////////////////////////
	// wires between switch and arbiter
	////////////////////////////////////////////////////
	wire [1:0] arbiter_x_sel;
	wire [2:0] arbiter_y_sel;
	wire [2:0] arbiter_z_sel;
	
	wire x_in_valid_to_switch;
	wire y_in_valid_to_switch;
	wire z_in_valid_to_switch;
	wire pe_in_valid_to_switch;
	
	wire x_input_eject_valid;
	wire y_input_eject_valid;
	wire z_input_eject_valid;

	////////////////////////////////////////////////////
	// Switch module
	////////////////////////////////////////////////////
	SWITCH#(
		.FLIT_SIZE(FLIT_SIZE)
	)
	SWITCH(
	// Input for passthrough
	.x_in_valid(x_in_valid_to_switch),						// receive from arbiter, after ejection is removed
	.y_in_valid(y_in_valid_to_switch),						// receive from arbiter, after ejection is removed
	.z_in_valid(z_in_valid_to_switch),						// receive from arbiter, after ejection is removed
	.x_input(x_input),
	.y_input(y_input),
	.z_input(z_input),
	// Output for passthrough and ejection
	.x_output(switch_x_output),
	.y_output(switch_y_output),
	.z_output(switch_z_output),
	.x_out_valid(switch_x_out_valid),
	.y_out_valid(switch_y_out_valid),
	.z_out_valid(switch_z_out_valid),
	// Injection
	.pe_in_valid(pe_in_valid_to_switch),					// receive from arbiter, after ejection is removed
	.pe_input(pe_input),
	// Ejection
	.x_input_eject_valid(x_input_eject_valid),			// input from arbiter
	.y_input_eject_valid(y_input_eject_valid),			// input from arbiter
	.z_input_eject_valid(z_input_eject_valid),			// input from arbiter
	.x_eject(switch_x_eject),
	.y_eject(switch_y_eject),
	.z_eject(switch_z_eject),
	.x_eject_valid(switch_x_eject_valid),
	.y_eject_valid(switch_y_eject_valid),
	.z_eject_valid(switch_z_eject_valid),
	////////////////////////////////////////////////////
	// Connect to ARBITER output
	////////////////////////////////////////////////////	
	// Arbitration signal
	.x_sel(arbiter_x_sel),
	.y_sel(arbiter_y_sel),
	.z_sel(arbiter_z_sel)
	);
	
	
	////////////////////////////////////////////////////
	// Arbiter module
	////////////////////////////////////////////////////
	ARBITER
	#(
	.CUR_X(CUR_X),
	.CUR_Y(CUR_Y),
	.CUR_Z(CUR_Z),
	.FLIT_SIZE(FLIT_SIZE),
	.ADDRESS_WIDTH(ADDRESS_WIDTH)
	)
	ARBITER
	(
	// Pass through
	.rst(rst),
	.x_in_valid(x_in_valid),
	.y_in_valid(y_in_valid),
	.z_in_valid(z_in_valid),
	.x_input(x_input),
	.y_input(y_input),
	.z_input(z_input),
	// Injection
	.pe_in_valid(pe_in_valid),
	.pe_input(pe_input),
	// Ejection
	.x_input_eject_valid(x_input_eject_valid),
	.y_input_eject_valid(y_input_eject_valid),
	.z_input_eject_valid(z_input_eject_valid),
	// Input valid signal to switch
	.x_in_valid_to_switch(x_in_valid_to_switch),
	.y_in_valid_to_switch(y_in_valid_to_switch),
	.z_in_valid_to_switch(z_in_valid_to_switch),
	.pe_in_valid_to_switch(pe_in_valid_to_switch),
	// Selection signal to switch
	.x_sel(arbiter_x_sel),						// MSB indicate whether injection is valid if the 1 LSB == 1'b0
	.y_sel(arbiter_y_sel),						// MSB indicate whether injection is valid if the 2 LSB == 2'd0
	.z_sel(arbiter_z_sel),						// MSB indicate whether injection is valid if the 2 LSB == 2'd0
	// Backpressure to PE
	.injection_success(arbiter_injection_success)				// Output to PE indicating if the injection is successful
);

	always@(posedge clk)
		begin
		if(rst)
			begin
			x_output <= 0;
			y_output <= 0;
			z_output <= 0;
			x_out_valid <= 1'b0;
			y_out_valid <= 1'b0;
			z_out_valid <= 1'b0;
			x_eject <= 0;
			y_eject <= 0;
			z_eject <= 0;
			x_eject_valid <= 1'b0;
			y_eject_valid <= 1'b0;
			z_eject_valid <= 1'b0;
			injection_success <= 1'b0;			
			end
		else
			begin
			x_output <= switch_x_output;
			y_output <= switch_y_output;
			z_output <= switch_z_output;
			x_out_valid <= switch_x_out_valid;
			y_out_valid <= switch_y_out_valid;
			z_out_valid <= switch_z_out_valid;
			x_eject <= switch_x_eject;
			y_eject <= switch_y_eject;
			z_eject <= switch_z_eject;
			x_eject_valid <= switch_x_eject_valid;
			y_eject_valid <= switch_y_eject_valid;
			z_eject_valid <= switch_z_eject_valid;
			injection_success <= arbiter_injection_success;
			end
		
		end

endmodule