#include "VCs.h"
#include "define.h"
#include "flit.h"
#include "math.h"
#include "crossbar_switch.h"
#include <stdio.h>
#include <stdlib.h>

void VCs::VCs_init(int Dir, flit* In, crossbar_switch* Sw, int Allow_VC_Num){
    dir = Dir;
    in = In;
    sw = Sw;

	ALLOW_VC_NUM = Allow_VC_Num;

	in_avail = true;

	input_prefetch_enable = false;
	prev_input_prefetch_enable = false;
	original_grant = 0;
    
    //init all in_latches
    for(int i = 0; i < VC_NUM; ++i){
        in_latch[i].valid = false;
		out_avail_latch[i] = true;
    }
    for(int i = 0; i < VC_NUM; ++i){
        VC_array[i].fifo_init(VC_SIZE, &(in_latch[i]), &(out_avail_latch[i]));
    }
    in_avail = true; 
    for(int i = 0; i < VC_NUM; ++i){
        out[i].valid = false;
    }
    //init all the VC states to idle state
    for(int i = 0; i < VC_NUM; ++i){
		VC_state[i] = VC_IDLE;
    }
    grant = 0; // initially no grants

	// VC usage status for profiling purpose
	num_active_VCs = 0;

    return;
}

void VCs::consume(){
/*
	// First check if this is the first cycle of pre-fetch
	if (input_prefetch_enable && !prev_input_prefetch_enable) {
		// save the original grant
		original_grant = grant;
		// set the in_avail as true, so the input from RC_out can be taken in
		in_avail = true;
	}
*/

	if (in->valid && in->dir_out == DIR_EJECT) {
		printf("Ejection packet enters VC!\n\n\n");
//		exit(-1);
	}

    //do the VC allocation first
	int i = 0;
    if(in->valid && (in->flit_type == HEAD_FLIT || in->flit_type == SINGLE_FLIT)){//allocate this flit to next available idle VC
		// When VC_class is 0
		if(!(in->VC_class)){
			// For VC class 0, you only allow to use VC 0~(ALLOW_VC-2)
			for (i = 0; i < ALLOW_VC_NUM - 1; ++i){
                if(VC_state[i] == VC_IDLE){
                    grant = (1 << i);
                    break;
                }
            }
			// make sure the last VC is not issued to flits with VC_class 0
			if (i == ALLOW_VC_NUM - 1)					
                grant = 0;														
        }
		// When VC class is 1, you only allow to use VC 1~(ALLOW_VC-1)
        else{
			for (i = 1; i < ALLOW_VC_NUM; ++i){
                if(VC_state[i] == VC_IDLE){
                    grant = (1 << i );
                    break;
                }
            }
			// if no VC available, make sure grant is not assigned
			if (i == ALLOW_VC_NUM)
                grant = 0;
        }
    }

    if(grant != 0){
        int grant_index = (int)(log2((double)grant));
        if(in->valid && VC_array[grant_index].in_avail){
            //latch the in data
            in_latch[grant_index] = *in;
			in_avail = true;
        }
    }
	else {
		if (in->valid)
			in_avail = false;
		else
			in_avail = true;
	}

    //latch the out avail 
    for(int i = 0; i < VC_NUM; ++i){
		if (VC_array[i].out.valid) {
//			if (!(sw->lookup_in_avail((dir - 1) * VC_NUM + i, VC_array[i].out.dir_out))) {
//				printf("mark\n");
//			}
			out_avail_latch[i] = sw->lookup_in_avail((dir - 1) * VC_NUM + i, VC_array[i].out.dir_out);				// THIS LINE CAUSE ERROR
//			if (out_avail_latch[i] == false) {
//				printf("test\n");
//			}
		}
		else
			out_avail_latch[i] = true;
    }

    //then call all the VC_array(which is FIFO) consume() function, take the input data to FIFO input latch, put the output_available status to the output_available_latch
    for(int i = 0; i < VC_NUM; ++i){
        VC_array[i].consume();
    }

	// Profiling: count how many VCs are being used currently
	num_active_VCs = 0;
	for (int i = 0; i < VC_NUM; ++i) {
		if (VC_state[i] != VC_IDLE) {
			num_active_VCs++;
		}
	}

	// Checking: if more than need VCs has been used
	if (num_active_VCs > ALLOW_VC_NUM) {
		printf("VC use overflow, used %d instead of the limite %d !!!!!!!!!!\n\n\n", num_active_VCs, ALLOW_VC_NUM);
		exit(-1);
	}

	// Store the previous input_prefetch_enable status (not used in the current implementation)
	prev_input_prefetch_enable = input_prefetch_enable;

    return;
}


void VCs::produce(){
    //update all the VC states
    for(int i = 0; i < VC_NUM; ++i){
//	for (int i = 0; i < ALLOW_VC_NUM; ++i) {
        if(VC_state[i] == VC_IDLE){ //empty buffer and No packet is occupying VC  
            if(VC_array[i].in_latch.valid){
                if(VC_array[i].in_avail){
                    if(VC_array[i].in_latch.flit_type == HEAD_FLIT || VC_array[i].in_latch.flit_type == SINGLE_FLIT){
                        VC_state[i] = VC_WAITING_FOR_OVC;
                    }
                    else{			// This one seems redundant, never been reached
                        VC_state[i] = VC_ACTIVE;
                    }
                }
                else{				// This one seems redundant, never been reached
                    VC_state[i] = VC_WAITING_FOR_CREDITS;
                }
            }
            else{
                VC_state[i] = VC_IDLE;
            }
        }
        else if(VC_state[i] == VC_ACTIVE){ // the flit_out has a granted OVC, buffer might be empty
            if(VC_array[i].out_avail_latch){
                if(VC_array[i].usedw == 1){
                    if(VC_array[i].in_latch.valid && (VC_array[i].in_latch.flit_type == HEAD_FLIT || VC_array[i].in_latch.flit_type == SINGLE_FLIT)){
                        VC_state[i] = VC_WAITING_FOR_OVC;
                    }
                    else if(VC_array[i].in_latch.valid){
                        VC_state[i] = VC_ACTIVE;
                    }
                    else if(VC_array[i].out.valid && (VC_array[i].out.flit_type == TAIL_FLIT || VC_array[i].out.flit_type == SINGLE_FLIT)){
                        VC_state[i] = VC_IDLE;
                    }
                    else{
                        VC_state[i] = VC_ACTIVE;
                    }
                }
                else{ //there are at least 2 flits in the VC
                    if(VC_array[i].out.valid && (VC_array[i].out.flit_type == TAIL_FLIT || VC_array[i].out.flit_type == SINGLE_FLIT)){ // the next out flit will be a head flit
                        VC_state[i] = VC_WAITING_FOR_OVC;
                    }
                    else{
                        VC_state[i] = VC_ACTIVE;
                    }
                }
            }
            else{
                VC_state[i] = VC_ACTIVE;//downstream OVC has no credits
            }
        }
        else if(VC_state[i] == VC_WAITING_FOR_OVC){
            if(VC_array[i].out_avail_latch){
				if (VC_array[i].out.flit_type == SINGLE_FLIT)
					VC_state[i] = VC_IDLE;
				else
					VC_state[i] = VC_ACTIVE;
            }
		
            else{
                VC_state[i] = VC_WAITING_FOR_OVC;
            }
        }
    }
    //first call all the VC produce() functions
    for(int i = 0; i < VC_NUM; ++i){
        VC_array[i].produce();
    }
    //update out
    for(int i = 0; i < VC_NUM; ++i){
        out[i] = VC_array[i].out;
    }
    //
    //
    //update in_avail
    if(grant !=0){
        int grant_index = (int)(log2((double)grant));
        in_avail = VC_array[grant_index].in_avail;
    }
    else{
		if(in->valid)
			in_avail = false;
    }
    //wipe all the in_latch
    for(int i = 0; i < VC_NUM; ++i){
        in_latch[i].valid = false;
    }
    return;

}


void VCs::VCs_free(){
	for (int i = 0; i < VC_NUM; ++i){
		VC_array[i].fifo_free();
	}

}

int VCs::count_active_VCs(){
	int ret = 0;
	for (int i = 0; i < VC_NUM; i++){
		if (VC_state[i] == VC_ACTIVE || VC_state[i] == VC_WAITING_FOR_OVC)
			ret++;
	}
	return ret;

}
