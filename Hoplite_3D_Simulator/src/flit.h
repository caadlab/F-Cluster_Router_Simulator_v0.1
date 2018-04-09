#ifndef FLIT_H
#define FLIT_H

class flit{
public:
    bool valid;
    char flit_type;
    bool VC_class;
    char dst_z;
    char dst_y;
    char dst_x;
    char inject_dir;
    char dir_out;
    int priority_dist;
    int priority_age;
    char src_z;
    char src_y;
    char src_x;
    int packet_id;
    int flit_id;
    int payload;
    int packet_size;

	// Ethan add for Hoplite
	// In Hoplite, the flit injection can be interrupted when there's passthrough requests during injection, in that case, the body lift in a packet will be the head for reinjection
	// Usually, RC_need should be true for HEAD_FLIT and SINGLE_FLIT, but when the injection is interrupted, then the body flit should also be asserted by the local_unit before injection
	bool RC_need;


	char O1TURN_id; //used when using O1TURN mode

	flit();
    flit(bool valid, char Type, int Payload, int Flit_id);
	flit(bool Valid, char Type, bool Vc_class, char Dst_z, char Dst_y, char Dst_x, int Priority_dist, int Priority_age, char Src_z, char Src_y, char Src_x, int Packet_id, int Flit_id, int Payload, int Packet_size, bool RC_Need);
        
};


#endif
