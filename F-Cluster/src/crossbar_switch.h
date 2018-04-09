#ifndef CROSSBAR_SWITCH_H
#define CROSSBAR_SWITCH_H

#define N_FAN_IN PORT_NUM * VC_NUM
#include "define.h"
#include "reduction_tree.h"

class crossbar_switch{
public:
   int cur_x;
   int cur_y;
   int cur_z;

   int N_fan_in;
//   int N_fan_out = PORT_NUM;
   int N_fan_out;
   int mode;
   flit* flit_in[N_FAN_IN];
//   int level_num = 4;
   int level_num;
   flit in_latch[N_FAN_IN];
   bool in_avail[PORT_NUM][N_FAN_IN];
   bool out_avail_latch[PORT_NUM];
   bool* out_avail[PORT_NUM];

   flit* in_list_to_tree[N_FAN_IN];//pointers passed to all the reduction tree
   bool* out_avail_to_tree[PORT_NUM];

   flit out[PORT_NUM];
   

   reduction_tree tree_list[PORT_NUM];

   void crossbar_switch_init(int Cur_x, int Cur_y, int Cur_z, int Mode, flit** In_list, bool** Out_avail);
   void consume();
   int produce();
   bool lookup_in_avail(int port_id, int our_dir);
   void crossbar_switch_free();

   // Debugging use
   // Used for locating the pattern on the input side of crossbar_switch
   // Put this function at the beginning of corssbar_switch.produce, take the in_latch[i] as input flit, scan N_FAN_IN in_latches
   int report_pattern_received(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* received_flit, int input_port_number);
   // Used for locating the pattern on the output side of crossbar_switch
   // Put this function at the end of corssbar_switch.produce, take the out[i] as input flit, scan PORT_NUM output ports
   int report_pattern_left(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* received_flit, int output_port_number);
};




#endif
