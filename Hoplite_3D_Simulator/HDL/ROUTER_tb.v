module ROUTER_tb;
	
	parameter FLIT_SIZE = 128;
	
	reg clk;
	reg rst;
	
	// passthrough inputs
	reg x_in_valid;
	reg y_in_valid;
	reg z_in_valid;
	reg [FLIT_SIZE-1:0] x_input;
	reg [FLIT_SIZE-1:0] y_input;
	reg [FLIT_SIZE-1:0] z_input;
	
	// PE inputs
	reg pe_in_valid;
	reg [FLIT_SIZE-1:0] pe_input;
	
	// Output for passthrough ports
	wire [FLIT_SIZE-1:0] x_output;
	wire [FLIT_SIZE-1:0] y_output;
	wire [FLIT_SIZE-1:0] z_output;
	wire x_out_valid;
	wire y_out_valid;
	wire z_out_valid;
	
	// Outputs for PE
	wire injection_success;
	
	// Outputs for ejection
	wire [FLIT_SIZE-1:0] x_eject;
	wire [FLIT_SIZE-1:0] y_eject;
	wire [FLIT_SIZE-1:0] z_eject;
	wire x_eject_valid;
	wire y_eject_valid;
	wire z_eject_valid;

	ROUTER
	#(
		.FLIT_SIZE(FLIT_SIZE),
		.ADDRESS_WIDTH(3),
		.CUR_X(0),
		.CUR_Y(0),
		.CUR_Z(0)
	)
	ROUTER
	(
		.clk(clk),
		.rst(rst),
		////////////////////////////////////////////////////
		// Connect to NODE input & output
		////////////////////////////////////////////////////
		// Input for passthrough
		.x_in_valid(x_in_valid),
		.y_in_valid(y_in_valid),
		.z_in_valid(z_in_valid),
		.x_input(x_input),
		.y_input(y_input),
		.z_input(z_input),
		// Output for passthrough
		.x_output(x_output),
		.y_output(y_output),
		.z_output(z_output),
		.x_out_valid(x_out_valid),
		.y_out_valid(y_out_valid),
		.z_out_valid(z_out_valid),
		////////////////////////////////////////////////////
		// Connect to PE
		////////////////////////////////////////////////////
		// Injection
		.pe_in_valid(pe_in_valid),
		.pe_input(pe_input),
		.injection_success(injection_success),
		// Ejection
		.x_eject(x_eject),
		.y_eject(y_eject),
		.z_eject(z_eject),
		.x_eject_valid(x_eject_valid),
		.y_eject_valid(y_eject_valid),
		.z_eject_valid(z_eject_valid)
	);
	
	always #1 clk <= ~clk;
	
	initial begin
		rst <= 1'b1;
		clk <= 1'b1;
		// passthrough
		x_in_valid <= 1'b0;
		y_in_valid <= 1'b0;
		z_in_valid <= 1'b0;
		x_input <= 0;
		y_input <= 0;
		z_input <= 0;
		// PE inputs
		pe_in_valid <= 1'b0;
		pe_input <= 0;
		
		#10
			rst <= 1'b0;
		
		#10
			x_in_valid <= 1'b1;
			y_in_valid <= 1'b1;
			z_in_valid <= 1'b1;
			x_input <= {119'd1, 3'd1, 3'd1, 3'd1};
			y_input <= {119'd2, 3'd5, 3'd4, 3'd0};
			z_input <= {119'd3, 3'd7, 3'd0, 3'd0};
			
			pe_in_valid <= 1'b0;
			pe_input <= {119'd3, 3'd1, 3'd2, 3'd3};
			
		#10
			x_in_valid <= 1'b1;
			y_in_valid <= 1'b1;
			z_in_valid <= 1'b0;
			x_input <= {119'd1, 3'd1, 3'd1, 3'd1};
			y_input <= {119'd2, 3'd5, 3'd4, 3'd0};
			z_input <= {119'd3, 3'd7, 3'd0, 3'd0};
			
			pe_in_valid <= 1'b1;
			pe_input <= {119'd3, 3'd1, 3'd2, 3'd3};
			
		#10
			x_in_valid <= 1'b0;
			y_in_valid <= 1'b1;
			z_in_valid <= 1'b0;
			x_input <= {119'd1, 3'd1, 3'd1, 3'd1};
			y_input <= {119'd2, 3'd5, 3'd4, 3'd0};
			z_input <= {119'd3, 3'd7, 3'd0, 3'd0};
			
			pe_in_valid <= 1'b1;
			pe_input <= {119'd3, 3'd1, 3'd2, 3'd3};
	
	end

endmodule