#include "define.h"
#include "local_unit.h"
#include "pattern.h"
#include<stdio.h>
#include<stdlib.h>

// Debugging use only
// Global variable for checking when does each flit has been injected
int injection_check_inject_direction = DIR_XPOS;
int injection_check_z = 0;
int injection_check_y = 0;
int injection_check_x = 0;
int injection_check_packet_id = 1;
int injection_check_flit_starting = 0;
int injection_check_flit_ending = 9;

void local_unit::local_unit_init(int Cur_x, int Cur_y, int Cur_z, int Mode, int Injection_gap, int Packet_size, int Packet_num, flit** Eject, bool** Inject_avail){
    cycle_counter = 0;

    cur_x = Cur_x;
    cur_y = Cur_y;
    cur_z = Cur_z;

    mode = Mode;
    injection_gap = Injection_gap * Packet_size;
    packet_size = Packet_size;
    packet_num = Packet_num;
    for(int i = 0; i < PORT_NUM; ++i){
        eject[i] = Eject[i];
        inject_avail[i] = Inject_avail[i];
        eject_pckt_counter[i] = 0;
        eject_flit_counter[i] = 0;
        inject_pckt_counter[i] = 0;
        eject_flit_counter[i] = 0;
        eject_state[i] = EJECT_IDLE; 
		inject_control_counter[i] = 0;
		inject_flit_counter[i] = 0;
		inject[i].valid = false;

		prev_injection_cycle[i] = 0;				// initialize the previous injection cycle for each one of the output port
    }
	credit_period_counter = 0;
    all_pckt_rcvd = false;
}

/*
// Debugging use
// Used for locating the pattern's location cycle by cycle
int local_unit::report_pattern_location(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, flit* received_flit) {
	flit cur_flit = *received_flit;
	if (cur_flit.inject_dir == injection_direction && cur_flit.src_z == check_z && cur_flit.src_y == check_y && cur_flit.src_x == check_x && cur_flit.packet_id == check_packet_id && cur_flit.flit_type == check_flit_type) {
		printf("Cycle: %d, Pattern[%d][%d][%d][%d][%d], the %d flit is at (%d, %d, %d)", cycle_counter, injection_direction, check_z, check_y, check_x, check_packet_id, check_flit_type, cur_x, cur_y, cur_z);
	}
	return 0;
}
*/

int local_unit::consume(){
	for (int i = 0; i < PORT_NUM; ++i){
		eject_latch[i] = *(eject[i]);
	}
/*
	for (int i = 0; i < PORT_NUM; ++i){
		inject_avail_latch[i] = *(inject_avail[i]);
	}
*/
        

//check all the rcvd data


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ATTENTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// Due to the fact that in deflection routing, the injection may not be consequtive, the flits in a single packet may take different routes, and the tail flit may arrive early than the body flit (but this is very less likely)
	// in the current implementation, when the tail flit is received, then this packet is recorded as received
	// In the future, need to check if all the flits has received before register this packet as received
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for(int i = 0; i < PORT_NUM; ++i){
        if(eject_latch[i].valid){
            if(eject_latch[i].flit_type == HEAD_FLIT){
/*
                if(eject_state[i] != EJECT_IDLE){
                    printf("ejecting from dir: %d, error in local unit (%d,%d,%d): unexpected head flit from (%d,%d,%d), whose dst is (%d,%d,%d), flit id is %d, packet id is %d, whose age is %d\n",i, cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id, eject_latch[i].priority_age);
					return -1;
                } 
*/
				if(eject_latch[i].dst_x != cur_x || eject_latch[i].dst_y != cur_y || eject_latch[i].dst_z != cur_z){
                    printf("error in local unit (%d,%d,%d): head flit arrives wrong dst: from (%d,%d,%d), whose dst is (%d,%d,%d), flit id is %d, packet id is %d\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);
					return -1;
                }
                if(pattern[eject_latch[i].inject_dir - 1][eject_latch[i].src_z][eject_latch[i].src_y][eject_latch[i].src_x][eject_latch[i].packet_id].sent == false){
					printf("ejecting from dir: %d, error in local unit (%d,%d,%d): head flit has not been sent yet: from (%d,%d,%d), whose dst is (%d,%d,%d), id is %d, packet id is %d, whose age is %d\n", i, cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id, eject_latch[i].priority_age);
					return -1;
                }
/*
				if (eject_latch[i].flit_id != eject_flit_counter[i]){
					printf("error in local unit (%d,%d,%d): head flit id is not right: from (%d,%d,%d), whose dst is (%d,%d,%d), id is %d, packet id is %d, expected id is %d\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id, eject_flit_counter[i]);
					return -1;
				}
*/
				eject_flit_counter[i]++;
                eject_state[i] = EJECT_RECVING; 
                cur_eject_src_x[i] = eject_latch[i].src_x;
                cur_eject_src_y[i] = eject_latch[i].src_y;
                cur_eject_src_z[i] = eject_latch[i].src_z;
            }
            else if(eject_latch[i].flit_type == BODY_FLIT){
/*
                if(eject_state[i] != EJECT_RECVING){
                    printf("ejecting from dir: %d, error in local unit (%d,%d,%d): unexpected body flit from (%d,%d,%d), whose dst is (%d,%d,%d), flit id is %d, packet id is %d\n", i, cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);
					return -1;
                }
*/
				if(eject_latch[i].dst_x != cur_x || eject_latch[i].dst_y != cur_y || eject_latch[i].dst_z != cur_z){
                    printf("ejecting from dir: %d, error in local unit (%d,%d,%d): body flit arrives wrong dst: from (%d,%d,%d), whose dst is (%d,%d,%d), flit id is %d, packet id is %d, whose age is %d\n", i, cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id, eject_latch[i].priority_age);
					return -1;
                }
				/*
                if(eject_latch[i].src_x != cur_eject_src_x[i] || eject_latch[i].src_y != cur_eject_src_y[i] || eject_latch[i].src_z != cur_eject_src_z[i]){
                    printf("error in local unit (%d,%d,%d): body mismatch with cur head flit: from (%d,%d,%d), whose dst is (%d,%d,%d), flit id is %d, packet id is %d\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);
					return -1;
                }
				*/
                if(pattern[eject_latch[i].inject_dir - 1][eject_latch[i].src_z][eject_latch[i].src_y][eject_latch[i].src_x][eject_latch[i].packet_id].sent == false){
                    printf("error in local unit (%d,%d,%d): body flit has not been sent yet: from (%d,%d,%d), whose dst is (%d,%d,%d), id is %d, packet id is %d\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);
					return -1;
                }
/*
				if (eject_latch[i].flit_id != eject_flit_counter[i]){
					printf("ejecting from dir: %d, error in local unit (%d,%d,%d): body flit id is not right: from (%d,%d,%d), whose dst is (%d,%d,%d), id is %d, packet id is %d, expected id is %d, whose age is %d, inject_dir is %d\n\n", i, cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id, eject_flit_counter[i], eject_latch[i].priority_age, eject_latch[i].inject_dir);
					return -1;
				}
*/
				eject_flit_counter[i]++;
            }
            else if(eject_latch[i].flit_type == TAIL_FLIT){
/*
				if(eject_state[i] != EJECT_RECVING){
                    printf("error in local unit (%d,%d,%d): unexpected tail flit from (%d,%d,%d), whose dst is (%d,%d,%d), flit id is %d, packet id is %d\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);
					return -1;
                }
*/
				if(eject_latch[i].dst_x != cur_x || eject_latch[i].dst_y != cur_y || eject_latch[i].dst_z != cur_z){
                    printf("error in local unit (%d,%d,%d): tail flit arrives wrong dst: from (%d,%d,%d), whose dst is (%d,%d,%d), flit id is %d, packet id is %d\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);
					return -1;
                }
/*
                if(eject_latch[i].src_x != cur_eject_src_x[i] || eject_latch[i].src_y != cur_eject_src_y[i] || eject_latch[i].src_z != cur_eject_src_z[i]){
                    printf("error in local unit (%d,%d,%d): tail mismatch with cur head flit: from (%d,%d,%d), whose dst is (%d,%d,%d), flit id is %d, packet id is %d\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);
					return -1;
                }
*/
				if(pattern[eject_latch[i].inject_dir - 1][eject_latch[i].src_z][eject_latch[i].src_y][eject_latch[i].src_x][eject_latch[i].packet_id].sent == false){
                    printf("error in local unit (%d,%d,%d): tail flit has not been sent yet: from (%d,%d,%d), whose dst is (%d,%d,%d), id is %d, packet id is %d\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);
					return -1;
                }
/*
                if(eject_flit_counter[i] + 1 != packet_size){
                    printf("error in local unit (%d,%d,%d): eject from %d, tail flit arrived but this packet is not complete: from (%d,%d,%d), whose dst is (%d,%d,%d), id is %d, packet id is %d, arrived flits: %d, expected flits: %d, inject dir is %d\n", i, cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id, eject_flit_counter[i] + 1, packet_size, eject_latch[i].inject_dir);
					return -1;
                }
				if (eject_latch[i].flit_id != eject_flit_counter[i]){
					printf("error in local unit (%d,%d,%d): tail flit id is not right: from (%d,%d,%d), whose dst is (%d,%d,%d), id is %d, packet id is %d, expected id is %d\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id, eject_flit_counter[i]);
					return -1;
				}
*/
                eject_flit_counter[i] = 0;
                eject_pckt_counter[i]++;
                
                eject_state[i] = EJECT_IDLE;
                pattern[eject_latch[i].inject_dir - 1][eject_latch[i].src_z][eject_latch[i].src_y][eject_latch[i].src_x][eject_latch[i].packet_id].rcvd = true;
                pattern[eject_latch[i].inject_dir - 1][eject_latch[i].src_z][eject_latch[i].src_y][eject_latch[i].src_x][eject_latch[i].packet_id].recv_time_stamp = cycle_counter;
            }
            else if(eject_latch[i].flit_type == SINGLE_FLIT){
/*
                if(eject_state[i] != EJECT_IDLE){
                    printf("error in local unit (%d,%d,%d): unexpected single flit from (%d,%d,%d), whose dst is (%d,%d,%d), flit id is %d, packet id is %d\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);
					return -1;
                }
*/
				if(eject_latch[i].dst_x != cur_x || eject_latch[i].dst_y != cur_y || eject_latch[i].dst_z != cur_z){
                    printf("error in local unit (%d,%d,%d): single flit arrives wrong dst: from (%d,%d,%d), whose dst is (%d,%d,%d), flit id is %d, packet id is %d\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);
					return -1;
                }
                if(pattern[eject_latch[i].inject_dir - 1][eject_latch[i].src_z][eject_latch[i].src_y][eject_latch[i].src_x][eject_latch[i].packet_id].sent == false){
                    printf("error in local unit (%d,%d,%d): single flit has not been sent yet: from (%d,%d,%d), whose dst is (%d,%d,%d), id is %d, packet id is %d\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);
					return -1;
                }
                if(packet_size != 1){
                    printf("error in local unit (%d,%d,%d): unexpected single flit, this pattern has not single flit, from (%d,%d,%d), whose dst is (%d,%d,%d), flit id is %d, packet id is %d\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);
                }
				if (eject_latch[i].flit_id != 0){
					printf("error in local unit (%d,%d,%d): single flit id is not right: from (%d,%d,%d), whose dst is (%d,%d,%d), id is %d, packet id is %d, expected id is 0\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);
					return -1;
				}
                eject_pckt_counter[i]++;
                pattern[eject_latch[i].inject_dir - 1][eject_latch[i].src_z][eject_latch[i].src_y][eject_latch[i].src_x][eject_latch[i].packet_id].rcvd = true;
                pattern[eject_latch[i].inject_dir - 1][eject_latch[i].src_z][eject_latch[i].src_y][eject_latch[i].src_x][eject_latch[i].packet_id].recv_time_stamp = cycle_counter;

            }
            else{
                printf("error in local unit (%d,%d,%d): wrong flit type: from (%d,%d,%d), whose dst is (%d,%d,%d), id is %d, packet id is %d,\n", cur_x, cur_y, cur_z, eject_latch[i].src_x, eject_latch[i].src_y, eject_latch[i].src_z, eject_latch[i].dst_x, eject_latch[i].dst_y, eject_latch[i].dst_z, eject_latch[i].flit_id, eject_latch[i].packet_id);    
            }
        }
    }

	return 0;

}

void local_unit::produce(){
	/*
	for (int i = 0; i < PORT_NUM; ++i){
		inject_avail_post_latch[i] = *(inject_avail[i]);
	}

	if (credit_period_counter < CREDIT_BACK_PERIOD)
		credit_period_counter++;
	else
		credit_period_counter = 0;
	*/

	for (int i = 0; i < PORT_NUM; ++i) {
		inject_avail_latch[i] = *(inject_avail[i]);
	}

    for(int i = 0; i < PORT_NUM; ++i){

		//if (!inject_avail_latch[i] && inject_avail_post_latch[i] && (credit_period_counter != CREDIT_BACK_PERIOD)){
		if (inject_avail_latch[i]) {
			if (inject_control_counter[i] == packet_size && inject_avail_latch[i]) {										// increment the injection counter every time injection of a packet finished
				inject_pckt_counter[i]++;
			}
			// !!!!!!!!!!!!!!!! ATTENTION !!!!!!!!!!!!!!!!!!!!!!!!
			// increment the injection control counter only if current injection port is available
			// In that case, the actual waiting time is longer than the injection_gap cycle
			// may need to change this to increment without considering the availability of the injection port
			inject_control_counter[i] = (inject_control_counter[i] <= injection_gap + packet_size) ? inject_control_counter[i] + 1 : 1;
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ATTENTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// Need reconsider the injection condition, need to check if the port is available for injection
		// But once current packet has finished injection, the port maybe occupied by passthrough, however the injection counter inside should still be working 
		// so I can't simply put a if condition on top this chunk of code, need to isolate the counter from the injection logic
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (inject_avail_latch[i]) {
			if (inject_pckt_counter[i] < global_injection_packet_size[i][cur_z][cur_y][cur_x]) {
				// only perform injection when the injection_control_counter is less than the packet size
				if (inject_control_counter[i] <= packet_size) {																		// first inject, then wait for injection_gap time
					// if currently is injecting the first flit of the packet, then set the status in global pattern array as send and record the sending time
					if (inject_control_counter[i] == 1) {
						pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].sent = true;
						// check if this packet have been send before
						if (pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].rcvd) {
							printf("this packet is already rcvd, error!: error in local unit (%d,%d,%d)\n", cur_x, cur_y, cur_z);
							exit(-1);
						}
						pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].rcvd = false;
						pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].send_time_stamp = cycle_counter;

						// debugging info
						// The checking variable is defined globally
						//if (i == injection_check_inject_direction && cur_z == injection_check_z && cur_y == injection_check_y && cur_x == injection_check_x && inject_pckt_counter[i] == injection_check_packet_id) {
						//	printf("@@@@@@@@@@@@@@ At cycle %d, Pattern[%d][%d][%d][%d][%d], the first flit is injected!!!!!!\n", cycle_counter, i, cur_z, cur_y, cur_x, inject_pckt_counter[i]);
						//}

					}

					// if this is a SINGLE_FLIT
					if (packet_size == 1) {

						inject[i].flit_type = SINGLE_FLIT;
						inject[i].RC_need = true;
						inject[i].flit_id = 0;
						inject[i].packet_id = inject_pckt_counter[i];
						inject[i].dst_z = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].dst_z;
						inject[i].dst_y = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].dst_y;
						inject[i].dst_x = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].dst_x;
						inject[i].priority_dist = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].mahattan_dist + XSIZE + YSIZE + ZSIZE;
						//inject[i].priority_age = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].send_time_stamp;
						inject[i].payload = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].payload;
						inject[i].O1TURN_id = -1;
						//           inject_pckt_counter[i]++;
					}

					// When this is a multiple flit packet
					else {
						//	pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].sent = true;
						//	pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].rcvd = false;

						if (inject_control_counter[i] == 1) {
							inject[i].flit_type = HEAD_FLIT;
							inject[i].RC_need = true;
							inject[i].flit_id = 0;
							inject[i].packet_id = inject_pckt_counter[i];
							inject[i].dst_z = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].dst_z;
							inject[i].dst_y = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].dst_y;
							inject[i].dst_x = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].dst_x;
							inject[i].priority_dist = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].mahattan_dist + XSIZE + YSIZE + ZSIZE;
							//inject[i].priority_age = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].send_time_stamp;
							inject[i].payload = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].payload;
							//		pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].send_time_stamp = cycle_counter;
						}
						else if (inject_control_counter[i] > 1 && inject_control_counter[i] < packet_size) {
							inject[i].flit_type = BODY_FLIT;
							// Implementation 2: make sure each injected flit is independent and need RC
							inject[i].RC_need = true;
//							// Implementation 1: if the injection has been interrupted, then this BODY_FLIT should be treat as HEAD_FLIT
//							if ((prev_injection_cycle[i] + 1) != cycle_counter) {
//								inject[i].RC_need = true;
//							}
//							else {
//								inject[i].RC_need = false;
//							}
							inject[i].flit_id = inject_control_counter[i] - 1;
							inject[i].packet_id = inject_pckt_counter[i];
							inject[i].dst_z = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].dst_z;
							inject[i].dst_y = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].dst_y;
							inject[i].dst_x = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].dst_x;
							inject[i].priority_dist = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].mahattan_dist + XSIZE + YSIZE + ZSIZE;
							//inject[i].priority_age = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].send_time_stamp;
							inject[i].payload = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].payload;
						}
						else if (inject_control_counter[i] == packet_size) {
							inject[i].flit_type = TAIL_FLIT;
							// Implementation 2: make sure each injected flit is independent and need RC
							inject[i].RC_need = true;
//							// Implementation 1: if the injection has been interrupted, then this TAIL_FLIT should be treat as HEAD_FLIT
//							if ((prev_injection_cycle[i] + 1) != cycle_counter) {
//								inject[i].RC_need = true;
//							}
//							else {
//								inject[i].RC_need = false;
//							}
							inject[i].flit_id = inject_control_counter[i] - 1;
							inject[i].packet_id = inject_pckt_counter[i];
							inject[i].dst_z = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].dst_z;
							inject[i].dst_y = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].dst_y;
							inject[i].dst_x = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].dst_x;
							inject[i].priority_dist = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].mahattan_dist + XSIZE + YSIZE + ZSIZE;
							//inject[i].priority_age = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].send_time_stamp;
							inject[i].payload = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].payload;
							//				inject_pckt_counter[i]++;
						}
					}

					//////////////////////////////////////////////////////////////
					// Debugging info: when did each flit has been injected
					/////////////////////////////////////////////////////////////
					//int injection_check_inject_direction = DIR_XPOS;
					//int injection_check_z = 0;
					//int injection_check_y = 0;
					//int injection_check_x = 1;
					//int injection_check_packet_id = 0;
					//int injection_check_flit_id = 6;
					// The checking flit info is defined globally for easy changing
					//if (i == injection_check_inject_direction - 1 && cur_z == injection_check_z && cur_y == injection_check_y && cur_x == injection_check_x && inject_pckt_counter[i] == injection_check_packet_id && inject[i].flit_id >= injection_check_flit_starting && inject[i].flit_id <= injection_check_flit_ending) {
					//	printf("@@@@@@@@@@@@@@ At cycle %d, Pattern[%d][%d][%d][%d][%d], the #%d flit just injected!!!!!!\n", cycle_counter, i, cur_z, cur_y, cur_x, inject_pckt_counter[i], inject[i].flit_id);
					//}
					////////////////////////////////////////////////////////////

					inject[i].valid = true;
					inject[i].VC_class = (i >= 3);

					inject[i].inject_dir = i + 1;
					inject[i].dir_out = i + 1;

					// set the priority age, the smaller this variable is, the higher the priority is
					// currently, use the initial transfer time as the priority age, even though this packet's transfer is interrupted, still use the very first flit's injection time as priority_age, this will give the following flits the same priority as the head_flit
					// If this cause conjestion, then try to use the real sending time for each flit as priority_age
					inject[i].priority_age = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].send_time_stamp;

					inject[i].src_x = cur_x;
					inject[i].src_y = cur_y;
					inject[i].src_z = cur_z;
					inject[i].O1TURN_id = -1;
					//inject[i].packet_size = pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].packet_size;

					// recording the current injection time, this will be used to compare with the current cycle time to determine if the injection has been interruputed
					prev_injection_cycle[i] = cycle_counter;
				}
				else {
					inject[i].valid = false;
				}
				//if ((inject_avail_post_latch[i] || inject_avail_latch[i]) && (credit_period_counter != (CREDIT_BACK_PERIOD-2))){
//				if ((inject_avail_latch[i])) {
//					if (inject[i].valid  && (inject[i].flit_type == SINGLE_FLIT || inject[i].flit_type == HEAD_FLIT)){
//						pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].sent = true;
//						pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].rcvd = false;
//						pattern[i][cur_z][cur_y][cur_x][inject_pckt_counter[i]].send_time_stamp = cycle_counter;
//					}
//					if (inject[i].valid && (inject[i].flit_type == SINGLE_FLIT || inject[i].flit_type == TAIL_FLIT))
//						inject_pckt_counter[i]++;
//					inject_control_counter[i] = (inject_control_counter[i] <= injection_gap + packet_size - 1) ? inject_control_counter[i] + 1 : 0;
//				}

			}
			else
				inject[i].valid = false;
		}
		else
			inject[i].valid = false;
//		// The local unit will still perform the injection operation even when the inject_available latch is false
//		// Here set the valid as false if the port is not available for injection
//		if (!inject_avail_latch[i]) {
//			inject[i].valid = false;
//		}
    }



	//////////////////////////////////////////////////////////////////
	// Increse cycle counter every time the produce function is called
	//////////////////////////////////////////////////////////////////
    cycle_counter++;
}
