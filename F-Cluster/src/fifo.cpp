#include "fifo.h"
#include <stdlib.h>
#include <stdio.h>
void fifo::fifo_init(int Size, flit* In, bool* Out_avail){
    size = Size;
    if(!(flit_array = (flit*)malloc(size * sizeof(flit)))){
        printf("no mem space for %d flits pointed by flit_array\n",size);
        exit(-1);
    }
	for (int i = 0; i < size; ++i){
		flit_array[i].valid = false;
	}
	in_latch.valid = false;
    head_ptr = 0;
    tail_ptr = 0;
    usedw = 0;
    in_avail = true;
    out.valid = false;
    in = In;
    out_avail = Out_avail;

	// prefetch related
	fifo_freeze_counter = 0;
	pre_fetch_enable = false;
	prev_pre_fetch_enable = false;
	peek_head_ptr = 0;
	prev_out.valid = false;
}

void fifo::consume(){
    in_latch = *in;
    out_avail_latch = *out_avail;
}

void fifo::produce(){
	// Working in Normal Mode
	if (!pre_fetch_enable) {
		int prev_head_ptr = head_ptr;
		// FIFO write
		if (in_avail && in_latch.valid) {
			flit_array[tail_ptr] = in_latch;
			tail_ptr = tail_ptr < (size - 1) ? tail_ptr + 1 : 0;
		}
		// FIFO read
		if (out_avail_latch && out.valid) {
			flit_array[head_ptr].valid = false;
			head_ptr = head_ptr < (size - 1) ? head_ptr + 1 : 0;
			// reset the fifo_freeze_counter if a flit is read out successfully
			fifo_freeze_counter = 0;
			// record for the previous successfully read out flit, use this to recover from prefetching
			// only record this under normal mode
			prev_out = out;
		}
        // perform read
		// out holds a copy of the top FIFO element
		// assign out the next FIFO top if the previous out value is taken by the downstream unit
		out = flit_array[head_ptr];

		// FIFO update usedw
		if (tail_ptr >= head_ptr) {
			usedw = tail_ptr - head_ptr;
		}
		else {
			usedw = tail_ptr - head_ptr + size;
		}

		// FIFO assign in_avail as long as the FIFO is not full
		in_avail = usedw < (size - 1);

		// increment the fifo_freeze_counter only when there's data inside fifo and the head_ptr remains the same
		if (usedw > 0 && head_ptr == prev_head_ptr) {
			fifo_freeze_counter++;
		}
		else if (usedw == 0) {
			fifo_freeze_counter = 0;
		}
	}

	// Working in Pre Fetching Mode
	else {
		// Setup the recovery point when first cycle enters the pre-fetching mode 
		if (pre_fetch_enable && !prev_pre_fetch_enable) {
			// Put the previously readout flit back to the head of the FIFO
			if (usedw < (size - 1)) {
				// move the head pointer one flit back
				head_ptr = (head_ptr > 0) ? head_ptr - 1 : 0;
				flit_array[head_ptr] = prev_out;
				flit_array[head_ptr].valid = true;
			}
			// when the FIFO is full, you cannot put it the stucked head flit back, then this failed
			// this shouldn't happen as long as the credit threshold is larger than CREDIT_BACK_PERIOD + LINKDELAY
			else{ 
				printf("Setting up the recovery point for prefetching failed! Refer to the router.cpp (pre-fetching section) and fifo.cpp file to find out the reason.\n");
				exit(-1);
			}
		}


		// FIFO write
		if (in_avail && in_latch.valid) {
			flit_array[tail_ptr] = in_latch;
			tail_ptr = tail_ptr < (size - 1) ? tail_ptr + 1 : 0;
		}
		// FIFO read
		// always read from peek_head_ptr
		// after the successful read, move all the flit behind this one slot ahead
		if (out_avail_latch && out.valid) {
			flit_array[peek_head_ptr].valid = false;
			// FIFO SHIFTING LOGIC
			// shift each flit behind peek_head_ptr one slot ahead
			int shift_dst_ptr = peek_head_ptr;
			int shift_src_ptr = peek_head_ptr < (size - 1) ? peek_head_ptr + 1 : 0;
			while (shift_src_ptr != tail_ptr) {
				flit_array[shift_dst_ptr] = flit_array[shift_src_ptr];
				// increase shift pointer
				shift_dst_ptr = shift_dst_ptr < (size - 1) ? shift_dst_ptr + 1 : 0;
				shift_src_ptr = shift_dst_ptr < (size - 1) ? shift_dst_ptr + 1 : 0;
			}
			// tail_ptr reduce 1 after shifting
			tail_ptr = (tail_ptr == 0) ? (size - 1) : (tail_ptr - 1);
			// Set the lateset element as invalid as it has been shifted
			flit_array[tail_ptr].valid = false;

			// CHECK IF PRE FETCHING IS DONE
			if (out.flit_type == TAIL_FLIT || out.flit_type == SINGLE_FLIT) {
				pre_fetch_enable = false;
			}
		}

		// FIFO READ
		if (pre_fetch_enable) {
			out = flit_array[peek_head_ptr];
		}
		// when the pre-fetching is done, then resume fetching from the top of the FIFO
		else {
			out = flit_array[head_ptr];
		}

		// FIFO update usedw
		if (tail_ptr >= head_ptr) {
			usedw = tail_ptr - head_ptr;
		}
		else {
			usedw = tail_ptr - head_ptr + size;
		}

		// FIFO assign in_avail as long as the FIFO is not full
		in_avail = usedw < (size - 1);

		// under pre fetching mode, always set freeze counter as 0, thus to avoid having prefetching on top of prefetching
		fifo_freeze_counter = 0;
	}

	// record the previous prefetch enable status
	prev_pre_fetch_enable = pre_fetch_enable;
}

// scanning through the entire fifo to see if there's a packet header that can use the last available VC when deadlock occurs
// only apply this function on input buffer
void fifo::fifo_peek() {


}

void fifo::fifo_free(){
	free(flit_array);
}
