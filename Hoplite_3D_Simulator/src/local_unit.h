#ifndef LOCAL_UNIT_H
#define LOCAL_UNIT_H
#include "flit.h"
class local_unit{
public:
    int cycle_counter;
	int prev_injection_cycle[PORT_NUM];				// recording the cycle number of the previous injection, use this one to compare with the current cycle_counter to determine if the injection is interrupted or not
    flit inject[PORT_NUM];
    bool* inject_avail[PORT_NUM];
    flit* eject[PORT_NUM];
    
    bool inject_avail_latch[PORT_NUM];
	bool inject_avail_post_latch[PORT_NUM];
    flit eject_latch[PORT_NUM];
	int credit_period_counter;

    int cur_x;
    int cur_y;
    int cur_z;
    
    int mode;
    int injection_gap; // the number of clock cycles gap between 2 injection packets
    int packet_size;
    int packet_num;
    
    int inject_pckt_counter[PORT_NUM];
	int next_inject_pckt_counter[PORT_NUM];
    int inject_flit_counter[PORT_NUM];
    int inject_control_counter[PORT_NUM];

	int eject_pckt_counter[PORT_NUM];
	int eject_flit_counter[PORT_NUM];
	int cur_eject_src_x[PORT_NUM];
	int cur_eject_src_y[PORT_NUM];
	int cur_eject_src_z[PORT_NUM];
	int cur_eject_pckt_id[PORT_NUM];

	int eject_state[PORT_NUM];

    bool all_pckt_rcvd;

	void local_unit_init(int Cur_x, int Cur_y, int Cur_z, int Mode, int Injection_gap, int Packet_size, int Packet_num, flit** Eject, bool** Inject_avail);
    void produce();
    int consume();
	//int report_pattern_location(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, flit* received_flit);

};


#endif
