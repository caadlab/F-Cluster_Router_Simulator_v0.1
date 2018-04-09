#ifndef ROUTER_H
#define ROUTER_H
#include "flit.h"
#include "define.h"
#include  "fifo.h"
#include "routing_comp.h"
#include "VCs.h"
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

	int allow_vc_num;

    bool inject_avail[PORT_NUM];

    int upstream_credits[PORT_NUM];
    int downstream_credits[PORT_NUM];
    int credit_period_counter;
	int downstream_vc_credits[PORT_NUM][VC_NUM];

    //input buffers
    fifo input_buffer_list[PORT_NUM];

    flit* flit_list_to_RC[PORT_NUM];
    bool* in_avail_from_RC[PORT_NUM];
    //route compute
    routing_comp RC_list[PORT_NUM];

    flit* flit_list_to_VA[PORT_NUM];
    bool* in_avail_from_VA[PORT_NUM];
    //VCs
    VCs VCs_list[PORT_NUM];

    flit* flit_list_to_SA[PORT_NUM * VC_NUM];
    bool* in_avail_from_SA[PORT_NUM * VC_NUM];
    //crossbar switch
    crossbar_switch xbar;
	
	flit* eject_ptrs[VC_NUM];
	bool* inject_avail_ptrs[VC_NUM];
	local_unit app_core;
    
    flit* flit_list_to_ST[PORT_NUM * VC_NUM];
    bool* in_avail_from_ST[PORT_NUM];
    
    bool out_avail_for_passthru[PORT_NUM];
    bool out_avail_for_inject[PORT_NUM];
    bool downstream_avail[PORT_NUM];
    //the availability for bypass traffic to use the out link
    
    int SA_mode;
    int routing_mode;
	int injection_mode;

	int packet_size;

    //
    bool occupy_by_inject[PORT_NUM]; //bool value denoting the out port is occupy the inject traffic
	bool occupy_by_passthru[PORT_NUM];
    
	void router_init(int Cur_x, int Cur_y, int Cur_z, int SA_Mode, int Routing_mode, int Injection_mode, flit** In, flit** Inject, int Injection_gap, int Packet_size, int Allow_VC_Num);

	// Debugging use
	// Used for locating the pattern on the receiving side cycle by cycle
	// Put this function after the RC and SA is done, so it will print out which direction the packet will be send to
	int report_pattern_received(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* received_flit, int incoming_dir);
	// Used for checking at which cycle the target flit has been injected at a certain node
	// Put this function at the end of router production, the input flit should connect to appcore_inject[i]
	int report_pattern_injected(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* injecting_flit);
	// Used for locating the pattern when it leave a node
	// Put this function after the output port in router.cpp is assigned, thus it will reflect when the flit has left the node
	int report_pattern_sent(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* received_flit);

    int consume();
    int produce();
    void router_free();

    

};

    


#endif
