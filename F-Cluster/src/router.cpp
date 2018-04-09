#include "router.h"
#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>

// datapath: input_buffer -> RC -> VC -> SW
// Before RC performs calculation, first check if there's slots available in VC, if so, then send to calculation


// Debugging use
// Used for locating the pattern on the receiving side cycle by cycle
// Put this function after the RC and SA is done, so it will print out which direction the packet will be send to
//**************Notice****************
// The injection_direction should be given by defined symbols like DIR_XPOS, DIR_ZNEG, etc
// In pattern[][][][][], the injection port # will start from 0, however, in the injected flit, it starts from 1, 2, ... to match the defined symbol DIR_XPOS, DIR_YPOS, .....
// Thus, always use symbols like DIR_XPOS, DIR_YPOS, etc to specify the injection_direction input variable
int router::report_pattern_received(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* received_flit, int incoming_dir) {
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
			flit_id = packet_size - 1;
			find_the_match = true;
		}
		else if (check_flit_type == BODY_FLIT) {
			flit_id = check_flit_id;
			if (flit_id > packet_size || flit_id <= 0) {
				printf("Debugging fuction: report_pattern_location fuction error out! When checking the body_flit, the given flit_id is either larger than the packet size, or the flit_id == 0!\n");
				exit(-1);
			}
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

			char incoming[20] = "";
			switch (incoming_dir) {
			case 0: sprintf(incoming, "%s", "DIR_XPOS"); break;
			case 1: sprintf(incoming, "%s", "DIR_YPOS"); break;
			case 2: sprintf(incoming, "%s", "DIR_ZPOS"); break;
			case 3: sprintf(incoming, "%s", "DIR_XNEG"); break;
			case 4: sprintf(incoming, "%s", "DIR_YNEG"); break;
			case 5: sprintf(incoming, "%s", "DIR_ZNEG"); break;
			}

			char flit_type[20] = "";
			switch (check_flit_type) {
			case 0: sprintf(flit_type, "%s", "HEAD_FLIT"); break;
			case 1: sprintf(flit_type, "%s", "BODY_FLIT"); break;
			case 2: sprintf(flit_type, "%s", "TAIL_FLIT"); break;
			case 3: sprintf(flit_type, "%s", "SINGLE_FLIT"); break;
			}

			// print out information
			printf("***** Router.cpp: Cycle: %d, Pattern[%s][%d][%d][%d][%d], src(%d,%d,%d), dst(%d,%d,%d), the #%d (%s) flit is at Node (%d, %d, %d), incoming direction is %s\n",
				app_core.cycle_counter, inject_dir, check_z, check_y, check_x, check_packet_id, cur_flit.src_x, cur_flit.src_y, cur_flit.src_z, cur_flit.dst_x, cur_flit.dst_y, cur_flit.dst_z, flit_id, flit_type, cur_x, cur_y, cur_z, incoming);
		}
	}
	return 0;
}


// Debugging use
// Used for checking at which cycle the target flit has been injected at a certain node
// Put this function at the end of router production, the input flit should connect to appcore_inject[i]
//**************Notice****************
// The injection_direction should be given by defined symbols like DIR_XPOS, DIR_ZNEG, etc
// In pattern[][][][][], the injection port # will start from 0, however, in the injected flit, it starts from 1, 2, ... to match the defined symbol DIR_XPOS, DIR_YPOS, .....
// Thus, always use symbols like DIR_XPOS, DIR_YPOS, etc to specify the injection_direction input variable
int router::report_pattern_injected(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* injecting_flit) {
	flit cur_flit = *injecting_flit;
	int outgoing_dir = DIR_EJECT;			// by default the outgoing direction is eject, if not recorded in output_port_mapping, then this means that the packet will be ejected here
	int flit_id;
	bool find_the_match = false;

	if (cur_flit.valid) {		// Only perform checking when the flit is valid
		// First determine if there's a match, if this is a body flit, need to confirm the flit_id is also what we are looking for
		if (cur_flit.inject_dir == injection_direction && cur_flit.src_z == check_z && cur_flit.src_y == check_y && cur_flit.src_x == check_x && cur_flit.flit_type == check_flit_type && cur_flit.packet_id == check_packet_id) {
			// If looking for a HEAD_FLIT or SINGLE_FLIT, then don't need to consider flit_id
			if (check_flit_type == HEAD_FLIT || check_flit_type == SINGLE_FLIT) {
				flit_id = 0;
				find_the_match = true;
			}
			else if (check_flit_type == TAIL_FLIT) {
				flit_id = packet_size - 1;
				find_the_match = true;
			}
			else if (check_flit_type == BODY_FLIT) {
				flit_id = check_flit_id;
				if (flit_id > packet_size || flit_id <= 0) {
					printf("Debugging fuction: report_pattern_location fuction error out! When checking the body_flit, the given flit_id is either larger than the packet size, or the flit_id == 0!\n");
					exit(-1);
				}
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
				printf("@@@@@ Router.cpp: Cycle: %d, Pattern[%s][%d][%d][%d][%d], src(%d,%d,%d), dst(%d,%d,%d), the #%d (%s) flit is injected on Port: %s, Node (%d, %d, %d)\n",
					app_core.cycle_counter, inject_dir, check_z, check_y, check_x, check_packet_id, cur_flit.src_x, cur_flit.src_y, cur_flit.src_z, cur_flit.dst_x, cur_flit.dst_y, cur_flit.dst_z, flit_id, flit_type, inject_dir, cur_x, cur_y, cur_z);
			}
		}
	}
	return 0;
}

// Debugging use
// Used for locating the pattern when it leave a node
// Put this function after the output port in router.cpp is assigned, thus it will reflect when the flit has left the node
//**************Notice****************
// The injection_direction should be given by defined symbols like DIR_XPOS, DIR_ZNEG, etc
// In pattern[][][][][], the injection port # will start from 0, however, in the injected flit, it starts from 1, 2, ... to match the defined symbol DIR_XPOS, DIR_YPOS, .....
// Thus, always use symbols like DIR_XPOS, DIR_YPOS, etc to specify the injection_direction input variable
int router::report_pattern_sent(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* received_flit) {
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
			flit_id = packet_size - 1;
			find_the_match = true;
		}
		else if (check_flit_type == BODY_FLIT) {
			flit_id = check_flit_id;
			if (flit_id > packet_size || flit_id <= 0) {
				printf("Debugging fuction: report_pattern_location fuction error out! When checking the body_flit, the given flit_id is either larger than the packet size, or the flit_id == 0!\n");
				exit(-1);
			}
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
			switch (cur_flit.dir_out) {
			case DIR_XPOS: sprintf(outgoing_dir, "%s", "DIR_XPOS"); break;
			case DIR_YPOS: sprintf(outgoing_dir, "%s", "DIR_YPOS"); break;
			case DIR_ZPOS: sprintf(outgoing_dir, "%s", "DIR_ZPOS"); break;
			case DIR_XNEG: sprintf(outgoing_dir, "%s", "DIR_XNEG"); break;
			case DIR_YNEG: sprintf(outgoing_dir, "%s", "DIR_YNEG"); break;
			case DIR_ZNEG: sprintf(outgoing_dir, "%s", "DIR_ZNEG"); break;
			}

			// print out information
			printf("$$$$$$ Router.cpp: Cycle: %d, Pattern[%s][%d][%d][%d][%d], src(%d,%d,%d), dst(%d,%d,%d), the #%d (%s) flit left Node (%d, %d, %d), outgoing direction is %s\n",
				app_core.cycle_counter, inject_dir, check_z, check_y, check_x, check_packet_id, cur_flit.src_x, cur_flit.src_y, cur_flit.src_z, cur_flit.dst_x, cur_flit.dst_y, cur_flit.dst_z, flit_id, flit_type, cur_x, cur_y, cur_z, outgoing_dir);
		}
	}
	return 0;
}

void router::router_init(int Cur_x, int Cur_y, int Cur_z, int SA_Mode, int Routing_mode, int Injection_mode, flit** In, flit** Inject, int Injection_gap, int Packet_size, int Allow_VC_Num){
    cur_x = Cur_x;
    cur_y = Cur_y;
    cur_z = Cur_z;

	allow_vc_num = Allow_VC_Num;

    SA_mode = SA_Mode;
    routing_mode = Routing_mode;
	injection_mode = Injection_mode;

	packet_size = Packet_size;

    credit_period_counter = 0;

    for(int i = 0; i < PORT_NUM; ++i){
        in[i] = In[i];
        inject[i] = Inject[i];
        upstream_credits[i] = 0;
        downstream_credits[i] = IN_Q_SIZE - 1;
        out[i].valid = false;
        eject[i].valid = false;
        inject_avail[i] = true;
		inject_latch[i].valid = false;
		in_latch[i].valid = false;

		for (int vc_counter = 0; vc_counter < VC_NUM; ++vc_counter) {
			downstream_vc_credits[i][vc_counter] = VC_SIZE - 1;
		}
    }

	for (int i = 0; i < PORT_NUM; ++i) {
		eject_ptrs[i] = &eject[i];
		inject_avail_ptrs[i] = &out_avail_for_inject[i];
	}

	app_core.local_unit_init(cur_x, cur_y, cur_z, injection_mode, Injection_gap, Packet_size, PACKET_NUM, eject_ptrs, inject_avail_ptrs, routing_mode, SA_mode);

    //allocate space for those subentities need dynamic allocation

    //init and connect all the subentities
    for(int i = 0; i < PORT_NUM; ++i){
        in_avail_from_RC[i] = &(VCs_list[i].in_avail);																			// connect 
		// before send data from input_buffer, first check if there's VC available, cause once send to RC for calculation, it need to send to a VC
        input_buffer_list[i].fifo_init(IN_Q_SIZE, &in_latch[i], in_avail_from_RC[i]);															// input buffer initilize
    }

    for(int i = 0; i < PORT_NUM; ++i){
        flit_list_to_RC[i] = &(input_buffer_list[i].out);
        in_avail_from_VA[i] = &(VCs_list[i].in_avail);
		// RC part: take input from input_buffer, output to VC, but need to check if there's VC available in advance
        RC_list[i].routing_comp_init(cur_x, cur_y, cur_z, i + 1, routing_mode, flit_list_to_RC[i], in_avail_from_VA[i], &downstream_credits[0], &downstream_credits[1], &downstream_credits[2], &downstream_credits[3], &downstream_credits[4], &downstream_credits[5]);
    }

    for(int i = 0; i < PORT_NUM; ++i){
        flit_list_to_VA[i] = &(RC_list[i].out);
        VCs_list[i].VCs_init(i + 1, flit_list_to_VA[i], &xbar, allow_vc_num);
    }

    for(int i = 0; i < PORT_NUM * VC_NUM; ++i){
        flit_list_to_SA[i] = &(VCs_list[i / VC_NUM].out[i % VC_NUM]);
    }
    for(int i = 0; i < PORT_NUM; ++i){
        in_avail_from_ST[i] = &(out_avail_for_passthru[i]); //initially this is for injetion   
    }
	xbar.crossbar_switch_init(cur_x, cur_y, cur_z, SA_mode, flit_list_to_SA, in_avail_from_ST);


    //init all avails
    for(int i = 0; i < PORT_NUM; ++i){
        occupy_by_inject[i] = false;
		occupy_by_passthru[i] = false;
        downstream_avail[i] = true;
        out_avail_for_passthru[i] = true;
        out_avail_for_inject[i] = true;
    }

}



int router::consume(){
    //latch in and inject
    for(int i = 0; i < PORT_NUM; ++i){
		// take in the credit from downsteam node
        if(in[i]->valid && in[i]->flit_type == CREDIT_FLIT){
            downstream_credits[i] = in[i]->payload;
			// assign the downstream VC credits
			for (int vc_counter = 0; vc_counter < VC_NUM; ++vc_counter) {
				downstream_vc_credits[i][vc_counter] = in[i]->vc_credit[vc_counter];
			}
            in_latch[i].valid = false;
        }
        else{
            in_latch[i] = *(in[i]);
        }
      //  inject_latch[i] = *(inject[i]);
    }
    
/*
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Debugging printout for tracing the path of a certain pattern
	// Only check this after the RC and SA is done so it will give the routing information
	////////////////////////////////////////////////////////////////////////////////////////////////
	int pattern_check_injection_direction = DIR_XPOS;				// Use symbols like DIR_XPOS, DIR_YPOS, etc to specify, as the flit.injection_direction starts from 1. Ex.: if checking pattern[1][x][x][x][x], then set this as DIR_YPOS 
	int pattern_check_z = 0;
	int pattern_check_y = 5;
	int pattern_check_x = 3;
	int pattern_check_packet_id = 0;
	int pattern_check_flit_id = 0;
	int pattern_check_flit_type = HEAD_FLIT;
	for (int i = 0; i < PORT_NUM; ++i) {
		if (in_latch[i].valid) {
			report_pattern_received(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, pattern_check_flit_type, 0, &in_latch[i], i);
			//report_pattern_received(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, BODY_FLIT, 1, &in_latch[i], i);
			//report_pattern_received(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, BODY_FLIT, 2, &in_latch[i], i);
			//report_pattern_received(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, TAIL_FLIT, 3, &in_latch[i], i);
		}
	}
*/
	////////////////////////////////////////////////
    //call all the consume for the subentities
	////////////////////////////////////////////////
	// 1, first, do VC allocation based on RC raw output, no need to wait one cycle for writing into in_latch
	// 2, then take in incoming flits from RC output when the current packet is assigned a VC and there's avaiable space in that VC buffer
	for (int i = 0; i < PORT_NUM; ++i) {
		VCs_list[i].consume();
	}

	// Take link input into the FIFO
    for(int i = 0; i < PORT_NUM; ++i){
        input_buffer_list[i].consume();
    }

	// Takes input buffer output into the in_latch (all the RC calculation is based on in_latch)
    for(int i = 0; i < PORT_NUM; ++i){
        RC_list[i].consume();
    }


    xbar.consume();  
	if(app_core.consume() == -1)
		return -1;
	return 0;


}


int router::produce(){
	//////////////////////////////////////////////////////////////////////////////////////
	// enable prefetching when deadlock occurs
	//////////////////////////////////////////////////////////////////////////////////////
	
	for (int i = 0; i < PORT_NUM; ++i) {
		// if the fifo has been blocked for too long, then start to peek into the FIFO and trying to locate a flit that will use the idle VC
		// perform the check when free period reach a threshold, and the input buffer should not in pre fetch mode, plus the current packet has not been assigned a VC
		if (input_buffer_list[i].fifo_freeze_counter > FIFO_BLOCK_THRESHOLD && !input_buffer_list[i].pre_fetch_enable && input_buffer_list[i].usedw > 0 && VCs_list[i].grant == 0) {
			// fetch the current head and tail pointer
			int probe_head_ptr = input_buffer_list[i].head_ptr;
			int probe_tail_ptr = input_buffer_list[i].tail_ptr;
			int buffer_size = input_buffer_list[i].size;
			// only perform checking if there's vacant VC
			// first check if the VC list has a vacant VC, usually when deadlock occurs, it's either the first or the last VC that's not been used
			if (VCs_list[i].VC_state[0] == VC_IDLE || VCs_list[i].VC_state[allow_vc_num - 1] == VC_IDLE) {		// check the class 0 and class 1 VC's state
				// get the expecting class we want to peek
				bool expecting_class = (VCs_list[i].VC_state[allow_vc_num - 1] == VC_IDLE);
				while (probe_head_ptr != probe_tail_ptr) {
					// locate a packet header inside the input buffer
					flit peek_flit = input_buffer_list[i].flit_array[probe_head_ptr];				// assign the peek flit
					// only perform checking if the current peek flit is valid (rare case that it may peek the flit that is on the tail which is invalid)
					if (peek_flit.valid) {
						if (peek_flit.flit_type == HEAD_FLIT || peek_flit.flit_type == SINGLE_FLIT) {
							// perform routing computation first to get the outgoing direction
							int dir;
							if (routing_mode == ROUTING_DOR_XYZ) {
								dir = RC_list[i].DOR_XYZ(cur_x, cur_y, cur_z, peek_flit.dst_x, peek_flit.dst_y, peek_flit.dst_z);
							}
							else if (routing_mode == ROUTING_RCA) {
								dir = RC_list[i].RCA(i + 1, cur_x, cur_y, cur_z, peek_flit.dst_x, peek_flit.dst_y, peek_flit.dst_z);
							}
							else if (routing_mode == ROUTING_ROMM) {
								dir = RC_list[i].ROMM(i + 1, cur_x, cur_y, cur_z, peek_flit.dst_x, peek_flit.dst_y, peek_flit.dst_z);
							}
							else if (routing_mode == ROUTING_O1TURN) {
								int new_path_id;
								dir = RC_list[i].O1TURN(i + 1, cur_x, cur_y, cur_z, peek_flit.dst_x, peek_flit.dst_y, peek_flit.dst_z, peek_flit.O1TURN_id, &new_path_id);
							}
							else if (routing_mode == ROUTING_RLB_XYZ) {
								int new_path_id;
								dir = RC_list[i].RLB(i + 1, cur_x, cur_y, cur_z, peek_flit.dst_x, peek_flit.dst_y, peek_flit.dst_z);
							}

							// perform the calculation for VC class change
							int dir_in = i + 1;
							bool new_VC_class;
							bool old_VC_class = peek_flit.VC_class;
							// When turn to another dimension
							if (dir_in != dir + 3 && dir != dir_in + 3) {
								if (dir == DIR_XPOS || dir == DIR_YPOS || dir == DIR_ZPOS)
									new_VC_class = 0;
								else
									new_VC_class = 1;
							}
							// When the packet still going down the same dimension
							else {
								switch (dir) {
								case DIR_XPOS:
									if (cur_x == 0)
										new_VC_class = 1;
									else
										new_VC_class = old_VC_class;
									break;
								case DIR_XNEG:
									if (cur_x == XSIZE - 1)
										new_VC_class = 0;
									else
										new_VC_class = old_VC_class;
									break;
								case DIR_YPOS:
									if (cur_y == 0)
										new_VC_class = 1;
									else
										new_VC_class = old_VC_class;
									break;
								case DIR_YNEG:
									if (cur_y == YSIZE - 1)
										new_VC_class = 0;
									else
										new_VC_class = old_VC_class;
									break;
								case DIR_ZPOS:
									if (cur_z == 0)
										new_VC_class = 1;
									else
										new_VC_class = old_VC_class;
									break;
								case DIR_ZNEG:
									if (cur_z == ZSIZE - 1)
										new_VC_class = 0;
									else
										new_VC_class = old_VC_class;
									break;
								default:
									new_VC_class = old_VC_class;
								}
							}

							// check the current peeking packet is the one we need
							// !!!! Make sure the packet that's gonna prefetched is inside the buffer entirely (especially for small packets)
							if (new_VC_class == expecting_class) {
								// get the distance between probe head pointer and tail pointer, use this variable to make sure that the entire packet is currently in the buffer
								int dst_head_tail = (probe_tail_ptr >= probe_head_ptr) ? (probe_tail_ptr - probe_head_ptr) : (probe_tail_ptr - probe_head_ptr + buffer_size);
								// only perform prefetching when the entire packet is currently inside the buffer
								if (dst_head_tail >= packet_size) {
									input_buffer_list[i].pre_fetch_enable = true;										// set the pre-fetch flag for input buffer
									input_buffer_list[i].peek_head_ptr = probe_head_ptr;							// assign the pre fetching header
									RC_list[i].input_prefetch_enable = true;												// set the pre-fetch flag for RC

									// exit the while loop once the target packet is found, move on to check the next input buffer
									break;
								}
							}
						} 
					} // end of checking for the current packet
					// increment the probing header
					probe_head_ptr = probe_head_ptr < (input_buffer_list[i].size - 1) ? probe_head_ptr + 1 : 0;
				} // end of probing the entire input buffer
			}
		} // end of checking the current input buffer
	} // end of the entire probing process


	////////////////////////////////////////////////
    // call all the produce fuctions for all the sub modules
	////////////////////////////////////////////////

	// Readout the next flit in the input buffer when the VC.in_avail is true
    for(int i = 0; i < PORT_NUM; ++i){
        input_buffer_list[i].produce();					
    }

	// Perform RC on the flit in RC.in_latch
	// There's a one cycle delay between the input_buffer.out and RC.out
	for (int i = 0; i < PORT_NUM; ++i) {
		// The prefetch enable is cleared inside input buffer
		// assign the prefetch enable status to RC_list every cycle, thus when input buffer getting out of prefetching state, RC_list will also getting out
		// in the currently implementation, there's nothing need to be done on the RC side when return to normal mode
		RC_list[i].input_prefetch_enable = input_buffer_list[i].pre_fetch_enable;

		// RC produce
		RC_list[i].produce();
	}

	//Takes in the RC.out when the current packet is grant a VC plus the selected VC buffer has free space
	for (int i = 0; i < PORT_NUM; ++i) {
		VCs_list[i].produce();
	}

    if(xbar.produce() == -1)
		return -1;
	app_core.produce();

    //increase the credit_period_counter
    
    if(credit_period_counter < CREDIT_BACK_PERIOD)
        credit_period_counter++;
    else
        credit_period_counter = 0;
    

    //decide all of the out_avail(s)
	for (int i = 0; i < PORT_NUM; ++i) {
		if (credit_period_counter != CREDIT_BACK_PERIOD - 1) {
			if (downstream_credits[i] >= CREDIT_THRESHOlD) {
				if (occupy_by_inject[i]) {
					out_avail_for_passthru[i] = false;
				}
				else if (xbar.out[i].valid || occupy_by_passthru[i]) {	
					out_avail_for_passthru[i] = true;
				}
				else {
					out_avail_for_passthru[i] = true;
				}
				//				if (!occupy_by_inject[i]){
				//					out_avail_for_passthru[i] = true;
				//					out_avail_for_inject[i] = false;
				//				}
			}
			else {
				out_avail_for_passthru[i] = false;
			}
		}
		else {
			//time to send back credit_period_counter
			out_avail_for_passthru[i] = false;
		}
	}

    for(int i = 0; i < PORT_NUM; ++i){
        if(credit_period_counter != CREDIT_BACK_PERIOD - 1){
			if (downstream_credits[i] >= CREDIT_THRESHOlD){
				if (occupy_by_inject[i]){
                    out_avail_for_inject[i] = true;
//					out_avail_for_passthru[i] = false;
                }
				else if (xbar.out[i].valid || occupy_by_passthru[i]){
					out_avail_for_inject[i] = false;
//					out_avail_for_passthru[i] = true;
				}
				else{
					out_avail_for_inject[i] = true;
//					out_avail_for_passthru[i] = true;
				}
//				if (!occupy_by_inject[i]){
//					out_avail_for_passthru[i] = true;
//					out_avail_for_inject[i] = false;
//				}
            }
            else{
//                out_avail_for_passthru[i] = false;
                out_avail_for_inject[i] = false;
            }
            
        }
        else{
            //time to send back credit_period_counter
 //           out_avail_for_passthru[i] = false;
            out_avail_for_inject[i] = false;
        }
    }

    //update the out on each port
    for(int i = 0; i < PORT_NUM; ++i){
		// the credit = input_buffer_size - #_of_words_in_buffer
        upstream_credits[i] = IN_Q_SIZE - 1 - input_buffer_list[i].usedw;

		// send out credit flit to upsteam node every CREDIT_BACK_PERIOD
		if (credit_period_counter == CREDIT_BACK_PERIOD - 1) {
			out[i].valid = true;
			out[i].flit_type = CREDIT_FLIT;
			// assign input buffer's credit
			out[i].payload = upstream_credits[i];
			// assign each VC buffer's credit
			for (int vc_counter = 0; vc_counter < VC_NUM; ++vc_counter) {
				out[i].vc_credit[vc_counter] = VC_SIZE - 1 - VCs_list[i].VC_array[vc_counter].usedw;
			}
        }
		// downstream credit is taken during consume process
		else if (downstream_credits[i] < CREDIT_THRESHOlD) {
			out[i].valid = false;
		}
		// if not occupited by local injection, then take the crossbar output on this port
		else if (xbar.out[i].valid && (!(occupy_by_inject[i]))){
			out[i] = xbar.out[i];
		}
		else if ((!(xbar.out[i].valid)) && occupy_by_passthru[i]) {												// ???????????????????????if xbar.out is not valid, still assign it to the output port???????????????????????????
			out[i] = xbar.out[i];
		}
//		else if ((!xbar.out[i].valid) && (!(occupy_by_inject[i])))
//			out[i].valid = false;
		else 
			out[i] = app_core.inject[i];

    }

	//update the eject
	for (int i = 0; i < PORT_NUM; ++i){
		eject[i] = RC_list[i].flit_eject;
	}
	//update occupy_by_passthru
	// the passthrough has higher priority than local injection
	for (int i = 0; i < PORT_NUM; ++i){
		if (xbar.out[i].valid && (!occupy_by_inject[i]) && xbar.out[i].flit_type == HEAD_FLIT)			// occupy the port when the head flit is there to take while not be occupied by local injection
			occupy_by_passthru[i] = true;
		else if (xbar.out[i].valid && xbar.out[i].flit_type == TAIL_FLIT && credit_period_counter != CREDIT_BACK_PERIOD - 1 && downstream_credits[i] >= CREDIT_THRESHOlD)				// release the occupation when the fail flit is send
			occupy_by_passthru[i] = false;
	}
	//update occupy_by_inject
	for (int i = 0; i < PORT_NUM; ++i){
		if (app_core.inject[i].valid && (!occupy_by_passthru[i]) && app_core.inject[i].flit_type == HEAD_FLIT){
			occupy_by_inject[i] = true;
		}
		else if (app_core.inject[i].valid && app_core.inject[i].flit_type == TAIL_FLIT && credit_period_counter != CREDIT_BACK_PERIOD - 1 && downstream_credits[i] >= CREDIT_THRESHOlD){
			occupy_by_inject[i] = false;
		}
	}


/*
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Debugging info: Checking the point when a certain flit has been injected
	// Use appcore.inject[i] as the input flit for checking
	////////////////////////////////////////////////////////////////////////////////////////////////
	int pattern_check_injection_direction = DIR_XPOS;							// Use symbols like DIR_XPOS, DIR_YPOS, etc to specify, as the flit.injection_direction starts from 1. Ex.: if checking pattern[1][x][x][x][x], then set this as DIR_YPOS 
	int pattern_check_z = 0;
	int pattern_check_y = 5;
	int pattern_check_x = 3;
	int pattern_check_packet_id = 0;
	int pattern_check_flit_id = 0;
	int pattern_check_flit_type = HEAD_FLIT;
	for (int i = 0; i < PORT_NUM; ++i) {
		if (app_core.inject[i].valid) {
			report_pattern_injected(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, pattern_check_flit_type, 0, &app_core.inject[i]);
			//report_pattern_injected(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, BODY_FLIT, pattern_check_flit_id, &app_core.inject[i]);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Debugging info: Checking when a certain flit has left which node
	// Put this function after the output port in router.cpp is assigned, thus it will reflect when the flit has left the node
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < PORT_NUM; ++i) {
		if (out[i].valid) {
			report_pattern_sent(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, pattern_check_flit_type, 0, &out[i]);
			//report_pattern_sent(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, pattern_check_flit_type, pattern_check_flit_id, &out[i]);
		}
	}
*/
	return 0;

}

void router::router_free(){
    for(int i = 0; i < PORT_NUM; ++i){
        input_buffer_list[i].fifo_free();
		VCs_list[i].VCs_free();
    }

    xbar.crossbar_switch_free();
}

