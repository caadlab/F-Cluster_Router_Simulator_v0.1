

`include "para.sv"

module internode_link
#(
    parameter DELAY = 100
)
(
    input rst,
    input clk,

    input tx_par_data_valid,
    input [FLIT_SIZE - 1 : 0] tx_par_data,
    output [FLIT_SIZE : 0] tx_ser_data,
    output tx_ready,

    output [FLIT_SIZE - 1 : 0] rx_par_data,
    output rx_par_data_valid,
    input [FLIT_SIZE : 0] rx_ser_data,
    output rx_ready

);

    

    reg [FLIT_SIZE : 0] shift_reg_tx[DELAY / 2 - 1 : 0];//half of the delay is consumed at the tx side, shift_reg_tx[0] connect to the tx_par_data
    reg [FLIT_SIZE : 0] shift_reg_rx[DELAY / 2 - 1 : 0];//the other half of the delay is consumed at the rx side, shift_reg_rx[0] connects to the rx_par_data

    assign tx_ready=1;
    assign rx_ready=1;//for simulation, the tx and rx ready signals both remain high
    
    integer i=0;
    always@(posedge clk) begin
        if(rst) begin
            for(i = 0; i < DELAY / 2; i = i + 1) begin
                shift_reg_tx[i] <= 0;
            end
        end
        else begin
            shift_reg_tx[0] <= {tx_par_data_valid, tx_par_data};
            for(i = 0; i < DELAY / 2 - 1; i = i + 1) begin
                shift_reg_tx[i + 1] <= shift_reg_tx[i];
            end
        end
    end
    assign tx_ser_data = shift_reg_tx[DELAY / 2 - 1];

    always@(posedge clk) begin
        if(rst) begin
            for(i = 0;i < DELAY / 2; i = i + 1) begin
                shift_reg_rx[i] <= 0;
            end
        end
        else begin
            shift_reg_rx[DELAY / 2 - 1] <= rx_ser_data;
            for(i = DELAY / 2 - 2; i >= 0; i = i - 1) begin
                shift_reg_rx[i]<=shift_reg_rx[i + 1];
            end
        end
    end
    assign rx_par_data = shift_reg_rx[0][FLIT_SIZE - 1 : 0];
    assign rx_par_data_valid = shift_reg_rx[0][FLIT_SIZE];
endmodule
    

    


    
