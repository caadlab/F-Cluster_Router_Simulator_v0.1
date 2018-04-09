#ifndef ROUTER_H
#define ROUTER_H
#include "flit.h"
#include "define.h"
#include  "fifo.h"
//#include "routing_comp.h"
//#include "VCs.h"
#include "local_unit.h"
class router{
public:

    int cur_x;
    int cur_y;
    int cur_z;

    flit* in[PORT_NUM];
    flit* inject[PORT_NUM];

    flit in_latch[PORT_NUM];
    flit inject_latch[PORT_NUM];

    flit out[PORT_NUM];
    flit eject[PORT_NUM];

    bool inject_avail[PORT_NUM];

    int upstream_credits[PORT_NUM];
    int downstream_credits[PORT_NUM];
    int credit_period_counter;

    //input buffers
    fifo input_buffer_list[PORT_NUM];

    flit* flit_list_to_RC[PORT_NUM];
    bool* in_avail_from_RC[PORT_NUM];
    //route compute
    //routing_comp RC_list[PORT_NUM];

    flit* flit_list_to_VA[PORT_NUM];
    bool* in_avail_from_VA[PORT_NUM];
    //VCs
    //VCs VCs_list[PORT_NUM];

    flit* flit_list_to_SA[PORT_NUM * VC_NUM];
    bool* in_avail_from_SA[PORT_NUM * VC_NUM];
    //crossbar switch
    //crossbar_switch xbar;
	
//	flit* eject_ptrs[VC_NUM];
//	bool* inject_avail_ptrs[VC_NUM];
	flit* eject_ptrs[PORT_NUM];
	bool* inject_avail_ptrs[PORT_NUM];
	local_unit app_core;
    
    flit* flit_list_to_ST[PORT_NUM * VC_NUM];
    bool* in_avail_from_ST[PORT_NUM];
    
    bool out_avail_for_passthru[PORT_NUM];
    bool out_avail_for_inject[PORT_NUM];
    bool downstream_avail[PORT_NUM];
    //the availability for bypass traffic to use the out link

	// Ethan add
	//bool potential_output_port[PORT_NUM][PORT_NUM];					// recording the potential output port during RC, for all the input ports
	bool ejection_status[PORT_NUM];												// recording which input port is currently ejecting
	int output_port_mapping[PORT_NUM];											// record the mapping of input and output
	//bool Passthrough_RC_needed[PORT_NUM];									// signify the sets of newly incoming packets that need to route
	//int cur_injection_priority_age[PORT_NUM];									// recording the priority age for the current injection packet, use this information to determine which injection port to give up for the passthrough requests when the idle ports are not enough for all the passthrough packet
	//flit prev_passthrough_flit[PORT_NUM];										// recording the previous received flit, using this information to determine if the newly received flit belongs to the same packet as the previous one, if not, this means the injection for that flit has been interrupted and this port should be freed

    int SA_mode;
    int routing_mode;
	int injection_mode;
	int packet_size;

    
    bool occupy_by_inject[PORT_NUM]; //bool value denoting the out port is occupy the inject traffic
	bool occupy_by_passthru[PORT_NUM];
    
	void router_init(int Cur_x, int Cur_y, int Cur_z, int SA_Mode, int Routing_mode, int Injection_mode, flit** In, flit** Inject, int Injection_gap, int Packet_size);
	int report_pattern_location(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* received_flit, int incoming_dir);

    int consume();
    int produce();
    void router_free();

    

};

    


#endif
