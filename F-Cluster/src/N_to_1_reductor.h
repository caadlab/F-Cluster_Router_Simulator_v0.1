#ifndef N_TO_1_REDUCTOR_H
#define N_TO_1_REDUCTOR_H
#include "flit.h"
#include "fifo.h"
class N_to_1_reductor{
public:
	int cur_x;
	int cur_y;
	int cur_z;

    int N_fan_in;
    int out_dir;
    int level;
    int id;
    int mode;
    int selector;
	bool selector_valid;
    bool occupy;

    flit** flit_in; //will allocate by N_fan_in slots after init
    flit* in_latch; // will allocate by N_fan_in slots after init
    bool* in_avail; // will allocate by N_fan_in slots after init
    bool out_avail_latch;
	bool* out_avail_latch_to_fifos; // will allcoate by N_fan_in slots after init
    bool* out_avail;
    
    flit out;
	flit* in_slot;
    fifo* in_Q_inst; //will allocate by N_fan_in fifo after init
	int cycle_counter;

	int* downstream_vc_class_0_free_num;
	int* downstream_vc_class_1_free_num;

	// profiling data that recording which packet is taking the current reductor
	int occupy_inject_dir;
	int occupy_src_x;
	int occupy_src_y;
	int occupy_src_z;
	int occupy_dst_x;
	int occupy_dst_y;
	int occupy_dst_z;
	int occupy_packet_id;

    void alloc(int N_Fan_in);
    void N_to_1_reductor_init(int Cur_x, int Cur_y, int Cur_z, int Out_dir, int Level, int Id, int Mode, flit** In_list, bool* Out_avail, int* Downsteam_free_VC_0, int* Downsteam_free_VC_1);
    void consume();
    int produce();
	void N_to_1_reductor_free();
};




#endif
