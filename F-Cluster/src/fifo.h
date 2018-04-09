#ifndef FIFO_H
#define FIFO_H
#include "flit.h"
class fifo{
	
public:
	flit* in;
	flit in_latch;
	bool out_avail_latch;
	bool in_avail;
	bool* out_avail;
	flit out;
	flit* flit_array;
	int size;
	int head_ptr;
	int tail_ptr;
	int usedw;

	// the control bit that enables fifo prefetch
	// asserted in router.cpp when the FIFO peek found the desired packet header
	// deasserted inside fifo after the prefetching of a packet is done
	bool pre_fetch_enable;
	bool prev_pre_fetch_enable;
	// the pointer for out of order fetching
	// this is assign in router.cpp when the FIFO peek found the desired packet header
	int peek_head_ptr;
	// Used to record the previous output data
	// Due the control mechanism, the packet header is read out from the FIFO when the blocking occurs, the head of the FIFO is the 2nd flit of a packet
	// Thus implement this to record the previous send out flit, when prefetch happens, bring this back and put it to the head of the input buffer
	flit prev_out;


	// use this counter to record how many cycles has passed since the buffer has been freezed
	// this one is only used for input_buffer
	int fifo_freeze_counter;

	void fifo_init(int Size, flit* In, bool* Out_avail);
	void consume();
	void produce();
	void fifo_free();
	void fifo_peek();


};



#endif
