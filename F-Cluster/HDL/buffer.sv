//Purpose: general-purpose buffer more realistic fifo
//Author: Jiayi Sheng
//Organization: CAAD lab @ Boston University
//Start date: Feb 10th 2015
//
//`define SIM
`define SYNTH
module buffer
#(
    parameter buffer_depth_log = 12,
    parameter buffer_depth=8,
    parameter buffer_width=64
)(
    input clk,
    input rst,
    input [buffer_width-1:0] in,
    input produce,
    input consume,
    output full,
    output empty,
    output reg [buffer_width-1:0] out,
    output [buffer_width-1:0] usedw
);
  	
    
`ifdef SIM
    parameter sample_cycle=64;
    reg[15:0] sample_counter;
    integer fd;
    always@(posedge clk) begin
        if(rst) begin
            sample_counter<=0;
        end
        else begin
            sample_counter<=(sample_counter==sample_cycle-1)?0:sample_counter+1;
        end
    end

    always@(posedge clk) begin
        if(sample_counter==sample_cycle-1) begin
            fd=$fopen("buffer_size.txt","a");
            if(fd) begin
                $display("buffer_size.txt open successfully\n");
            end
            else begin
                $display("file open failed\n");
            end
            $strobe("Displaying in %m ");
            $strobe("buffer utilization is: ");
            $strobe("tail is %d, head is %d, util is %d ",tail, head,((tail>=head)?(tail-head):(tail-head+buffer_depth)));
            $fdisplay(fd,"%d %d",((tail>=head)?(tail-head):(tail-head+buffer_depth)),buffer_depth);
            $fclose(fd);
        end
    end


`endif
    

 `ifdef SYNTH
	scfifo	scfifo_component (
				.aclr (rst),
				.clock (clk),
				.data (in),
				.rdreq (consume),
				.wrreq (produce),
				.empty (empty),
				.full (full),
				.q (out),
				.usedw (usedw),
				.almost_empty (),
				.almost_full (),
				.sclr ());
	defparam
		scfifo_component.add_ram_output_register = "ON",
		scfifo_component.intended_device_family = "Stratix V",
		scfifo_component.lpm_hint = "RAM_BLOCK_TYPE=AUTO",
		scfifo_component.lpm_numwords = buffer_depth,
		scfifo_component.lpm_showahead = "ON",
		scfifo_component.lpm_type = "scfifo",
		scfifo_component.lpm_width = buffer_width,
		scfifo_component.lpm_widthu = buffer_depth_log,
		scfifo_component.overflow_checking = "ON",
		scfifo_component.underflow_checking = "ON",
		scfifo_component.use_eab = "ON";           
`else
    

    
    integer i;

    wire[buffer_depth-1:0] head_next;
	wire[buffer_depth-1:0] tail_next;
	
    reg[buffer_depth-1:0] head;
	reg[buffer_depth-1:0] tail;

	reg[buffer_width-1:0] fifo[buffer_depth:0]; 

    initial begin
        integer i;
        for(i = 0; i <= buffer_depth; i = i + 1) begin
            fifo[i] = 0;
        end
    end


    assign empty = (head == tail);
	assign full = (tail == buffer_depth - 1) ? (head == 0) : (head == tail + 1);
	assign head_next = (head == buffer_depth - 1) ? 0 : head + 1;
	assign tail_next = (tail == buffer_depth - 1) ? 0 : tail + 1;


    always@(*) begin
        out = fifo[head];
    end

    always@(posedge clk) begin
        if(produce)
            fifo[tail] <= in;
    end


    always@(posedge clk) begin
        if(rst) begin
            tail <= 0;
        end
        else begin
            if(produce && ~full)
                tail <= tail_next;
        end
    end

    always@(posedge clk) begin
        if(rst) begin
            head <= 0;
        end
        else begin
            if(consume && ~empty)
                head <= head_next;
        end
    end


    reg [buffer_width - 1 : 0] usedw_reg;
    always@(*) begin
        if(tail >= head) begin
            usedw_reg = tail - head;
        end
        else begin
            usedw_reg = buffer_depth - head + tail;
        end
    end
    assign usedw = usedw_reg;

`endif

endmodule
        
