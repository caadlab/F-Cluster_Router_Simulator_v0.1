`include "para.sv"
 module network_tb;



 	reg clk,rst;

	always #5 clk=~clk;

	network    net0(clk,rst);
	initial begin
		clk=0;
		rst=1;

		#100 rst=0;
	end
endmodule
