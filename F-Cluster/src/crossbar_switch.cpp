#include "define.h"
#include "flit.h"
#include "crossbar_switch.h"
#include<stdio.h>
#include<stdlib.h>


// Debugging use
// Used for locating the pattern on the input side of crossbar_switch
// Put this function at the beginning of corssbar_switch.produce, take the in_latch[i] as input flit, scan N_FAN_IN in_latches
//**************Notice****************
// The injection_direction should be given by defined symbols like DIR_XPOS, DIR_ZNEG, etc
// In pattern[][][][][], the injection port # will start from 0, however, in the injected flit, it starts from 1, 2, ... to match the defined symbol DIR_XPOS, DIR_YPOS, .....
// Thus, always use symbols like DIR_XPOS, DIR_YPOS, etc to specify the injection_direction input variable
int crossbar_switch::report_pattern_received(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* received_flit, int input_port_number) {
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
			printf("xxxxxxxx Crossbar_switch.cpp: Pattern[%s][%d][%d][%d][%d], src(%d,%d,%d), dst(%d,%d,%d), the #%d (%s) flit is on Node(%d,%d,%d) crossbar input, input port # is %d\n",
				inject_dir, check_z, check_y, check_x, check_packet_id, cur_flit.src_x, cur_flit.src_y, cur_flit.src_z, cur_flit.dst_x, cur_flit.dst_y, cur_flit.dst_z, flit_id, flit_type, cur_x, cur_y, cur_z, input_port_number);
		}
	}
	return 0;
}


// Used for locating the pattern on the output side of crossbar_switch
// Put this function at the end of corssbar_switch.produce, take the out[i] as input flit, scan PORT_NUM output ports
//**************Notice****************
// The injection_direction should be given by defined symbols like DIR_XPOS, DIR_ZNEG, etc
// In pattern[][][][][], the injection port # will start from 0, however, in the injected flit, it starts from 1, 2, ... to match the defined symbol DIR_XPOS, DIR_YPOS, .....
// Thus, always use symbols like DIR_XPOS, DIR_YPOS, etc to specify the injection_direction input variable
int crossbar_switch::report_pattern_left(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* received_flit, int output_port_number) {
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

			char outgoing_dir[20] = "";
			switch (output_port_number) {
			case 0: sprintf(outgoing_dir, "%s", "DIR_XPOS"); break;
			case 1: sprintf(outgoing_dir, "%s", "DIR_YPOS"); break;
			case 2: sprintf(outgoing_dir, "%s", "DIR_ZPOS"); break;
			case 3: sprintf(outgoing_dir, "%s", "DIR_XNEG"); break;
			case 4: sprintf(outgoing_dir, "%s", "DIR_YNEG"); break;
			case 5: sprintf(outgoing_dir, "%s", "DIR_ZNEG"); break;
			}

			// print out information
			printf("oooooooooo Crossbar_switch.cpp: Pattern[%s][%d][%d][%d][%d], src(%d,%d,%d), dst(%d,%d,%d), the #%d (%s) flit is on Node(%d,%d,%d) corssbar output, outgoing port # is %s\n",
				inject_dir, check_z, check_y, check_x, check_packet_id, cur_flit.src_x, cur_flit.src_y, cur_flit.src_z, cur_flit.dst_x, cur_flit.dst_y, cur_flit.dst_z, flit_id, flit_type, cur_x, cur_y, cur_z, outgoing_dir);
		}
	}
	return 0;
}

void crossbar_switch::crossbar_switch_init(int Cur_x, int Cur_y, int Cur_z, int Mode, flit** In_list, bool** Out_avail){
	
	cur_x = Cur_x;
	cur_y = Cur_y;
	cur_z = Cur_z;


	N_fan_in = N_FAN_IN;											// PORT_NUM * VC_NUM
	N_fan_out = PORT_NUM;
    mode = Mode;
	for(int j = 0; j < N_FAN_IN; ++j)
		in_latch[j].valid = false;

    for(int i = 0; i < PORT_NUM; ++i){
        for(int j = 0; j < N_FAN_IN; ++j){
            in_avail[i][j] = true;
        }
        out[i].valid = false;
        out_avail_latch[i] = true;
    }
    for(int i = 0; i < N_FAN_IN; ++i){
        flit_in[i] = In_list[i];
    }
    for(int i = 0; i < PORT_NUM; ++i){
        out_avail[i] = Out_avail[i];
    }


    for(int j = 0; j < N_FAN_IN; ++j){
        in_list_to_tree[j] = &in_latch[j];        
    }
  
    for(int i = 0; i < PORT_NUM; ++i){
        out_avail_to_tree[i] = &out_avail_latch[i];
    }
    


    for(int i = 0; i < PORT_NUM; ++i){
        tree_list[i].reduction_tree_init(cur_x, cur_y, cur_z, N_fan_in, i + 1, 4, mode, 18, 6, 2, in_list_to_tree, out_avail_to_tree[i]);
    }
    

}

void crossbar_switch::consume(){
    //latch all the in data
	for (int i = 0; i < N_FAN_IN; ++i) {
		in_latch[i] = *(flit_in[i]);
	}

    //latch all the out_avail
    for(int i = 0; i < PORT_NUM; ++i){
        out_avail_latch[i] = *(out_avail[i]);
    }
    
    //call all the consume functions of all the reduction tree
    
    for(int i = 0; i < PORT_NUM; ++i){
        tree_list[i].consume();
    }
    return;
}

int crossbar_switch::produce(){
/*
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Debugging printout for tracing the path of a certain pattern
	// Used for locating the pattern on the input side of crossbar_switch
	////////////////////////////////////////////////////////////////////////////////////////////////
	int pattern_check_injection_direction = DIR_XPOS;				// Use symbols like DIR_XPOS, DIR_YPOS, etc to specify, as the flit.injection_direction starts from 1. Ex.: if checking pattern[1][x][x][x][x], then set this as DIR_YPOS 
	int pattern_check_z = 0;
	int pattern_check_y = 0;
	int pattern_check_x = 0;
	int pattern_check_packet_id = 0;
	int pattern_check_flit_id = 0;
	int pattern_check_flit_type = HEAD_FLIT;
	for (int i = 0; i < N_FAN_IN; ++i) {
		if (in_latch[i].valid) {
			report_pattern_received(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, pattern_check_flit_type, pattern_check_flit_id, &in_latch[i], i);
			//report_pattern_received(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, BODY_FLIT, 1, &in_latch[i], i);
			//report_pattern_received(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, BODY_FLIT, 2, &in_latch[i], i);
			//report_pattern_received(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, TAIL_FLIT, 3, &in_latch[i], i);
		}
	}
*/

    //call all the produce functions of all the reduction trees
    for(int i = 0; i < PORT_NUM; ++i){
        if(tree_list[i].produce() == -1)
			return -1;
    }

    //update the out 
    for(int i = 0; i < PORT_NUM; ++i){
        out[i] = tree_list[i].out;
    }
/*
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Debugging printout for tracing the path of a certain pattern
	// Used for locating the pattern on the output side of crossbar_switch
	////////////////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < PORT_NUM; ++i) {
		if (out[i].valid) {
			report_pattern_left(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, pattern_check_flit_type, pattern_check_flit_id, &out[i], i);
		}
	}
*/

    //the in_avail belongs to the lookup_in_avail
	return 0;
}

bool crossbar_switch::lookup_in_avail(int port_id, int out_dir){
    if(out_dir > 6 || out_dir < 1){
		printf("Switch allocation error!!\n");
        printf("dir %d is wrong, port_id is %d\n", out_dir, port_id);
        exit(-1);
    }
    if(port_id >= N_FAN_IN || port_id < 0){
		printf("Switch allocation error!!\n");
        printf("port_id is wrong\n");
        exit(-1);
    }
    return tree_list[out_dir - 1].in_avail[port_id];
}

void crossbar_switch::crossbar_switch_free(){
    for(int i = 0; i < PORT_NUM; ++i){
        tree_list[i].reduction_tree_free();
    }
}
