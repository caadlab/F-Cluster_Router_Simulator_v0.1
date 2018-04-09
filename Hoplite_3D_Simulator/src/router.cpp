#include "router.h"
#include <stdio.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The working steps inside router:
// *Read in all the input into the in_latch (Only thing performed in Consume stage) 
// *Free unused output port: 
//				Check if the input flit is invalid, then the occupied output port by that input should be freed, if any
//				Check if the current received flit belongs to the same packet as the previous one received on this packet, if not, then this means that packet inejction has been interruputed and this port should be freed from passthrough occupation
// *Check which input flit need to perform routing computation and which port it can potentially be sent out
// *Pause all the injection ports temporaliy and provide those ports for passthrough, this will gives priority to passthrough over injection (if not been used for passthrough, then the port will be used again for injection)
// *Swithc Allocation: Determine which output to go
// *Restore those injection port that have not been used for passthrough
// *Deflection routing: If a certain input cannot go to the desired output port, then send it to any port that is available
// *Available for injection: check which port is now available for injection and send the information to app_core
// *Ejection assignment: assign the input flit that arrives the destination to the ejection_latch
// *App_core consume: takes in the ejected flit and perform checking
// *App_core produce: generate the injection flits if the port is available
// *Update output mapping result: if the app_core does performs injection on a port, mark that port as occupied by injection
// *Assigning the output port
// *Output mapping update: if injected flit is TAIL_FLIT or SINGLE_FLIT, then the injection port should be freed
// *Output mapping update: if the passthrough flit is TAIL_FLIT or SINGLE_FLIT, then the passthrough port should be freed
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ONE BIG PROBLEM:
// While the node in injecting, there're input coming from all the input ports, at this time, the injection need to stop!!!!!!!!!!!
// need to implement this!!!!!!!!!!!!!
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Debugging use only
// Global variables for reporting certain patterns location cycle by cycle
int pattern_check_injection_direction = DIR_XPOS;
int pattern_check_z = 0;
int pattern_check_y = 0;
int pattern_check_x = 0;
int pattern_check_packet_id = 3408;
int pattern_check_flit_id = 0;					// only need to assign this when it's the BODY_FLIT
int pattern_check_flit_type = SINGLE_FLIT;

void router::router_init(int Cur_x, int Cur_y, int Cur_z, int SA_Mode, int Routing_mode, int Injection_mode, flit** In, flit** Inject, int Injection_gap, int Packet_size){
    cur_x = Cur_x;
    cur_y = Cur_y;
    cur_z = Cur_z;

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
    }

	for (int i = 0; i < PORT_NUM; ++i) {
		eject_ptrs[i] = &(eject[i]);
		inject_avail_ptrs[i] = &out_avail_for_inject[i];
	}

	app_core.local_unit_init(cur_x, cur_y, cur_z, injection_mode, Injection_gap, Packet_size, PACKET_NUM, eject_ptrs, inject_avail_ptrs);

    //allocate space for those subentities need dynamic allocation

	for (int i = 0; i < PORT_NUM; ++i) {
		bool valid_constant = true;
		input_buffer_list[i].fifo_init(IN_Q_SIZE, &in_latch[i], &valid_constant);			// output always valid for input_buffer in deflective routing
	}

/*
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
        VCs_list[i].VCs_init(i + 1, flit_list_to_VA[i], &xbar);
    }

    for(int i = 0; i < PORT_NUM * VC_NUM; ++i){
        flit_list_to_SA[i] = &(VCs_list[i / VC_NUM].out[i % VC_NUM]);
    }
    for(int i = 0; i < PORT_NUM; ++i){
        in_avail_from_ST[i] = &(out_avail_for_passthru[i]); //initially this is for injetion   
    }
	xbar.crossbar_switch_init(SA_mode, flit_list_to_SA, in_avail_from_ST);
*/

    //init all avails
    for(int i = 0; i < PORT_NUM; ++i){
        occupy_by_inject[i] = false;
		occupy_by_passthru[i] = false;
        downstream_avail[i] = true;
        out_avail_for_passthru[i] = true;
        out_avail_for_inject[i] = true;
    }

	//// initialize the output_port_available array
	//for (int i = 0; i < PORT_NUM; ++i) {
	//	for (int j = 0; j < PORT_NUM; ++j) {
	//		potential_output_port[i][j] = false;
	//	}
	//}

	// initialize the ejection status array
	for (int i = 0; i < PORT_NUM; ++i) {
		ejection_status[i] = false;
	}

	//// initialize the priority_age array for injection packets
	//for (int i = 0; i < PORT_NUM; ++i) {
	//	cur_injection_priority_age[i] = 0;
	//}

	// IMPORTANT!!!! This array stores the SA results!!!!!!!!!!!!!!!!!
	// initialize the input&output mapping status array
	// 0: OUT_FROM_XNEG;
	// 1: OUT_FROM_YNEG;
	// 2: OUT_FROM_ZNEG;
	// 3: OUT_FROM_INJECTION;
	// 4: OUT_IDLE;
	for (int i = 0; i < PORT_NUM; ++i) {
		output_port_mapping[i] = OUT_IDLE;
	}

	//// initalize the array that used to recording newly incoming packets that need to perform routing
	//for (int i = 0; i < PORT_NUM; ++i) {
	//	Passthrough_RC_needed[i] = false;
	//}

	//// recording the previous received flit, using this information to determine if the newly received flit belongs to the same packet as the previous one, if not, this means the injection for that flit has been interrupted and this port should be freed
	//for (int i = 0; i < PORT_NUM; ++i) {
	//	prev_passthrough_flit[i].valid = false;
	//}
}

// Debugging use
// Used for locating the pattern's location cycle by cycle
// Put this function after the RC and SA is done, so it will print out which direction the packet will be send to
int router::report_pattern_location(int injection_direction, int check_z, int check_y, int check_x, int check_packet_id, int check_flit_type, int check_flit_id, flit* received_flit, int incoming_dir) {
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
		else{
			find_the_match = false;
		}

		// When a match is found, print out
		if (find_the_match) {
			// locate output direction
			for (int i = 0; i < PORT_NUM; ++i) {
				if (output_port_mapping[i] == incoming_dir) outgoing_dir = i;
			}

			char inject_dir[20] = "";
			switch (injection_direction) {
			case DIR_XPOS: sprintf(inject_dir, "%s", "DIR_XPOS"); break;
			case DIR_YPOS: sprintf(inject_dir, "%s", "DIR_YPOS"); break;
			case DIR_ZPOS: sprintf(inject_dir, "%s", "DIR_ZPOS"); break;
			}

			char incoming[20] = "";
			switch (incoming_dir) {
			case 0: sprintf(incoming, "%s", "DIR_XNEG"); break;
			case 1: sprintf(incoming, "%s", "DIR_YNEG"); break;
			case 2: sprintf(incoming, "%s", "DIR_ZNEG"); break;
			}

			char outgoing[20] = "";
			switch (outgoing_dir) {
			case 0: sprintf(outgoing, "%s", "DIR_XPOS"); break;
			case 1: sprintf(outgoing, "%s", "DIR_YPOS"); break;
			case 2: sprintf(outgoing, "%s", "DIR_ZPOS"); break;
			case DIR_EJECT: sprintf(outgoing, "%s", "DIR_EJECT"); break;
			}

			char flit_type[20] = "";
			switch (check_flit_type) {
			case 0: sprintf(flit_type, "%s", "HEAD_FLIT"); break;
			case 1: sprintf(flit_type, "%s", "BODY_FLIT"); break;
			case 2: sprintf(flit_type, "%s", "TAIL_FLIT"); break;
			case 3: sprintf(flit_type, "%s", "SINGLE_FLIT"); break;
			}

			// print out information
			printf("*****Cycle: %d, Pattern[%s][%d][%d][%d][%d], src(%d,%d,%d), dst(%d,%d,%d), the #%d (%s) flit is at Node (%d, %d, %d), incoming direction is %s, outgoing direction is %s\n",   
				app_core.cycle_counter, inject_dir, check_z, check_y, check_x, check_packet_id, cur_flit.src_x, cur_flit.src_y, cur_flit.src_z, cur_flit.dst_x, cur_flit.dst_y, cur_flit.dst_z, flit_id, flit_type, cur_x, cur_y, cur_z, incoming, outgoing);
		}
	}
	return 0;
}

int router::consume(){
    //latch in and inject
	for (int i = 0; i < PORT_NUM; ++i) {
		in_latch[i] = *(in[i]);
	}

	return 0;
}

int router::produce() {
	//call all the produce fuctions for all the sub modules
	for (int i = 0; i < PORT_NUM; ++i) {
		input_buffer_list[i].produce();
	}
/*
	// initialize the output_port_available array every time RC is performed 
	for (int i = 0; i < PORT_NUM; ++i) {
		for (int j = 0; j < PORT_NUM; ++j) {
			potential_output_port[i][j] = false;
		}
	}
	// initalize the array that used to recording newly incoming packets that need to perform routing
	for (int i = 0; i < PORT_NUM; ++i) {
		Passthrough_RC_needed[i] = false;
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Free unused output port
	// First check if the input flit is invalid, then the output port occupied by that input should also be freed, if any
	// Then check if the new incoming flit belongs to the same packet as the previous one, if not, then this means the inection has been interrupted and the previous passthrough is done, free port
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// When the input flit is not valid, then the output occupied by that input port should be free, if there's any
	for (int i = 0; i < PORT_NUM; ++i) {
		if (!in_latch[i].valid) {
			for (int j = 0; j < PORT_NUM; ++j) {
				if (output_port_mapping[j] == i) {
					output_port_mapping[j] = OUT_IDLE;
				}
			}
		}
	}
*/

	// clear the output mapping info
	for (int i = 0; i < PORT_NUM; ++i) {
		output_port_mapping[i] = OUT_IDLE;
	}

	////////////////////////////////////////////////
	// RC and VA logic here
	////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ROUTING LOGIC:
	// Schematic: 
	//																									 / XPOS
	//		ZNEG -> ZPOS				YNEG - YPOS							XNEG - YPOS
	//													 \ ZPOS									 \ ZPOS
	//
	// Priority order: ZNEG > YNEG >XNEG, following DOR routing:
	// 
	// Implementation:
	//		First check if the input is for ejection
	//		if ZNEG input is valid, then it will take ZPOS
	//		if YNEG input is valid, then
	//			if ydst == ycur, then 
	//				if ZPOS taken, then deflect on YPOS
	//				else take ZPOS
	//			else
	//				take YPOS
	//		if XNEG input is valid, then
	//			if xdst == xcur, then
	//				if YPOS is taken, then deflect on XPOS
	//				else take YPOS
	//			else
	//				take XPOS
	//		Check the available ports left, if there is, then inject
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Routing Computation, determine the output port for each incoming flit from ZNEG, YNEG, XNEG
	// Check if the incoming flit reach the destination
	for (int i = 0; i < PORT_NUM; ++i) {
		if (in_latch[i].valid && cur_x == in_latch[i].dst_x && cur_y == in_latch[i].dst_y && cur_z == in_latch[i].dst_z) {
			ejection_status[i] = true;
		}
		else
			ejection_status[i] = false;
	}
	// Check ZNEG first, when flit on Z direction, it means that the X, Y coordinates are all match
	if (in_latch[IN_PORT_ZNEG].valid && !ejection_status[IN_PORT_ZNEG]) {			// Only perform RC when the input is valid
		flit flit_in = in_latch[IN_PORT_ZNEG];
		if (flit_in.dst_z != cur_z) {		// if z coordinate not matching, then assign ZNEG input to ZPOS output
			output_port_mapping[OUT_PORT_ZPOS] = OUT_FROM_ZNEG;
		}
		else {
			output_port_mapping[OUT_PORT_ZPOS] = OUT_IDLE;
		}
	}
	// Check YNEG
	if (in_latch[IN_PORT_YNEG].valid && !ejection_status[IN_PORT_YNEG]) {			// Only perform RC when the input is valid
		flit flit_in = in_latch[IN_PORT_YNEG];
		if (flit_in.dst_y != cur_y) {		// if y coordinate not matching, then assign YNEG input to YPOS output
			output_port_mapping[OUT_PORT_YPOS] = OUT_FROM_YNEG;
		}
		else if (output_port_mapping[OUT_PORT_ZPOS] == OUT_IDLE) {		// if Y coordinates matches, then it's time to send to Z direction if it's not occupied
			output_port_mapping[OUT_PORT_ZPOS] = OUT_FROM_YNEG;
			output_port_mapping[OUT_PORT_YPOS] = OUT_IDLE;
		}
		else {		// if Y coordinate matches, but ZPOS is occupied, then deflect the flit back to YPOS
			output_port_mapping[OUT_PORT_YPOS] = OUT_FROM_YNEG;
		}
	}
	// Check XNEG
	if (in_latch[IN_PORT_XNEG].valid && !ejection_status[IN_PORT_XNEG]) {			// Only perform RC when the input is valid
		flit flit_in = in_latch[IN_PORT_XNEG];
		if (flit_in.dst_x != cur_x) {		// if x coordinate not matching, then assign XNEG input to XPOS output
			output_port_mapping[OUT_PORT_XPOS] = OUT_FROM_XNEG;
		}
		else if (flit_in.dst_y != cur_y) {		// if x coordinate match, but y coordinate not match
			if (output_port_mapping[OUT_PORT_YPOS] == OUT_IDLE) {		// if X coordinates matches, then it's time to send to Y direction if it's not occupied
				output_port_mapping[OUT_PORT_YPOS] = OUT_FROM_XNEG;
				output_port_mapping[OUT_PORT_XPOS] = OUT_IDLE;
			}
			else {		// if X coordinate matches, but YPOS is occupied, then deflect the flit back to XPOS
				output_port_mapping[OUT_PORT_XPOS] = OUT_FROM_XNEG;
			}
		}
		else {		// if both X and Y coordinates matches, then try to send to ZPOS directly
			if (output_port_mapping[OUT_PORT_ZPOS] == OUT_IDLE) {		// If ZPOS available, then send to ZPOS
				output_port_mapping[OUT_PORT_ZPOS] = OUT_FROM_XNEG;
				output_port_mapping[OUT_PORT_XPOS] = OUT_IDLE;
			}
			else if (output_port_mapping[OUT_PORT_YPOS] == OUT_IDLE) {		// if ZPOS not available, but YPOS is available, then send to YPOS
				output_port_mapping[OUT_PORT_YPOS] = OUT_FROM_XNEG;
				output_port_mapping[OUT_PORT_XPOS] = OUT_IDLE;
			}
			else {		// If neither ZPOS nor YPOS is available, then deflect on XPOS
				output_port_mapping[OUT_PORT_XPOS] = OUT_FROM_XNEG;
			}
		}
	}
//
//	// Routing computation, determine which incoming flit need to perform Routing Computation and which port is should go
//	int new_incoming_packet_counter = 0;
//	for (int i = 0; i < PORT_NUM; ++i) {
//		//if (in_latch[i].valid && (in_latch[i].flit_type == HEAD_FLIT || in_latch[i].flit_type == SINGLE_FLIT)) {		// perform RC only when this is HEAD_FLIT of SINGLE_FLIT
//		// Perform RC if the flit.RC_need is true
//		// cases: HEAD_FLIT, SINGLE_FLIT, interrupted BODY_FLIT and TAIL_FLIT
//		if (in_latch[i].valid && in_latch[i].RC_need) {
//			// first determine if this is ejection
//			if (cur_x == in_latch[i].dst_x && cur_y == in_latch[i].dst_y && cur_z == in_latch[i].dst_z) {
//				// eject
//				ejection_status[i] = true;
//			}
//			else {
//				// if this is not the destination, then the ejection_status is false
//				ejection_status[i] = false;
//
//				// recording packet from current input port need to perform routing
//				Passthrough_RC_needed[i] = true;
//
//				// recording the list of potential optimial port current packet can go
//				if (cur_x != in_latch[i].dst_x)	potential_output_port[i][DIR_XPOS - 1] = true;
//				if (cur_y != in_latch[i].dst_y)	potential_output_port[i][DIR_YPOS - 1] = true;
//				if (cur_z != in_latch[i].dst_z)	potential_output_port[i][DIR_ZPOS - 1] = true;
//
//				// increment the newly incoming packet counter, used for debugging
//				new_incoming_packet_counter++;
//			}
//		}
//	}
//
//
///*
//	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	// Implementation 1: Check if some injection port should pause for passthrough, if so, set the selected injection port as IDLE so the passthrough packet can use it
//	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	// check how many idle ports are available for passthrough
//	int idle_outport_for_passthrough = 0;
//	for (int i = 0; i < PORT_NUM; ++i) {
//		if (output_port_mapping[i] == OUT_IDLE) {
//			idle_outport_for_passthrough++;
//		}
//	}
//	// When the free output port is not enough for the new input packet to passthrough, then have to pause some injection
//	// choose the injecting packets that have the lowest priority and interruput it
//	if (new_incoming_packet_counter > idle_outport_for_passthrough)
//	{
//		// calculate how many injection ports need to be interrupted
//		int num_injection_need_to_interrupt = new_incoming_packet_counter - idle_outport_for_passthrough;
//		for (int i = 0; i < num_injection_need_to_interrupt; ++i) {
//			int select_injection_port = 0;
//			int select_priority_age = 0;
//			// find the injecting packets that has the largest priority_age, which means this one is the newest one, and should be interrupted
//			for (int j = 0; j < PORT_NUM; ++j) {
//				if (output_port_mapping[j] == OUT_FROM_INJECTION && cur_injection_priority_age[j] > select_priority_age) {
//					select_injection_port = j;
//					select_priority_age = cur_injection_priority_age[j];
//				}
//			}
//			// after the injection port to interrruput is selected, set that port as IDLE so the passthrough packet can use it
//			output_port_mapping[select_injection_port] = OUT_IDLE;
//		}
//	}
//*/	
//
//	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	// Implementation 2: Always give priority to passthrough, let passthrough packets determine if they want to take the port or not, if so, interruput the injections regardless of the priority_age
//	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	bool output_port_prev_for_injection[PORT_NUM];		// use this one to record the previous injection port, after the SA stage, rstore those injection port that have not been occupied
//	for (int injection_port = 0; injection_port < PORT_NUM; ++injection_port) {
//		if (output_port_mapping[injection_port] == OUT_FROM_INJECTION) {
//			output_port_prev_for_injection[injection_port] = true;
//			output_port_mapping[injection_port] = OUT_IDLE;
//		}
//		else {
//			output_port_prev_for_injection[injection_port] = false;
//		}
//	}
//
//	// set all the outport as idle before perform routing computation
//	for (int i = 0; i < PORT_NUM; ++i) {
//		output_port_mapping[i] = OUT_IDLE;
//	}
//
//
//	//////////////////////////////////////////////////////////////////
//	// SA for passthrough
//	//////////////////////////////////////////////////////////////////
//	// for each output port, if available for passthrough, check all the input port that requesting this port and assign it to the one with highest priority
//	for (int output_port = 0; output_port < PORT_NUM; ++output_port) {
//		// when the output port is idle, then start to select a output for it
//		if (output_port_mapping[output_port] == OUT_IDLE) {
//			int priority_age = 32767;
//			int assigned_input_port = 0;
//			bool assigned_input_port_valid = false;
//			// check all the input ports that requesting this port, find the one with the minimum priority_age and assign it
//			for (int input_port = 0; input_port < PORT_NUM; ++input_port) {
//				if (in_latch[input_port].valid) {			// Only check those valid input
//					if (potential_output_port[input_port][output_port] && (in_latch[input_port].priority_age < priority_age)) {	// when this output port is a potential one for the current input port, and it has a smaller priority_age
//						priority_age = in_latch[input_port].priority_age;
//						assigned_input_port = input_port;
//						assigned_input_port_valid = true;
//					}
//				}
//			}
//
//			// at this point, the suitable input port is already selected for the current output port
//			if (assigned_input_port_valid) {
//				// since this input is already taken one output, need to clear the potential_output_port array so it won't request for another port
//				// also clear the output available status for passthrough and injection
//				for (int clear_input = 0; clear_input < PORT_NUM; ++clear_input) {
//					potential_output_port[assigned_input_port][clear_input] = false;
//				}
//				// now let's assign the selected input to the output
//				output_port_mapping[output_port] = assigned_input_port;
//				// clear the routing request for the routed packets
//				Passthrough_RC_needed[assigned_input_port] = false;
//			}
//		}
//	}
//
///*
//	// When the injection and passthrough currently taking all the output ports, still we need to interrupt some injection, so let's find out how many passthrough packets have not been assign yet
//	int unroute_counter = 0;
//	for (int unrouted = 0; unrouted < PORT_NUM; ++unrouted) {
//		if (Passthrough_RC_needed[unrouted]) {
//			unroute_counter++;
//		}
//	}
//
//	// Check how many ports are idle
//	int idle_counter = 0;
//	for (int i = 0; i < PORT_NUM; ++i) {
//		if (output_port_mapping[i] == OUT_IDLE) {
//			idle_counter++;
//		}
//	}
//*/
//	//////////////////////////////////////////////////////////////////
//	// Deflection routing
//	// Check if there's any unrouted packets, if so, then perform the deflection routing: assgin this packet to any port that is available
//	//////////////////////////////////////////////////////////////////
//	for (int unrouted = 0; unrouted < PORT_NUM; ++unrouted) {
//		if (Passthrough_RC_needed[unrouted]) {
//			for (int check_idle_output = 0; check_idle_output < PORT_NUM; ++check_idle_output) {
//				if (output_port_mapping[check_idle_output] == OUT_IDLE) {
//					output_port_mapping[check_idle_output] = unrouted;
//					// also clear the output request status
//					for (int clear_input = 0; clear_input < PORT_NUM; ++clear_input) {
//						potential_output_port[unrouted][clear_input] = false;
//					}
//					break;
//				}
//			}
//		}
//	}
//
//	//////////////////////////////////////////////////////////////////////////////////////////////////////////
//	// restore those injection ports that have not been used for passthrough, then apply the deflection routing
//	//////////////////////////////////////////////////////////////////////////////////////////////////////////
//	for (int injection_port = 0; injection_port < PORT_NUM; ++injection_port) {
//		if (output_port_prev_for_injection[injection_port] && output_port_mapping[injection_port] == OUT_IDLE) {
//			output_port_mapping[injection_port] = OUT_FROM_INJECTION;
//		}
//	}


	//////////////////////////////////////////////////////////////////
	// Check port that is available for injection
	//////////////////////////////////////////////////////////////////
	// Update available prots for injection
	for (int i = 0; i < PORT_NUM; ++i) {
		if (output_port_mapping[i] == OUT_IDLE) {
			out_avail_for_inject[i] = true;
		}
		else {
			out_avail_for_inject[i] = false;
		}
	}

	//////////////////////////////////////////////////////////////////
	// Ejection port assignment
	//////////////////////////////////////////////////////////////////
	for (int i = 0; i < PORT_NUM; ++i) {
		if (ejection_status[i]) {
			eject[i] = in_latch[i];
		}
		else
			eject[i].valid = false;
	}
	//////////////////////////////////////////////////////////////////
	// Locale unit consume (ejection)
	//////////////////////////////////////////////////////////////////
	if (app_core.consume() == -1)
		return -1;

	//////////////////////////////////////////////////////////////////
	// Locale unit produce (injection)
	//////////////////////////////////////////////////////////////////
	app_core.produce();
/*
    //increase the credit_period_counter   
    if(credit_period_counter < CREDIT_BACK_PERIOD)
        credit_period_counter++;
    else
        credit_period_counter = 0;
*/    


/*
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
*/
	//////////////////////////////////////////////////////////////////
	// Output port mapping update: post injection
	//////////////////////////////////////////////////////////////////
	for (int i = 0; i < PORT_NUM; ++i) {
		if (app_core.inject[i].valid) {
			output_port_mapping[i] = OUT_FROM_INJECTION;
		}
	}

	//////////////////////////////////////////////////////////////////
	// Update the output port for passthrough and injection
	// The index for prev_passthrough_flit is mapped based on the output port
	//////////////////////////////////////////////////////////////////
	for (int i = 0; i < PORT_NUM; ++i) {
		int input_source = output_port_mapping[i];
		// When this is passthrough
		if (input_source == OUT_FROM_XNEG || input_source == OUT_FROM_YNEG || input_source == OUT_FROM_ZNEG) {
			// assign the output port
			out[i] = in_latch[input_source];
		}
		// When this is injection
		else if (input_source == OUT_FROM_INJECTION) {
			out[i] = app_core.inject[i];
		}
		// if the port is not assigned, then the output is set as invalid
		else {
			out[i].valid = false;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Debugging printout for tracing the path of a certain pattern
	// Only check this after the RC and SA is done so it will give the routing information
	////////////////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < PORT_NUM; ++i) {
		if (in_latch[i].valid) {
			// these variables will be defined globally for easy changes
			//int pattern_check_injection_direction = DIR_XPOS;
			//int pattern_check_z = 0;
			//int pattern_check_y = 0;
			//int pattern_check_x = 1;
			//int pattern_check_packet_id = 0;
			//int pattern_check_flit_id = 2;
			//int pattern_check_flit_type = BODY_FLIT;
			
			// function definition
			//report_pattern_location(check_injection_direction, check_z, check_y, check_x, check_packet_id, check_flit_id, check_flit_type, &in_latch[i], i);

			if (pattern_check_flit_type == SINGLE_FLIT) {
				report_pattern_location(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, SINGLE_FLIT, 0, &in_latch[i], i);
			}
			else {
				report_pattern_location(pattern_check_injection_direction, pattern_check_z, pattern_check_y, pattern_check_x, pattern_check_packet_id, pattern_check_flit_type, pattern_check_flit_id, &in_latch[i], i);
//				for (int id = 1; id < packet_size - 1; ++id) {
//					report_pattern_location(check_injection_direction, check_z, check_y, check_x, check_packet_id, id, BODY_FLIT, &in_latch[i], i);
//				}
//				report_pattern_location(check_injection_direction, check_z, check_y, check_x, check_packet_id, packet_size - 1, TAIL_FLIT, &in_latch[i], i);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//// update the injection packets' priority age
	//// This is used for determining which injection port should be give up to passthrough
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//for (int i = 0; i < PORT_NUM; ++i) {
	//	int input_source = output_port_mapping[i];
	//	// if this port is used for injection, then record the priorit_age
	//	if (input_source == OUT_FROM_INJECTION) {
	//		cur_injection_priority_age[i] = app_core.inject[i].priority_age;
	//	}
	//	// if the port is not used as injection, then record 0
	//	else {
	//		cur_injection_priority_age[i] = 0;
	//	}
	//}

	////////////////////////////////////////////////////////////////////
	//// Output port mapping update: clear the occupation if the injection is finished
	//// This one should be performed after the output port is assigned
	////////////////////////////////////////////////////////////////////
	//for (int i = 0; i < PORT_NUM; ++i) {
	//	if (app_core.inject[i].valid &&(app_core.inject[i].flit_type == TAIL_FLIT || app_core.inject[i].flit_type == SINGLE_FLIT)) {
	//		output_port_mapping[i] = OUT_IDLE;
	//	}
	//}

	////////////////////////////////////////////////////////////////////
	//// Free output occupation:
	//// !!!!! Need rework here: if the packet injection is interrupted, then need to free certain output port even when it's only a body_flit
	//// Output port mapping update: clear the occupation after passthrough is done
	////////////////////////////////////////////////////////////////////
	//for (int i = 0; i < PORT_NUM; ++i) {
	//	int input_source = output_port_mapping[i];
	//	// when this is passthrough
	//	if (input_source == OUT_FROM_XNEG || input_source == OUT_FROM_YNEG || input_source == OUT_FROM_ZNEG) {
	//		flit current = in_latch[input_source];
	//		// If the current packet finished passthrough, then set this output port as IDLE 
	//		if (current.flit_type == TAIL_FLIT || current.flit_type == SINGLE_FLIT) {
	//			output_port_mapping[i] = OUT_IDLE;
	//		}
	//	}
	//}

/*
    //update the out on each port
    for(int i = 0; i < PORT_NUM; ++i){
		// the credit = input_buffer_size - #_of_words_in_buffer
        upstream_credits[i] = IN_Q_SIZE - 1 - input_buffer_list[i].usedw;

		// send out credit flit to upsteam node every CREDIT_BACK_PERIOD
        if(credit_period_counter == CREDIT_BACK_PERIOD - 1){
            out[i].valid = true;
            out[i].flit_type = CREDIT_FLIT;
            out[i].payload = upstream_credits[i];
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
*/
/*
	//update the eject
	for (int i = 0; i < PORT_NUM; ++i){
		eject[i] = RC_list[i].flit_eject;
	}
*/
/*
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
*/

	return 0;

}

void router::router_free(){
    for(int i = 0; i < PORT_NUM; ++i){
        input_buffer_list[i].fifo_free();
//		VCs_list[i].VCs_free();
    }

//    xbar.crossbar_switch_free();
}

