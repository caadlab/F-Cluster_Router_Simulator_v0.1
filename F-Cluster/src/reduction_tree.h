#ifndef REDUCTION_TREE_H
#define REDUCTION_TREE_H

#include "flit.h"
#include "N_to_1_reductor.h"
class reduction_tree{
public:
	int cur_x;
	int cur_y;
	int cur_z;

    int N_fan_in;
    int out_dir;
    int mode;
    int level_num;
    //int l4_N = 1; //default to be 1
	int l4_N; //default to be 1
    int l4_W;
    int l3_N;
    int l3_W;
    int l2_N;
    int l2_W;
    int l1_N;
    int l1_W;
    flit** flit_in;
    flit* in_latch;
    bool* in_avail;
    bool out_avail_latch;
    bool* out_avail;
    
    flit out;
    flit* in_slot;

	int* downstream_vc_class_0_free_num;
	int* downstream_vc_class_1_free_num;

	bool* out_dir_match;			// if the head flit taking this direction, then set for all the body and tail flits, when the tail flit left, reset this one as false
 
	N_to_1_reductor* l1_reductors;//will allocated by l1_N slots after init
    N_to_1_reductor* l2_reductors;//will allocated by l2_N slots after init
    N_to_1_reductor* l3_reductors;//will allocated by l3_N slots after init
    N_to_1_reductor l4_reductor;

	flit** flit_to_l1;
	flit** flit_l1_to_l2;
	bool** in_avail_l2_to_l1;
	flit** flit_l2_to_l3;
	bool** in_avail_l3_to_l2;
	flit** flit_l3_to_l4;
	bool** in_avail_l4_to_l3;
	bool* out_avail_to_l4;
    
    void reduction_tree_init(int Cur_x, int Cur_y, int Cur_z, int N_Fan_in, int Out_dir, int Level_num, int Mode, int L1_N, int L2_N, int L3_N, flit** In_list, bool* Out_avail, int* Downsteam_free_VC_0, int* Downsteam_free_VC_1);
    void consume();
    int produce();
	void reduction_tree_free();

	// Debugging use
	// Used for locating the pattern on the input side of each N_to_1_reductor
	// Put this function at the beginning of each N_to_1_reductor.produce, take the N_to_1_reductor.in_slot[i] as input flit, scan N_Fan_in (3 or 2) inputs
	int report_pattern_on_reductor_input(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* received_flit, int reductor_level, int reductor_id, int input_port_id);
};



#endif
