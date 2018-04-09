#include "reduction_tree.h"
#include "N_to_1_reductor.h"
#include "flit.h"
#include <stdlib.h>
#include <stdio.h>
#include "define.h"


// Debugging use
// Used for locating the pattern on the input side of each N_to_1_reductor
// Put this function at the beginning of each N_to_1_reductor.produce, take the N_to_1_reductor.in_slot[i] as input flit, scan N_Fan_in (3 or 2) inputs
//**************Notice****************
// The injection_direction should be given by defined symbols like DIR_XPOS, DIR_ZNEG, etc
// In pattern[][][][][], the injection port # will start from 0, however, in the injected flit, it starts from 1, 2, ... to match the defined symbol DIR_XPOS, DIR_YPOS, .....
// Thus, always use symbols like DIR_XPOS, DIR_YPOS, etc to specify the injection_direction input variable
int reduction_tree::report_pattern_on_reductor_input(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* received_flit, int reductor_level, int reductor_id, int input_port_id) {
	flit cur_flit = *received_flit;
	int outgoing_dir = DIR_EJECT;			// by default the outgoing direction is eject, if not recorded in output_port_mapping, then this means that the packet will be ejected here
	int flit_id;
	bool find_the_match = false;

	// First determine if there's a match, if this is a body flit, need to confirm the flit_id is also what we are looking for
	if (cur_flit.inject_dir == injection_direction && cur_flit.src_z == check_z && cur_flit.src_y == check_y && cur_flit.src_x == check_x && cur_flit.flit_type == check_flit_type && cur_flit.packet_id == check_packet_id) {
		// If looking for a HEAD_FLIT or SINGLE_FLIT, then don't need to consider flit_id
		if (check_flit_type == HEAD_FLIT || check_flit_type == SINGLE_FLIT) {
			flit_id = 0;
			find_the_match = true;
		}
		else if (check_flit_type == TAIL_FLIT) {
			flit_id = check_flit_id;
			find_the_match = true;
		}
		else if (check_flit_type == BODY_FLIT) {
			flit_id = check_flit_id;
			// when it's  a body flit, need to check the flit_id
			if (cur_flit.flit_id == check_flit_id) {
				find_the_match = true;
			}
			else
			{
				find_the_match = false;
			}
		}
		else {
			find_the_match = false;
		}

		// When a match is found, print out
		if (find_the_match) {
			char inject_dir[20] = "";
			switch (injection_direction) {
			case DIR_XPOS: sprintf(inject_dir, "%s", "DIR_XPOS"); break;
			case DIR_YPOS: sprintf(inject_dir, "%s", "DIR_YPOS"); break;
			case DIR_ZPOS: sprintf(inject_dir, "%s", "DIR_ZPOS"); break;
			case DIR_XNEG: sprintf(inject_dir, "%s", "DIR_XNEG"); break;
			case DIR_YNEG: sprintf(inject_dir, "%s", "DIR_YNEG"); break;
			case DIR_ZNEG: sprintf(inject_dir, "%s", "DIR_ZNEG"); break;
			}

			char flit_type[20] = "";
			switch (check_flit_type) {
			case 0: sprintf(flit_type, "%s", "HEAD_FLIT"); break;
			case 1: sprintf(flit_type, "%s", "BODY_FLIT"); break;
			case 2: sprintf(flit_type, "%s", "TAIL_FLIT"); break;
			case 3: sprintf(flit_type, "%s", "SINGLE_FLIT"); break;
			}

			// print out information
			printf("^^^^^^^^^^ Reduction_tree.cpp: Pattern[%s][%d][%d][%d][%d], the #%d (%s) flit is on Node(%d,%d,%d)'s Level %d, # %d reductor's input side, input port # is %d\n",
				inject_dir, check_z, check_y, check_x, check_packet_id, flit_id, flit_type, cur_x, cur_y, cur_z, reductor_level, reductor_id, input_port_id);
		}
	}
	return 0;
}



void reduction_tree::reduction_tree_init(int Cur_x, int Cur_y, int Cur_z, int N_Fan_in, int Out_dir, int Level_num, int Mode, int L1_N, int L2_N, int L3_N, flit** In_list, bool* Out_avail){
	cur_x = Cur_x;
	cur_y = Cur_y;
	cur_z = Cur_z;
	
	N_fan_in = N_Fan_in;
	out_dir = Out_dir;
    level_num = Level_num;
    mode = Mode;
    l1_N = L1_N;
    l2_N = L2_N;
    l3_N = L3_N;
    l4_N = 1;
    if(!(flit_in = (flit**)malloc(N_fan_in * sizeof(flit*)))){
		printf("No mem space for %d slots for flit** flit_in in reductor tree in %d dir\n", N_fan_in, out_dir);
        exit(-1);
    }
    
    if(!(in_latch = (flit*)malloc(N_fan_in * sizeof(flit)))){
        printf("No mem space for %d slots for flit* in_latch in reductor in %d dir\n", N_fan_in, out_dir);
        exit(-1);
    }   

    if(!(in_avail = (bool*)malloc(N_fan_in * sizeof(bool)))){
        printf("No mem space for %d slots for bool* in_avail in reductor in %d dir\n", N_fan_in, out_dir);
        exit(-1);
    }
    
    if(!(in_slot = (flit*)malloc(N_fan_in * sizeof(flit)))){
        printf("No mem space for %d slots for N_fan_in* in_slot in reductor in %d dir\n", N_fan_in, out_dir);
        exit(-1);
    }

    for(int i = 0; i < N_fan_in; ++i){
        flit_in[i] = In_list[i];
        in_latch[i].valid = false;
        in_avail[i] = true;
        in_slot[i].valid = false;
    }
    out_avail_latch = true;
    out_avail = Out_avail;
	out.valid = false;

    //then init all the reductors in l1 and l2 and l3
    if(!(l1_reductors = (N_to_1_reductor*)malloc(l1_N * sizeof(N_to_1_reductor)))){
        printf("No mem space for %d l1 reductors for reduction tree on dir %d\n", l1_N, out_dir);
        exit(-1);
    }
    if(!(l2_reductors = (N_to_1_reductor*)malloc(l2_N * sizeof(N_to_1_reductor)))){
        printf("No mem space for %d l2 reductors for reduction tree on dir %d\n", l2_N, out_dir);
        exit(-1);
    } 
    if(!(l3_reductors = (N_to_1_reductor*)malloc(l3_N * sizeof(N_to_1_reductor)))){
        printf("No mem space for %d l2 reductors for reduction tree on dir %d\n", l2_N, out_dir);
        exit(-1);
    }
    
    //then call init functions of all the reductors
    l1_W = N_fan_in / l1_N;
    l2_W = l1_N / l2_N;
    l3_W = l2_N / l3_N;
    l4_W = l3_N;
    


    for(int i = 0; i < l1_N; ++i){
		l1_reductors[i].alloc(l1_W);
    }
    for(int i = 0; i < l2_N; ++i){
        l2_reductors[i].alloc(l2_W);
    }
    for(int i = 0; i < l3_N; ++i){
        l3_reductors[i].alloc(l3_W);
    }
    l4_reductor.alloc(l4_W);

    if(!(flit_to_l1 = (flit**)malloc(N_fan_in * sizeof(flit*)))){
        printf("No mem space of %d pointers for to l1 reductors\n", N_fan_in);
        exit(-1);
    }
  
    if(!(flit_l1_to_l2 = (flit**)malloc(l1_N * sizeof(flit*)))){
        printf("No mem space of %d pointers for l1 to l2 reductors\n", l1_N);
        exit(-1);
    }

    if(!(flit_l2_to_l3 = (flit**)malloc(l2_N * sizeof(flit*)))){
        printf("No mem space of %d pointers for l2 to l3 reductors\n", l2_N);
        exit(-1);
    }

    if(!(flit_l3_to_l4 = (flit**)malloc(l3_N * sizeof(flit*)))){
        printf("No mem space of %d pointers for l3 to l4 reductors\n", l3_N);
        exit(-1);
    }

    if(!(in_avail_l2_to_l1 = (bool**)malloc(l1_N * sizeof(bool*)))){
		printf("No mem space for %d pointers for l2 to l1 in_avail\n", l1_N);
        exit(-1);
    }
 
    if(!(in_avail_l3_to_l2 = (bool**)malloc(l2_N * sizeof(bool*)))){
		printf("No mem space for %d pointers for l3 to l2 in_avail\n", l2_N);
        exit(-1);
    }
 
    if(!(in_avail_l4_to_l3 = (bool**)malloc(l3_N * sizeof(bool*)))){
		printf("No mem space for %d pointers for l3 to l2 in_avail\n", l3_N);
        exit(-1);
    }

    if(!(out_avail_to_l4 = (bool*)malloc(sizeof(bool)))){
        printf("No mem space for 1 pointer for to l4 out_avail\n");
        exit(-1);
    }

    for(int i = 0; i < N_fan_in; ++i){
        flit_to_l1[i] = &in_latch[i];
    }

    for(int i = 0; i < l1_N; ++i){
        flit_l1_to_l2[i] = &(l1_reductors[i].out);
        in_avail_l2_to_l1[i] = &(l2_reductors[i / l2_W].in_avail[i % l2_W]);
    }
    for(int i = 0; i < l2_N; ++i){
        flit_l2_to_l3[i] = &(l2_reductors[i].out);
        in_avail_l3_to_l2[i] = &(l3_reductors[i / l3_W].in_avail[i % l3_W]);
    }
    for(int i = 0; i < l3_N; ++i){
        flit_l3_to_l4[i] = &(l3_reductors[i].out);
        in_avail_l4_to_l3[i] = &(l4_reductor.in_avail[i]);
    }

    out_avail_to_l4 = &out_avail_latch; 
    
    for(int i = 0; i < l1_N; ++i){
        l1_reductors[i].N_to_1_reductor_init(cur_x, cur_y, cur_z, out_dir, 1, i, mode, &(flit_to_l1[i * l1_W]), in_avail_l2_to_l1[i]);
    }
    for(int i = 0; i < l2_N; ++i){
		l2_reductors[i].N_to_1_reductor_init(cur_x, cur_y, cur_z, out_dir, 2, i, mode, &(flit_l1_to_l2[i * l2_W]), in_avail_l3_to_l2[i]);
    }
    for(int i = 0; i < l3_N; ++i){
		l3_reductors[i].N_to_1_reductor_init(cur_x, cur_y, cur_z, out_dir, 3, i, mode, &(flit_l2_to_l3[i * l3_W]), in_avail_l4_to_l3[i]);
    }   

	l4_reductor.N_to_1_reductor_init(cur_x, cur_y, cur_z, out_dir, 4, 0, mode, flit_l3_to_l4, out_avail_to_l4);

}

void reduction_tree::consume(){
    //latch the out_avail
    out_avail_latch = *out_avail;
    //latch the flit_in
    for(int i = 0; i < N_fan_in; ++i){
		if (flit_in[i]->valid && flit_in[i]->dir_out == out_dir)
			in_latch[i] = *(flit_in[i]);
		else
			in_latch[i].valid = false;
    }

    //call all the subentities consume
    for(int i = 0; i < l1_N; ++i){
        l1_reductors[i].consume();
    }
    for(int i = 0; i < l2_N; ++i){
        l2_reductors[i].consume();
    }
    for(int i = 0; i < l3_N; ++i){
        l3_reductors[i].consume();
    }
    l4_reductor.consume();

}

int reduction_tree::produce(){
     //call all the subentities produce()

    for(int i = 0; i < l1_N; ++i){
/*
		////////////////////////////////////////////////////////////////////////////////////////////////
		// Debugging printout for tracing the path of a certain pattern
		// Used for locating the pattern on the input side of each reductors
		////////////////////////////////////////////////////////////////////////////////////////////////
		int pattern_check_injection_direction = DIR_XPOS;				// Use symbols like DIR_XPOS, DIR_YPOS, etc to specify, as the flit.injection_direction starts from 1. Ex.: if checking pattern[1][x][x][x][x], then set this as DIR_YPOS 
		int pattern_check_z = 0;
		int pattern_check_y = 0;
		int pattern_check_x = 0;
		int pattern_check_packet_id = 0;
		int pattern_check_flit_id = 0;
		int pattern_check_flit_type = HEAD_FLIT;
		for (int kk = 0; kk < 3; ++kk) {
			if (l1_reductors[i].in_slot[kk].valid) {
				report_pattern_on_reductor_input(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, pattern_check_flit_type, pattern_check_flit_id, &l1_reductors[i].in_slot[kk], 1, i, kk);
			}
		}
*/
		if (l1_reductors[i].produce() == -1)
			return -1;
    }
    for(int i = 0; i < l2_N; ++i){
/*
		////////////////////////////////////////////////////////////////////////////////////////////////
		// Debugging printout for tracing the path of a certain pattern
		// Used for locating the pattern on the input side of each reductors
		////////////////////////////////////////////////////////////////////////////////////////////////
		int pattern_check_injection_direction = DIR_XPOS;				// Use symbols like DIR_XPOS, DIR_YPOS, etc to specify, as the flit.injection_direction starts from 1. Ex.: if checking pattern[1][x][x][x][x], then set this as DIR_YPOS 
		int pattern_check_z = 0;
		int pattern_check_y = 0;
		int pattern_check_x = 0;
		int pattern_check_packet_id = 0;
		int pattern_check_flit_id = 0;
		int pattern_check_flit_type = HEAD_FLIT;
		for (int kk = 0; kk < 3; ++kk) {
			if (l1_reductors[i].in_slot[kk].valid) {
				report_pattern_on_reductor_input(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, pattern_check_flit_type, pattern_check_flit_id, &l2_reductors[i].in_slot[kk], 2, i, kk);
			}
		}
*/
        if(l2_reductors[i].produce() == -1)
			return -1;
    }
    for(int i = 0; i < l3_N; ++i){
/*
		////////////////////////////////////////////////////////////////////////////////////////////////
		// Debugging printout for tracing the path of a certain pattern
		// Used for locating the pattern on the input side of each reductors
		////////////////////////////////////////////////////////////////////////////////////////////////
		int pattern_check_injection_direction = DIR_XPOS;				// Use symbols like DIR_XPOS, DIR_YPOS, etc to specify, as the flit.injection_direction starts from 1. Ex.: if checking pattern[1][x][x][x][x], then set this as DIR_YPOS 
		int pattern_check_z = 0;
		int pattern_check_y = 0;
		int pattern_check_x = 0;
		int pattern_check_packet_id = 0;
		int pattern_check_flit_id = 0;
		int pattern_check_flit_type = HEAD_FLIT;
		for (int kk = 0; kk < 3; ++kk) {
			if (l1_reductors[i].in_slot[kk].valid) {
				report_pattern_on_reductor_input(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, pattern_check_flit_type, pattern_check_flit_id, &l3_reductors[i].in_slot[kk], 3, i, kk);
			}
		}
*/
		if (l3_reductors[i].produce() == -1)
			return -1;
    }
/*
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Debugging printout for tracing the path of a certain pattern
	// Used for locating the pattern on the input side of each reductors
	////////////////////////////////////////////////////////////////////////////////////////////////
	int pattern_check_injection_direction = DIR_XPOS;				// Use symbols like DIR_XPOS, DIR_YPOS, etc to specify, as the flit.injection_direction starts from 1. Ex.: if checking pattern[1][x][x][x][x], then set this as DIR_YPOS 
	int pattern_check_z = 0;
	int pattern_check_y = 0;
	int pattern_check_x = 0;
	int pattern_check_packet_id = 0;
	int pattern_check_flit_id = 0;
	int pattern_check_flit_type = HEAD_FLIT;
	for (int kk = 0; kk < 2; ++kk) {
		if (l4_reductor.in_slot[kk].valid) {
			report_pattern_on_reductor_input(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, pattern_check_flit_type, pattern_check_flit_id, &l4_reductor.in_slot[kk], 4, 0, kk);
		}
	}
*/
    if(l4_reductor.produce() == -1)
		return -1;

    //update in_avail and out
    for(int i = 0; i < N_fan_in; ++i){
        in_avail[i] = l1_reductors[i / l1_W].in_avail[i % l1_W];
    }
    out = l4_reductor.out;
	return 0;
}
void reduction_tree::reduction_tree_free(){
	for(int i = 0; i < l1_N; ++i){
        l1_reductors[i].N_to_1_reductor_free();
    }
    for(int i = 0; i < l2_N; ++i){
		l2_reductors[i].N_to_1_reductor_free();
    }
    for(int i = 0; i < l3_N; ++i){
		l3_reductors[i].N_to_1_reductor_free();
    }
	l4_reductor.N_to_1_reductor_free();
    free(l1_reductors);
    free(l2_reductors);
    free(l3_reductors);

    free(flit_to_l1);
    free(flit_l1_to_l2);
    free(flit_l2_to_l3);
    free(flit_l3_to_l4);
    free(in_avail_l2_to_l1);
    free(in_avail_l3_to_l2);
    free(in_avail_l4_to_l3);
 //   free(out_avail_to_l4);
    
    free(flit_in);
    free(in_latch);
    free(in_avail);
    free(in_slot);

    return;

}






