////////////////////////////////////////////////////////////////////////////////////////////////
// Moduel: ARBITER
// By: Chen Yang
// Organization: CAAD Lab @ Boston University
// 03/28/2018
// 
// Generating control signals for the switch muxes
// Each flit carrying destination coordinates {PAYLOAD, Z_DST, Y_DST, X_DST}
// All combinational logic
//
////////////////////////////////////////////////////////////////////////////////////////////////

module ARBITER
#(
	parameter CUR_X = 0,
	parameter CUR_Y = 0,
	parameter CUR_Z = 0,
	
	parameter FLIT_SIZE = 128,
	
	parameter ADDRESS_WIDTH = 3
)
(
	// Pass through
	input rst,
	input x_in_valid,
	input y_in_valid,
	input z_in_valid,
	input [FLIT_SIZE-1:0] x_input,
	input [FLIT_SIZE-1:0] y_input,
	input [FLIT_SIZE-1:0] z_input,
	// Injection
	input pe_in_valid,
	input [FLIT_SIZE-1:0] pe_input,
	// Ejection
	output x_input_eject_valid,
	output y_input_eject_valid,
	output z_input_eject_valid,
	// Input valid signal to switch
	output x_in_valid_to_switch,
	output y_in_valid_to_switch,
	output z_in_valid_to_switch,
	output pe_in_valid_to_switch,
	// Selection signal to switch
	output reg [1:0] x_sel,						// MSB indicate whether injection is valid if the 1 LSB == 1'b0
	output reg [2:0] y_sel,						// MSB indicate whether injection is valid if the 2 LSB == 2'd0
	output reg [2:0] z_sel,						// MSB indicate whether injection is valid if the 2 LSB == 2'd0
	// Backpressure to PE
	output reg injection_success				// Output to PE indicating if the injection is successful
);

	// Collect destination node coordinates
	wire [ADDRESS_WIDTH-1:0] x_dst_x_input;
	wire [ADDRESS_WIDTH-1:0] y_dst_x_input;
	wire [ADDRESS_WIDTH-1:0] z_dst_x_input;
	wire [ADDRESS_WIDTH-1:0] x_dst_y_input;
	wire [ADDRESS_WIDTH-1:0] y_dst_y_input;
	wire [ADDRESS_WIDTH-1:0] z_dst_y_input;
	wire [ADDRESS_WIDTH-1:0] x_dst_z_input;
	wire [ADDRESS_WIDTH-1:0] y_dst_z_input;
	wire [ADDRESS_WIDTH-1:0] z_dst_z_input;
	wire [ADDRESS_WIDTH-1:0] x_dst_injection;
	wire [ADDRESS_WIDTH-1:0] y_dst_injection;
	wire [ADDRESS_WIDTH-1:0] z_dst_injection;
	assign x_dst_x_input = x_input[ADDRESS_WIDTH-1:0];
	assign y_dst_x_input = x_input[2*ADDRESS_WIDTH-1:ADDRESS_WIDTH];
	assign z_dst_x_input = x_input[3*ADDRESS_WIDTH-1:2*ADDRESS_WIDTH];
	assign x_dst_y_input = y_input[ADDRESS_WIDTH-1:0];
	assign y_dst_y_input = y_input[2*ADDRESS_WIDTH-1:ADDRESS_WIDTH];
	assign z_dst_y_input = y_input[3*ADDRESS_WIDTH-1:2*ADDRESS_WIDTH];
	assign x_dst_z_input = z_input[ADDRESS_WIDTH-1:0];
	assign y_dst_z_input = z_input[2*ADDRESS_WIDTH-1:ADDRESS_WIDTH];
	assign z_dst_z_input = z_input[3*ADDRESS_WIDTH-1:2*ADDRESS_WIDTH];
	assign x_dst_injection = pe_input[ADDRESS_WIDTH-1:0];
	assign y_dst_injection = pe_input[2*ADDRESS_WIDTH-1:ADDRESS_WIDTH];
	assign z_dst_injection = pe_input[3*ADDRESS_WIDTH-1:2*ADDRESS_WIDTH];
	
	// Determine the ejection status for each input port
	wire pe_input_eject_valid;
	assign x_input_eject_valid = (x_in_valid && x_dst_x_input == CUR_X && y_dst_x_input == CUR_Y && z_dst_x_input == CUR_Z)? 1'b1 : 1'b0;
	assign y_input_eject_valid = (y_in_valid && x_dst_y_input == CUR_X && y_dst_y_input == CUR_Y && z_dst_y_input == CUR_Z)? 1'b1 : 1'b0;
	assign z_input_eject_valid = (z_in_valid && x_dst_z_input == CUR_X && y_dst_z_input == CUR_Y && z_dst_z_input == CUR_Z)? 1'b1 : 1'b0;
	assign pe_input_eject_valid = (pe_in_valid && x_dst_injection == CUR_X && y_dst_injection == CUR_Y && z_dst_injection == CUR_Z)? 1'b1 : 1'b0;
	
	// Assign the valid signal to switch input
	assign x_in_valid_to_switch = x_in_valid && !x_input_eject_valid;
	assign y_in_valid_to_switch = y_in_valid && !y_input_eject_valid;
	assign z_in_valid_to_switch = z_in_valid && !z_input_eject_valid;
	assign pe_in_valid_to_switch = pe_in_valid && !pe_input_eject_valid;
	
	// Output port occupy status
	wire z_out_occupy;
	wire y_out_occupy;
	wire x_out_occupy;
	
	// Temp selection output after evaluating passthrough traffic
	reg x_sel_passthrough;
	reg [1:0] y_sel_passthrough;
	reg [1:0] z_sel_passthrough;
	
	/////////////////////////////////////////////////////////
	// Passthrough arbitration logic
	/////////////////////////////////////////////////////////
	always@(*)
		begin
		if(rst)
			begin
			x_sel_passthrough <= 1'd0;
			y_sel_passthrough <= 2'd0;
			z_sel_passthrough <= 2'd0;
			end
		// Z input valid, and this is not ejection, then Z input take Z output
		else if(z_in_valid_to_switch)
			begin
			z_sel_passthrough <= 2'd3; 			// Z_out <= Z_in
			// Y input valid, cause Z output is taken, then Y input deflected to Y output
			if(y_in_valid_to_switch)	
				begin
				y_sel_passthrough	<= 2'd2; 		// Y_out <= Y_in, even if cur_y == dst_y, still need to be deflected cause Z_out is taken
				// X input valid, cause Z, Y output are taken, then X input deflected to X output
				if(x_in_valid_to_switch)
					begin
					x_sel_passthrough <= 1'b1;		// X_out <= X_in, even if cur_x == dst_x, still need to be deflected cause Y_out is taken
					end
				// X input not valid, then X output is not taken
				else
					begin
					x_sel_passthrough <= 1'b0;		// X_out no passthrough
					end
				end
			else
				begin
				// Y input invalid, X input valid and X wanna take Y
				if(x_in_valid_to_switch && x_dst_x_input == CUR_X)
					begin
					y_sel_passthrough <= 2'd1;		// Y_out <= X_in
					x_sel_passthrough <= 1'b0;		// X_out no passthrough 
					end
				// Y input invalid, X input valid, but X input still need to take X output
				else if(x_in_valid_to_switch)		
					begin
					y_sel_passthrough <= 2'd0;		// Y_out no passthrough
					x_sel_passthrough <= 1'b1;		// X_out <= X_in
					end
				// both X and Y input not valid, then Y and X output is not taken
				else		
					begin
					y_sel_passthrough <= 2'd0;		// Y_out no passthrough
					x_sel_passthrough <= 1'b0;		// X_out no passthrough
					end
				end
			end
		// Z output is available for X or Y input to take
		else
			begin
			// Y input is valid and need to turn to Z
			if(y_in_valid_to_switch && y_dst_y_input == CUR_Y)
				begin
				z_sel_passthrough <= 2'd2;			// Z_out <= Y_in
				// Y take Z output, X input valid and X wanna take Y
				if(x_in_valid_to_switch && x_dst_x_input == CUR_X)
					begin
					y_sel_passthrough <= 2'd1;		// Y_out <= X_in
					x_sel_passthrough <= 1'b0;		// X_out no passthrough 
					end
				// Y take Z output, X input valid, but X input still need to take X output
				else if(x_in_valid_to_switch)		
					begin
					y_sel_passthrough <= 2'd0;		// Y_out no passthrough
					x_sel_passthrough <= 1'b1;		// X_out <= X_in
					end
				// Y take Z output, X input not valid, then both Y and X outputs are not taken
				else		
					begin
					y_sel_passthrough <= 2'd0;		// Y_out no passthrough
					x_sel_passthrough <= 1'b0;		// X_out no passthrough
					end
				end
				
			// Z input is invalid, Y input is valid, but still need to go Y direction
			else if(y_in_valid_to_switch)
				begin
				y_sel_passthrough <= 2'd2;
				// X input is valid, as Y output is taken by Y input, the X input can either take X output or Z output
				if(x_in_valid_to_switch)
					begin
					// X input need to turn to Z directly
					if(x_dst_x_input == CUR_X && y_dst_x_input == CUR_Y)
						begin
						z_sel_passthrough <= 2'd1;		// Z_out <= X_in
						x_sel_passthrough <= 1'b0;		// X_out no passthrough
						end
					// X input cannot turn to Z
					else
						begin
						z_sel_passthrough <= 2'd0;		// Z_out no passthrough
						x_sel_passthrough <= 1'b1;		// X_out <= X_in
						end
					end
				// Both Z and X input is not valid	
				else
					begin
					z_sel_passthrough <= 2'd0;		// Z_out no passthrough
					x_sel_passthrough <= 1'b0;		// X_out no passthrough
					end
				end
				
			// Both Z and Y inputs are invalid, then X input is free to take all 3 output ports
			else
				begin
				// X input is valid
				if(x_in_valid_to_switch)
					begin
					// X input take Z output
					if(x_dst_x_input == CUR_X && y_dst_x_input == CUR_Y)
						begin
						z_sel_passthrough <= 2'd1;		// Z_out <= X_in
						y_sel_passthrough <= 2'd0;		// Y_out no passthrough
						x_sel_passthrough <= 1'b0;		// X_out no passthrough
						end
					// X input take Y output
					else if(x_dst_x_input == CUR_X)
						begin
						z_sel_passthrough <= 2'd0;		// Z_out no passthrough
						y_sel_passthrough <= 2'd1;		// Y_out <= X_in
						x_sel_passthrough <= 1'b0;		// X_out no passthrough
						end
					// X input take X output
					else
						begin
						z_sel_passthrough <= 2'd0;		// Z_out no passthrough
						y_sel_passthrough <= 2'd0;		// Y_out no passthrough
						x_sel_passthrough <= 1'b1;		// X_out <= X_in
						end
					end
				// None of X Y Z inputs are valid
				else
					begin
					z_sel_passthrough <= 2'd0;		// Z_out no passthrough
					y_sel_passthrough <= 2'd0;		// Y_out no passthrough
					x_sel_passthrough <= 1'b0;		// X_out no passthrough
					end
				end
			end
		end

	/////////////////////////////////////////////////////////
	// Control output assignment and injection selection
	/////////////////////////////////////////////////////////
	always@(*)
		begin
		if(rst)
			begin
			x_sel <= 2'd0;
			y_sel <= 3'd0;
			z_sel <= 3'd0;
			injection_success <= 1'b0;
			end
		// Injection needed
		else if(pe_in_valid_to_switch)
			begin
			// injection on X direction
			if(x_dst_injection != CUR_X)
				begin
				y_sel <= {1'b0, y_sel_passthrough};		// Y_out is all set
				z_sel <= {1'b0, z_sel_passthrough};		// Z_out is all set
				// X_out is available for injection
				if(x_sel_passthrough == 1'b0)
					begin
					x_sel <= {1'b1, 1'b0};					// Injection on X_out
					injection_success <= 1'b1;
					end
				// X_out is not available for injection, injection fail
				else
					begin
					x_sel <= {1'b0, x_sel_passthrough};
					injection_success <= 1'b0;
					end
				end
			// injection on Y direction
			else if(y_dst_injection != CUR_Y)
				begin
				x_sel <= {1'b0, x_sel_passthrough};		// X_out is all set
				z_sel <= {1'b0, z_sel_passthrough}; 	// Z_out is all set
				// Y_out is available for injection
				if(y_sel_passthrough == 2'd0)
					begin
					y_sel <= {1'b1, 2'd0};					// Injection on Y_out
					injection_success <= 1'b1;
					end
				// Y_out is not available for injection, injection fail
				else
					begin
					y_sel <= {1'b0, y_sel_passthrough};
					injection_success <= 1'b0;
					end
				end
			// injectio on Z direction
			else
				begin
				x_sel <= {1'b0, x_sel_passthrough};		// X_out is all set
				y_sel <= {1'b0, y_sel_passthrough}; 	// Y_out is all set
				// Z_out is available for injection
				if(z_sel_passthrough == 2'd0)
					begin
					z_sel <= {1'b1, 2'd0};					// Injection on Z_out
					injection_success <= 1'b1;
					end
				// Z_out is not available for injection, injection fail
				else
					begin
					z_sel <= {1'b0, z_sel_passthrough};
					injection_success <= 1'b0;
					end
				end
			end
		// No injection needed
		else
			begin
			x_sel <= {1'b0, x_sel_passthrough};
			y_sel <= {1'b0, y_sel_passthrough};
			z_sel <= {1'b0, z_sel_passthrough};
			injection_success <= 1'b0;
			end
		end
		
endmodule