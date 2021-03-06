#include "network.h"
#include "pattern.h"
#include "define.h"
#include "packet.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string>
#include <fstream>

int global_injection_packet_size[PORT_NUM][ZSIZE][YSIZE][XSIZE];
packet* pattern[PORT_NUM][ZSIZE][YSIZE][XSIZE];
bool staturation_status[15][PORT_NUM][ZSIZE][YSIZE][XSIZE];
int injection_blocked_counter[15][PORT_NUM][ZSIZE][YSIZE][XSIZE];


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pattern Generation
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int pos_or_neg(int src, int dst, int x_or_y_or_z){
	int size;
	int ret = 2; //1 is pos, -1 is neg, 0 is both fine, 2 is error
	if (x_or_y_or_z == 0)//x dim
		size = XSIZE;
	else if (x_or_y_or_z == 1)
		size = YSIZE;
	else if (x_or_y_or_z == 2)
		size = ZSIZE;
	else
		return 2;

	if (src >= size || src < 0 || dst >= size || dst < 0)
		return 2;

	if (src > dst){
		if (size % 2 == 0){
			if (src - dst < size / 2)
				ret = -1;
			else if (src - dst > size / 2)
				ret = 1;
			else
				ret = 0;
		}
		else{
			if (src - dst <= size / 2)
				ret = -1;
			else
				ret = 1;
		}
	}
	else if (src < dst){
		if (size % 2 == 0){
			if (dst - src < size / 2)
				ret = 1;
			else if (dst - src > size / 2)
				ret = -1;
			else
				ret = 0;
		}
		else{
			if (dst - src <= size / 2)
				ret = 1;
			else
				ret = -1;
		}
	}
	else
		ret = 2;
	return ret;
}

char comp_inject_dir(int src_x, int src_y, int src_z, int dst_x, int dst_y, int dst_z){
//compute which direction to inject, the injection direction must not violate the ristricted turning rules. 
//the six turning is forbidden
//zneg->xpos
//zneg->xneg
//zneg->ypos
//zneg->yneg
//yneg->xpos
//yneg->xneg
	char ret;
	int x_dir;
	int y_dir;
	int z_dir;
	if (src_x == dst_x && src_y == dst_y && src_z == dst_z){
		ret = DIR_EJECT;
	}
	else if (src_x == dst_x && src_y == dst_y){
		z_dir = pos_or_neg(src_z, dst_z, 2);
		if (z_dir == -1)
			ret = DIR_ZNEG;
		else if (z_dir == 1)
			ret = DIR_ZPOS;
		else{
			int i = 0;// rand() % 2;
			if (i == 0)
				ret = DIR_ZPOS;
			else
				ret = DIR_ZNEG;
		}

	}
	else if (src_x == dst_x){
		y_dir = pos_or_neg(src_y, dst_y, 1);
		if (y_dir == -1)
			ret = DIR_YNEG;
		else if (y_dir == 1)
			ret = DIR_YPOS;
		else{
			int i = 1;// rand() % 2;
			if (i == 0)
				ret = DIR_YPOS;
			else
				ret = DIR_YNEG;
		}
	}
	else{//in this case, all the DIR_XPOS, DIR_YPOS, DIR_ZPOS, DIR_XNEG are allowed to use, DIR_YNEG and DIR_ZNEG are not allowed
		x_dir = pos_or_neg(src_x, dst_x, 0);
		y_dir = pos_or_neg(src_y, dst_y, 1);
		z_dir = pos_or_neg(src_z, dst_z, 2);
		if (z_dir == -1 || z_dir == 2){ //zpos is banned also
			if (y_dir == -1 || y_dir == 2){//ypos is banned also
				if (x_dir == -1)
					ret = DIR_XNEG;
				else if (x_dir == 1)
					ret = DIR_XPOS;
				else{
					int i = 0;// rand() % 2;
					if (i == 0)
						ret = DIR_XPOS;
					else
						ret = DIR_XNEG;
				}
			}
			else if (y_dir == 1 || y_dir == 0){//ypos is allowed
				if (x_dir == -1){
					int i = 1;// rand() % 2;
					if (i == 0)
						ret = DIR_YPOS;
					else
						ret = DIR_XNEG;
				}
				else if (x_dir == 1){
					int i = 0;// rand() % 2;
					if (i == 0)
						ret = DIR_YPOS;
					else
						ret = DIR_XPOS;
				}
				else{
					int i = 1;// rand() % 3;
					if (i == 0)
						ret = DIR_YPOS;
					else if (i == 1)
						ret = DIR_XPOS;
					else
						ret = DIR_XNEG;
				}
			}

		}
	
		else if (z_dir == 1 || z_dir == 0){//zpos is allowed
			if (y_dir == -1 || y_dir == 2){//ypos is banned also
				if (x_dir == -1){
					int i = 0;// rand() % 2;
					if (i == 0)
						ret = DIR_ZPOS;
					else
						ret = DIR_XNEG;
				}
				else if (x_dir == 1){
					int i = 1;// rand() % 2;
					if (i == 0)
						ret = DIR_ZPOS;
					else
						ret = DIR_XPOS;
				}
				else{
					int i = 0;// rand() % 3;
					if (i == 0)
						ret = DIR_ZPOS;
					else if (i == 1)
						ret = DIR_XPOS;
					else
						ret = DIR_XNEG;
				}
			}
			else if (y_dir == 1 || y_dir == 0){//ypos is allowed
				if (x_dir == -1){
					int i = 1;// rand() % 3;
					if (i == 0)
						ret = DIR_ZPOS;
					else if (i == 1)
						ret = DIR_YPOS;
					else
						ret = DIR_XNEG;
				}
				else if (x_dir == 1){
					int i = 2;// rand() % 3;
					if (i == 0)
						ret = DIR_ZPOS;
					else if (i == 1)
						ret = DIR_YPOS;
					else
						ret = DIR_XPOS;
				}
				else{
					int i = 3;// rand() % 4;
					if (i == 0)
						ret = DIR_YPOS;
					else if (i == 1)
						ret = DIR_XPOS;
					else if (i == 2)
						ret = DIR_XNEG;
					else
						ret = DIR_ZPOS;
				}

			}
		}

	}

	return ret;
}

char comp_inject_dir_tornado(int src_x, int src_y, int src_z, int dst_x, int dst_y, int dst_z){
	//compute which direction to inject, the injection direction must not violate the ristricted turning rules. 
	//the six turning is forbidden
	//zneg->xpos
	//zneg->xneg
	//zneg->ypos
	//zneg->yneg
	//yneg->xpos
	//yneg->xneg
	char ret;
	int x_dir;
	int y_dir;
	int z_dir;
	if (src_z == dst_z && src_x != dst_x){
		int pos_x_dist = 0;
		int neg_x_dist = 0;
		if (dst_x > src_x){
			pos_x_dist = dst_x - src_x;
			neg_x_dist = XSIZE - pos_x_dist;
		}
		else{
			neg_x_dist = src_x - dst_x;
			pos_x_dist = XSIZE - neg_x_dist;
		}
		
		if (pos_x_dist >= neg_x_dist)
			return DIR_XNEG;
		else
			return DIR_XPOS;
		
	}


	if (src_x == dst_x && src_y == dst_y && src_z == dst_z){
		ret = DIR_EJECT;
	}
	else if (src_x == dst_x && src_y == dst_y){
		z_dir = pos_or_neg(src_z, dst_z, 2);
		if (z_dir == -1)
			ret = DIR_ZNEG;
		else if (z_dir == 1)
			ret = DIR_ZPOS;
		else{
			int i = 0;// rand() % 2;
			if (i == 0)
				ret = DIR_ZPOS;
			else
				ret = DIR_ZNEG;
		}

	}
	else if (src_x == dst_x){
		y_dir = pos_or_neg(src_y, dst_y, 1);
		if (y_dir == -1)
			ret = DIR_YNEG;
		else if (y_dir == 1)
			ret = DIR_YPOS;
		else{
			int i = 1;// rand() % 2;
			if (i == 0)
				ret = DIR_YPOS;
			else
				ret = DIR_YNEG;
		}
	}
	else{//in this case, all the DIR_XPOS, DIR_YPOS, DIR_ZPOS, DIR_XNEG are allowed to use, DIR_YNEG and DIR_ZNEG are not allowed
		x_dir = pos_or_neg(src_x, dst_x, 0);
		y_dir = pos_or_neg(src_y, dst_y, 1);
		z_dir = pos_or_neg(src_z, dst_z, 2);
		if (z_dir == -1 || z_dir == 2){ //zpos is banned also
			if (y_dir == -1 || y_dir == 2){//ypos is banned also
				if (x_dir == -1)
					ret = DIR_XNEG;
				else if (x_dir == 1)
					ret = DIR_XPOS;
				else{
					int i = 0;// rand() % 2;
					if (i == 0)
						ret = DIR_XPOS;
					else
						ret = DIR_XNEG;
				}
			}
			else if (y_dir == 1 || y_dir == 0){//ypos is allowed
				if (x_dir == -1){
					int i = 1;// rand() % 2;
					if (i == 0)
						ret = DIR_YPOS;
					else
						ret = DIR_XNEG;
				}
				else if (x_dir == 1){
					int i = 0;// rand() % 2;
					if (i == 0)
						ret = DIR_YPOS;
					else
						ret = DIR_XPOS;
				}
				else{
					int i = 1;// rand() % 3;
					if (i == 0)
						ret = DIR_YPOS;
					else if (i == 1)
						ret = DIR_XPOS;
					else
						ret = DIR_XNEG;
				}
			}

		}

		else if (z_dir == 1 || z_dir == 0){//zpos is allowed
			if (y_dir == -1 || y_dir == 2){//ypos is banned also
				if (x_dir == -1){
					int i = 0;// rand() % 2;
					if (i == 0)
						ret = DIR_ZPOS;
					else
						ret = DIR_XNEG;
				}
				else if (x_dir == 1){
					int i = 1;// rand() % 2;
					if (i == 0)
						ret = DIR_ZPOS;
					else
						ret = DIR_XPOS;
				}
				else{
					int i = 0;// rand() % 3;
					if (i == 0)
						ret = DIR_ZPOS;
					else if (i == 1)
						ret = DIR_XPOS;
					else
						ret = DIR_XNEG;
				}
			}
			else if (y_dir == 1 || y_dir == 0){//ypos is allowed
				if (x_dir == -1){
					int i = 1;// rand() % 3;
					if (i == 0)
						ret = DIR_ZPOS;
					else if (i == 1)
						ret = DIR_YPOS;
					else
						ret = DIR_XNEG;
				}
				else if (x_dir == 1){
					int i = 2;// rand() % 3;
					if (i == 0)
						ret = DIR_ZPOS;
					else if (i == 1)
						ret = DIR_YPOS;
					else
						ret = DIR_XPOS;
				}
				else{
					int i = 3;// rand() % 4;
					if (i == 0)
						ret = DIR_YPOS;
					else if (i == 1)
						ret = DIR_XPOS;
					else if (i == 2)
						ret = DIR_XNEG;
					else
						ret = DIR_ZPOS;
				}

			}
		}

	}

	return ret;
}


int total_packet_sent = 0;

// each node send to the closest neighbor (x+1, y, z), (x-1, y, z), (x, y+1, z), (x, y-1, z) ......
int gen_pattern_nearest_neighbor(int pattern_size){
    //pattern size means the number of packets of each pair of nodes in the pattern
    total_packet_sent = 0;
	for (int i = 0; i < PORT_NUM; ++i) {
		for (int j = 0; j < ZSIZE; ++j) {
			for (int k = 0; k < YSIZE; ++k) {
				for (int m = 0; m < XSIZE; ++m) {
					if (!(pattern[i][j][k][m] = (packet*)malloc(pattern_size * sizeof(packet)))) {
						printf("error when allocating space for pattern\n");
						exit(-1);
					}
					global_injection_packet_size[i][j][k][m] = pattern_size;
				}
			}
		}
	}

	for (int i = 0; i < PORT_NUM; ++i) {
		for (int j = 0; j < ZSIZE; ++j) {
			for (int k = 0; k < YSIZE; ++k) {
				for (int m = 0; m < XSIZE; ++m) {
                    for(int n = 0; n < pattern_size; ++n){
                        pattern[i][j][k][m][n].valid = true;
						total_packet_sent++;
                        pattern[i][j][k][m][n].src_x = m;
                        pattern[i][j][k][m][n].src_y = k;
                        pattern[i][j][k][m][n].src_z = j;
                        pattern[i][j][k][m][n].id = n;
                        if(i == 0){
                            pattern[i][j][k][m][n].dst_x = m + 1 >= XSIZE ? 0 : m + 1;
                            pattern[i][j][k][m][n].dst_y = k;
                            pattern[i][j][k][m][n].dst_z = j;
                        }
                        else if(i == 1){
                            pattern[i][j][k][m][n].dst_x = m;
                            pattern[i][j][k][m][n].dst_y = k + 1 >= YSIZE ? 0 : k + 1;
                            pattern[i][j][k][m][n].dst_z = j;
                        }
                        else if(i == 2){
                            pattern[i][j][k][m][n].dst_x = m;
                            pattern[i][j][k][m][n].dst_y = k;
                            pattern[i][j][k][m][n].dst_z = j + 1 >= ZSIZE ? 0 : j + 1;
                        }
                        else if(i == 3){
                            pattern[i][j][k][m][n].dst_x = m != 0 ? m - 1 : XSIZE - 1;
                            pattern[i][j][k][m][n].dst_y = k;
                            pattern[i][j][k][m][n].dst_z = j;
                        }
                        else if(i == 4){
                            pattern[i][j][k][m][n].dst_x = m;
                            pattern[i][j][k][m][n].dst_y = k != 0 ? k - 1 : YSIZE - 1;
                            pattern[i][j][k][m][n].dst_z = j;
                        }
                        else if(i == 5){
                            pattern[i][j][k][m][n].dst_x = m;
                            pattern[i][j][k][m][n].dst_y = k;
                            pattern[i][j][k][m][n].dst_z = j != 0 ? j - 1 : ZSIZE - 1;
                        }
                        pattern[i][j][k][m][n].mahattan_dist = 1;
                        pattern[i][j][k][m][n].sent = false;
                        pattern[i][j][k][m][n].rcvd = false;
                        pattern[i][j][k][m][n].payload = n;
                        
                    }
                }
            }
        }
        
    }
	return 6;
}

// each node send to (x+1, y+1, z+1), (x+1, y-1, z+1), (x-1, y+1, z+1), (x-1,y-1, z+1), (x+1, y+1, z-1), (x+1, y-1, z-1), (x-1, y+1, z-1), (x-1,y-1, z-1)
int gen_pattern_three_hop_diagonal(int pattern_size){
	total_packet_sent = 0;
	for (int i = 0; i < PORT_NUM; ++i){
		for (int j = 0; j < ZSIZE; ++j){
			for (int k = 0; k < YSIZE; ++k){
				for (int m = 0; m < XSIZE; ++m){
					if (!(pattern[i][j][k][m] = (packet*)malloc(2 * pattern_size * sizeof(packet)))){
						printf("error when allocating space for pattern\n");
						exit(-1);
					}
					if (i <= 3)
						global_injection_packet_size[i][j][k][m] = 2 * pattern_size;
					else
						global_injection_packet_size[i][j][k][m] = 0;
					for (int n = 0; n < 2 * pattern_size; ++n){
						
						pattern[i][j][k][m][n].src_x = m;
						pattern[i][j][k][m][n].src_y = k;
						pattern[i][j][k][m][n].src_z = j;
						pattern[i][j][k][m][n].id = n;
						if (i == 0){
							pattern[i][j][k][m][n].valid = true;
							total_packet_sent++;
							if (n % 2 == 0){
								pattern[i][j][k][m][n].dst_x = m + 1 >= XSIZE ? 0 : m + 1;
								pattern[i][j][k][m][n].dst_y = k + 1 >= YSIZE ? 0 : k + 1;
								pattern[i][j][k][m][n].dst_z = j + 1 >= ZSIZE ? 0 : j + 1;
							}
							else{
								pattern[i][j][k][m][n].dst_x = m + 1 >= XSIZE ? 0 : m + 1;
								pattern[i][j][k][m][n].dst_y = k != 0 ? k - 1 : YSIZE - 1;
								pattern[i][j][k][m][n].dst_z = j != 0 ? j - 1 : ZSIZE - 1;
							}
						}
						else if (i == 1){
							pattern[i][j][k][m][n].valid = true;
							total_packet_sent++;
							if (n % 2 == 0){
								pattern[i][j][k][m][n].dst_x = m + 1 >= XSIZE ? 0 : m + 1;
								pattern[i][j][k][m][n].dst_y = k + 1 >= YSIZE ? 0 : k + 1;
								pattern[i][j][k][m][n].dst_z = j != 0 ? j - 1 : ZSIZE - 1;
							}
							else{
								pattern[i][j][k][m][n].dst_x = m != 0 ? m - 1 : XSIZE - 1;
								pattern[i][j][k][m][n].dst_y = k + 1 >= YSIZE ? 0 : k + 1;
								pattern[i][j][k][m][n].dst_z = j != 0 ? j - 1 : ZSIZE - 1;
							}
						}
						else if (i == 2){
							pattern[i][j][k][m][n].valid = true;
							total_packet_sent++;
							if (n % 2 == 0){
								pattern[i][j][k][m][n].dst_x = m != 0 ? m - 1 : XSIZE - 1;
								pattern[i][j][k][m][n].dst_y = k != 0 ? k - 1 : YSIZE - 1;
								pattern[i][j][k][m][n].dst_z = j + 1 >= ZSIZE ? 0 : j + 1;
							}
							else{
								pattern[i][j][k][m][n].dst_x = m + 1 >= XSIZE ? 0 : m + 1;
								pattern[i][j][k][m][n].dst_y = k != 0 ? k - 1 : YSIZE - 1;
								pattern[i][j][k][m][n].dst_z = j + 1 >= ZSIZE ? 0 : j + 1;
							}
						}
						else if (i == 3){
							pattern[i][j][k][m][n].valid = true;
							total_packet_sent++;
							if (n % 2 == 0){
								pattern[i][j][k][m][n].dst_x = m != 0 ? m - 1 : XSIZE - 1;
								pattern[i][j][k][m][n].dst_y = k != 0 ? k - 1 : YSIZE - 1;
								pattern[i][j][k][m][n].dst_z = j != 0 ? j - 1 : ZSIZE - 1;
							}
							else{
								pattern[i][j][k][m][n].dst_x = m != 0 ? m - 1 : XSIZE - 1;
								pattern[i][j][k][m][n].dst_y = k + 1 >= YSIZE ? 0 : k + 1;
								pattern[i][j][k][m][n].dst_z = j + 1 >= ZSIZE ? 0 : j + 1;
							}
						}
						else{
							pattern[i][j][k][m][n].valid = false;
						}
						pattern[i][j][k][m][n].mahattan_dist = 3;
						pattern[i][j][k][m][n].sent = false;
						pattern[i][j][k][m][n].rcvd = false;
						pattern[i][j][k][m][n].payload = n;

					}
				}
			}
		}

	}

	return 4;

}

// Each node send to the 26 neighbor nodes
int gen_pattern_cube_nearest_neighbor(int pattern_size){ //each node multicast to 26 nearest neighbors
	total_packet_sent = 0;
	bool port_used[PORT_NUM];
	for (int ii = 0; ii < PORT_NUM; ++ii){
		port_used[ii] = false;
	}
	char cur_inject_dir;
	// allocate and initialize pattern array
	for (int z = 0; z < ZSIZE; ++z) {
		for (int y = 0; y < YSIZE; ++y) {
			for (int x = 0; x < XSIZE; ++x) {
				for (int i = 0; i < PORT_NUM; ++i) {
					global_injection_packet_size[i][z][y][x] = 0;
					if (!(pattern[i][z][y][x] = (packet*)malloc(26 * pattern_size * sizeof(packet)))) {//worst case, all the packets are injected in a single injection port
						printf("error when allocating space for pattern\n");
						exit(-1);
					}
					for (int j = 0; j < 26 * pattern_size; ++j) {
						pattern[i][z][y][x][j].valid = false;
						pattern[i][z][y][x][j].rcvd = false;
						pattern[i][z][y][x][j].sent = false;
					}
				}
			}
		}
	}
	// assign array elements
	for (int j = 0; j < pattern_size; ++j) {
		for (int z = 0; z < ZSIZE; ++z) {
			for (int y = 0; y < YSIZE; ++y) {
				for (int x = 0; x < XSIZE; ++x) {
					for (int dst_z = -1; dst_z <= 1; ++dst_z){
						for (int dst_y = -1; dst_y <= 1; ++dst_y){
							for (int dst_x = -1; dst_x <= 1; ++dst_x){

								int real_dst_x = (x + dst_x < 0) ? (x + dst_x + XSIZE) : ((x + dst_x >= XSIZE) ? (x + dst_x - XSIZE) : (x + dst_x));
								int real_dst_y = (y + dst_y < 0) ? (y + dst_y + YSIZE) : ((y + dst_y >= YSIZE) ? (y + dst_y - YSIZE) : (y + dst_y));
								int real_dst_z = (z + dst_z < 0) ? (z + dst_z + ZSIZE) : ((z + dst_z >= ZSIZE) ? (z + dst_z - ZSIZE) : (z + dst_z));

								cur_inject_dir = comp_inject_dir(x, y, z, real_dst_x, real_dst_y, real_dst_z);
								
								if (cur_inject_dir != DIR_EJECT){
									port_used[cur_inject_dir - 1] = true;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].inject_dir = cur_inject_dir;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_x = x;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_y = y;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_z = z;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].id = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_x = real_dst_x;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_y = real_dst_y;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_z = real_dst_z;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].mahattan_dist = abs(dst_z) + abs(dst_y) + abs(dst_x);
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].sent = false;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].rcvd = false;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].payload = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].valid = true;
									global_injection_packet_size[cur_inject_dir - 1][z][y][x]++;
									total_packet_sent++;
								}

							}
						}
					}
				}
			}
		}
	}

	int port_used_counter = 0;
	for (int jj = 0; jj < PORT_NUM; ++jj){
		if (port_used[jj])
			port_used_counter++;
	}
	return port_used_counter;

}


// Each node send to its single complement node
int gen_pattern_bitcomplement(int pattern_size) {
	total_packet_sent = 0;
	char cur_inject_dir;
	bool port_used[PORT_NUM];
	for (int ii = 0; ii < PORT_NUM; ++ii){
		port_used[ii] = false;
	}
	// allocate and initialize pattern array
	for (int z = 0; z < ZSIZE; ++z) {
		for (int y = 0; y < YSIZE; ++y) {
			for (int x = 0; x < XSIZE; ++x) {
				for (int i = 0; i < PORT_NUM; ++i) {
					global_injection_packet_size[i][z][y][x] = 0;
					if (!(pattern[i][z][y][x] = (packet*)malloc(pattern_size * sizeof(packet)))) {//worst case, all the packets are injected in a single injection port
						printf("error when allocating space for pattern\n");
						exit(-1);
					}
					for (int j = 0; j < pattern_size; ++j) {
						pattern[i][z][y][x][j].valid = false;
						pattern[i][z][y][x][j].rcvd = false;
						pattern[i][z][y][x][j].sent = false;
					}
				}
			}
		}
	}
	// assign array elements
	for (int j = 0; j < pattern_size; ++j) {
		for (int z = 0; z < ZSIZE; ++z) {
			for (int y = 0; y < YSIZE; ++y) {
				for (int x = 0; x < XSIZE; ++x) {
					int dst_z = ZSIZE - 1 - z;
					int dst_y = YSIZE - 1 - y;
					int dst_x = XSIZE - 1 - x;
					cur_inject_dir = comp_inject_dir(x, y, z, dst_x, dst_y, dst_z);
				
					if (cur_inject_dir != DIR_EJECT){
						port_used[cur_inject_dir - 1] = true;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].inject_dir = cur_inject_dir;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_x = x;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_y = y;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_z = z;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].id = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_x = dst_x;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_y = dst_y;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_z = dst_z;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].mahattan_dist = abs(dst_z) + abs(dst_y) + abs(dst_x);
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].sent = false;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].rcvd = false;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].payload = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].valid = true;
						global_injection_packet_size[cur_inject_dir - 1][z][y][x]++;
						total_packet_sent++;

					}
				}
			}
		}
	}
	int port_used_counter = 0;
	for (int jj = 0; jj < PORT_NUM; ++jj){
		if (port_used[jj])
			port_used_counter++;
	}
	return 1;
}

// Each node send to a single node (x, y, z) -> (y, z, x)
int gen_pattern_transpose(int pattern_size) {
	total_packet_sent = 0;
	char cur_inject_dir;
	bool port_used[PORT_NUM];
	for (int ii = 0; ii < PORT_NUM; ++ii) {
		port_used[ii] = false;
	}

	for (int z = 0; z < ZSIZE; ++z) {
		for (int y = 0; y < YSIZE; ++y) {
			for (int x = 0; x < XSIZE; ++x) {
				for (int i = 0; i < PORT_NUM; ++i) {
					global_injection_packet_size[i][z][y][x] = 0;
					if (!(pattern[i][z][y][x] = (packet*)malloc(pattern_size * sizeof(packet)))) {//worst case, all the packets are injected in a single injection port
						printf("error when allocating space for pattern\n");
						exit(-1);
					}
					for (int j = 0; j < pattern_size; ++j) {
						pattern[i][z][y][x][j].valid = false;
						pattern[i][z][y][x][j].rcvd = false;
						pattern[i][z][y][x][j].sent = false;
					}
				}
				int dst_z = x;
				int dst_y = z;
				int dst_x = y;
				cur_inject_dir = comp_inject_dir(x, y, z, dst_x, dst_y, dst_z);
				for (int j = 0; j < pattern_size; ++j){
					if (cur_inject_dir != DIR_EJECT){
						port_used[cur_inject_dir - 1] = true;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].inject_dir = cur_inject_dir;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_x = x;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_y = y;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_z = z;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].id = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_x = dst_x;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_y = dst_y;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_z = dst_z;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].mahattan_dist = abs(dst_z) + abs(dst_y) + abs(dst_x);
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].sent = false;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].rcvd = false;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].payload = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].valid = true;
						global_injection_packet_size[cur_inject_dir - 1][z][y][x]++;
						total_packet_sent++;

					}
				}
			}
		}
	}
	int port_used_counter = 0;
	for (int jj = 0; jj < PORT_NUM; ++jj) {
		if (port_used[jj])
			port_used_counter++;
	}

	return 1;

}

// Each node send to a single node (Tornado on the Y direction)
int gen_pattern_tornado(int pattern_size) {
	total_packet_sent = 0;
	char cur_inject_dir;

	for (int z = 0; z < ZSIZE; ++z) {
		for (int y = 0; y < YSIZE; ++y) {
			for (int x = 0; x < XSIZE; ++x) {
				for (int i = 0; i < PORT_NUM; ++i) {
					global_injection_packet_size[i][z][y][x] = 0;
					if (!(pattern[i][z][y][x] = (packet*)malloc(pattern_size * sizeof(packet)))) {//worst case, all the packets are injected in a single injection port
						printf("error when allocating space for pattern\n");
						exit(-1);
					}
					for (int j = 0; j < pattern_size; ++j) {
						pattern[i][z][y][x][j].valid = false;
						pattern[i][z][y][x][j].rcvd = false;
						pattern[i][z][y][x][j].sent = false;
					}
				}
				int dst_z = z;
				int dst_y = (y + YSIZE / 2 - 1 >= YSIZE) ? y + YSIZE / 2 - 1 - YSIZE : y + YSIZE / 2 - 1;
				int dst_x = (x + 1 >= XSIZE) ? (x + 1 - XSIZE) : (x);
				cur_inject_dir = comp_inject_dir_tornado(x, y, z, dst_x, dst_y, dst_z);
				for (int j = 0; j < pattern_size; ++j){
					if (cur_inject_dir != DIR_EJECT){
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].inject_dir = cur_inject_dir;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_x = x;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_y = y;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_z = z;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].id = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_x = dst_x;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_y = dst_y;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_z = dst_z;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].mahattan_dist = abs(dst_z) + abs(dst_y) + abs(dst_x);
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].sent = false;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].rcvd = false;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].payload = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].valid = true;
						global_injection_packet_size[cur_inject_dir - 1][z][y][x]++;
						total_packet_sent++;

					}
				}
			}
		}
	}
	int port_used_counter = 0;

	return 1;

}

// Each node send to all the nodes in the network
int gen_pattern_all_to_all(int pattern_size){ //each node multicast to 26 nearest neighbors
	total_packet_sent = 0;
	char cur_inject_dir;
	bool port_used[PORT_NUM];
	for (int ii = 0; ii < PORT_NUM; ++ii){
		port_used[ii] = false;
	}
	// allocate and initialize pattern array
	for (int z = 0; z < ZSIZE; ++z) {
		for (int y = 0; y < YSIZE; ++y) {
			for (int x = 0; x < XSIZE; ++x) {
				for (int i = 0; i < PORT_NUM; ++i) {
					global_injection_packet_size[i][z][y][x] = 0;
					if (!(pattern[i][z][y][x] = (packet*)malloc(XSIZE * YSIZE * ZSIZE * pattern_size * sizeof(packet)))) {//worst case, all the packets are injected in a single injection port
						printf("error when allocating space for pattern\n");
						exit(-1);
					}
					for (int j = 0; j < XSIZE * YSIZE * ZSIZE * pattern_size; ++j) {
						pattern[i][z][y][x][j].valid = false;
						pattern[i][z][y][x][j].rcvd = false;
						pattern[i][z][y][x][j].sent = false;
					}
				}
			}
		}
	}
	// assign array elements
	for (int j = 0; j < pattern_size; ++j) {
		for (int z = 0; z < ZSIZE; ++z) {
			for (int y = 0; y < YSIZE; ++y) {
				for (int x = 0; x < XSIZE; ++x) {
					for (int dst_z = - ZSIZE / 2; dst_z <= ceil(double(ZSIZE / 2.0)) - 1; ++dst_z){
						for (int dst_y = -YSIZE / 2; dst_y <= ceil(double(YSIZE / 2.0)) - 1; ++dst_y){
							for (int dst_x = -XSIZE / 2; dst_x <= ceil(double(XSIZE / 2.0)) - 1; ++dst_x){
								int real_dst_x = (x + dst_x < 0) ? (x + dst_x + XSIZE) : ((x + dst_x >= XSIZE) ? (x + dst_x - XSIZE) : (x + dst_x));
								int real_dst_y = (y + dst_y < 0) ? (y + dst_y + YSIZE) : ((y + dst_y >= YSIZE) ? (y + dst_y - YSIZE) : (y + dst_y));
								int real_dst_z = (z + dst_z < 0) ? (z + dst_z + ZSIZE) : ((z + dst_z >= ZSIZE) ? (z + dst_z - ZSIZE) : (z + dst_z));
								if (real_dst_x < 0 || real_dst_x >= XSIZE){
									printf("error, dst error\n");
									exit(-1);
								}
								if (real_dst_y < 0 || real_dst_y >= YSIZE){
									printf("error, dst error\n");
									exit(-1);
								}
								if (real_dst_z < 0 || real_dst_z >= ZSIZE){
									printf("error, dst error\n");
									exit(-1);
								}

								cur_inject_dir = comp_inject_dir(x, y, z, real_dst_x, real_dst_y, real_dst_z);
								
								if (cur_inject_dir != DIR_EJECT){
									port_used[cur_inject_dir - 1] = true;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].inject_dir = cur_inject_dir;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_x = x;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_y = y;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_z = z;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].id = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_x = real_dst_x;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_y = real_dst_y;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_z = real_dst_z;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].mahattan_dist = abs(dst_z) + abs(dst_y) + abs(dst_x);
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].sent = false;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].rcvd = false;
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].payload = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
									pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].valid = true;
									global_injection_packet_size[cur_inject_dir - 1][z][y][x]++;
									total_packet_sent++;
								}

							}
						}
					}
				}
			}
		}
	}
	int port_used_counter = 0;
	for (int jj = 0; jj < PORT_NUM; ++jj){
		if (port_used[jj])
			port_used_counter++;
	}
	return port_used_counter;

}

// generate random pattern, randomly send to 8 of the nodes in the network
int gen_pattern_random(int pattern_size) { 
	int packet_num = 8;
	total_packet_sent = 0;
	char cur_inject_dir;
	bool port_used[PORT_NUM];
	for (int ii = 0; ii < PORT_NUM; ++ii) {
		port_used[ii] = false;
	}
	// allocate and initialize pattern array
	for (int z = 0; z < ZSIZE; ++z) {
		for (int y = 0; y < YSIZE; ++y) {
			for (int x = 0; x < XSIZE; ++x) {
				for (int i = 0; i < PORT_NUM; ++i) {
					global_injection_packet_size[i][z][y][x] = 0;
					if (!(pattern[i][z][y][x] = (packet*)malloc(packet_num * pattern_size * sizeof(packet)))) {//worst case, all the packets are injected in a single injection port
						printf("error when allocating space for pattern\n");
						exit(-1);
					}
					for (int j = 0; j < packet_num * pattern_size; ++j) {
						pattern[i][z][y][x][j].valid = false;
						pattern[i][z][y][x][j].rcvd = false;
						pattern[i][z][y][x][j].sent = false;
					}
				}
			}
		}
	}
	// assign array elements
	for (int j = 0; j < pattern_size; ++j) {
		for (int z = 0; z < ZSIZE; ++z) {
			for (int y = 0; y < YSIZE; ++y) {
				for (int x = 0; x < XSIZE; ++x) {
					for (int m = 0; m < packet_num; ++m) {
						int real_dst_x = rand() % XSIZE;
						int real_dst_y = rand() % YSIZE;
						int real_dst_z = rand() % ZSIZE;
						if (real_dst_x < 0 || real_dst_x >= XSIZE) {
							printf("error, dst error\n");
							exit(-1);
						}
						if (real_dst_y < 0 || real_dst_y >= YSIZE) {
							printf("error, dst error\n");
							exit(-1);
						}
						if (real_dst_z < 0 || real_dst_z >= ZSIZE) {
							printf("error, dst error\n");
							exit(-1);
						}

						cur_inject_dir = comp_inject_dir(x, y, z, real_dst_x, real_dst_y, real_dst_z);

						if (cur_inject_dir != DIR_EJECT) {
							port_used[cur_inject_dir - 1] = true;
							pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].inject_dir = cur_inject_dir;
							pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_x = x;
							pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_y = y;
							pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_z = z;
							pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].id = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
							pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_x = real_dst_x;
							pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_y = real_dst_y;
							pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_z = real_dst_z;
							pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].mahattan_dist = abs(real_dst_x - x) + abs(real_dst_y - y) + abs(real_dst_z - z);
							pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].sent = false;
							pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].rcvd = false;
							pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].payload = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
							pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].valid = true;
							global_injection_packet_size[cur_inject_dir - 1][z][y][x]++;
							total_packet_sent++;
						}

					}
				} // end of generting patterns for one node

			}
		}
	}
	int port_used_counter = 0;
	for (int jj = 0; jj < PORT_NUM; ++jj) {
		if (port_used[jj])
			port_used_counter++;
	}
	return port_used_counter;

}

// generate mixed pattern, send to cube nearest neighbor, but for the 8 corner nodes, send to 9 hop NN instead of 3 hop NN
int gen_pattern_neighbor_and_far(int pattern_size) { //each node multicast to 26 nearest neighbors
	total_packet_sent = 0;
	bool port_used[PORT_NUM];
	for (int ii = 0; ii < PORT_NUM; ++ii) {
		port_used[ii] = false;
	}
	char cur_inject_dir;
	// allocate and initialize pattern array
	for (int z = 0; z < ZSIZE; ++z) {
		for (int y = 0; y < YSIZE; ++y) {
			for (int x = 0; x < XSIZE; ++x) {
				for (int i = 0; i < PORT_NUM; ++i) {
					global_injection_packet_size[i][z][y][x] = 0;
					if (!(pattern[i][z][y][x] = (packet*)malloc(26 * pattern_size * sizeof(packet)))) {//worst case, all the packets are injected in a single injection port
						printf("error when allocating space for pattern\n");
						exit(-1);
					}
					for (int j = 0; j < 26 * pattern_size; ++j) {
						pattern[i][z][y][x][j].valid = false;
						pattern[i][z][y][x][j].rcvd = false;
						pattern[i][z][y][x][j].sent = false;
					}
				}
			}
		}
	}
	// assign array elements
	for (int j = 0; j < pattern_size; ++j) {
		for (int z = 0; z < ZSIZE; ++z) {
			for (int y = 0; y < YSIZE; ++y) {
				for (int x = 0; x < XSIZE; ++x) {
					for (int dst_z = -1; dst_z <= 1; ++dst_z) {
						for (int dst_y = -1; dst_y <= 1; ++dst_y) {
							for (int dst_x = -1; dst_x <= 1; ++dst_x) {
								// if this is not the 3HOP corner node
								if (abs(dst_z) + abs(dst_y) + abs(dst_x) != 3) {
									// get the real desination in torus network
									int real_dst_x = (x + dst_x < 0) ? (x + dst_x + XSIZE) : ((x + dst_x >= XSIZE) ? (x + dst_x - XSIZE) : (x + dst_x));
									int real_dst_y = (y + dst_y < 0) ? (y + dst_y + YSIZE) : ((y + dst_y >= YSIZE) ? (y + dst_y - YSIZE) : (y + dst_y));
									int real_dst_z = (z + dst_z < 0) ? (z + dst_z + ZSIZE) : ((z + dst_z >= ZSIZE) ? (z + dst_z - ZSIZE) : (z + dst_z));
									// get the injection port
									cur_inject_dir = comp_inject_dir(x, y, z, real_dst_x, real_dst_y, real_dst_z);
									// assign the pattern entry
									if (cur_inject_dir != DIR_EJECT) {
										port_used[cur_inject_dir - 1] = true;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].inject_dir = cur_inject_dir;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_x = x;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_y = y;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_z = z;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].id = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_x = real_dst_x;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_y = real_dst_y;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_z = real_dst_z;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].mahattan_dist = abs(dst_z) + abs(dst_y) + abs(dst_x);
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].sent = false;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].rcvd = false;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].payload = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].valid = true;
										global_injection_packet_size[cur_inject_dir - 1][z][y][x]++;
										total_packet_sent++;
									}
								}
								// When it's the corner node, then replace it with the 9 HOP node
								else {
									int tmp_dst_x = 3 * dst_x;
									int tmp_dst_y = 3 * dst_y;
									int tmp_dst_z = 3 * dst_z;
									// get the real desination in torus network
									int real_dst_x = (x + tmp_dst_x < 0) ? (x + tmp_dst_x + XSIZE) : ((x + tmp_dst_x >= XSIZE) ? (x + tmp_dst_x - XSIZE) : (x + tmp_dst_x));
									int real_dst_y = (y + tmp_dst_y < 0) ? (y + tmp_dst_y + YSIZE) : ((y + tmp_dst_y >= YSIZE) ? (y + tmp_dst_y - YSIZE) : (y + tmp_dst_y));
									int real_dst_z = (z + tmp_dst_z < 0) ? (z + tmp_dst_z + ZSIZE) : ((z + tmp_dst_z >= ZSIZE) ? (z + tmp_dst_z - ZSIZE) : (z + tmp_dst_z));
									// get the injection port
									cur_inject_dir = comp_inject_dir(x, y, z, real_dst_x, real_dst_y, real_dst_z);
									// assign the pattern entry
									if (cur_inject_dir != DIR_EJECT) {
										port_used[cur_inject_dir - 1] = true;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].inject_dir = cur_inject_dir;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_x = x;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_y = y;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_z = z;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].id = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_x = real_dst_x;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_y = real_dst_y;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_z = real_dst_z;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].mahattan_dist = abs(dst_z) + abs(dst_y) + abs(dst_x);
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].sent = false;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].rcvd = false;
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].payload = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
										pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].valid = true;
										global_injection_packet_size[cur_inject_dir - 1][z][y][x]++;
										total_packet_sent++;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	int port_used_counter = 0;
	for (int jj = 0; jj < PORT_NUM; ++jj) {
		if (port_used[jj])
			port_used_counter++;
	}
	return port_used_counter;

}

// Bit reversal on 3-bit digits
int bit_reverse_3_bit(int input_number) {
	if (input_number > 7 || input_number < 0) {
		printf("Error: when generating bit reverse pattern, the input number exceed the 0~7 range!\n\n");
		exit(-1);
	}

	switch (input_number) {
	case 0: return 7;
	case 1: return 6;
	case 2: return 5;
	case 3: return 4;
	case 4: return 3;
	case 5: return 2;
	case 6: return 1;
	case 7:return 0;
	}
}

// Bit shuffle on 3-bit digits
int bit_shuffle_3_bit(int input_number) {
	if (input_number > 7 || input_number < 0) {
		printf("Error: when generating bit reverse pattern, the input number exceed the 0~7 range!\n\n");
		exit(-1);
	}

	switch (input_number) {
	case 0: return 0;
	case 1: return 2;
	case 2: return 4;
	case 3: return 6;
	case 4: return 1;
	case 5: return 3;
	case 6: return 5;
	case 7:return 7;
	}
}

// Each node send to 1 shuffle node (x_shuf, y_shuf, z_shuf)
// Notice: for 3-bit digit, shuffle 3 times will bring it back to original number
int gen_pattern_shuffle(int pattern_size) {
	total_packet_sent = 0;
	char cur_inject_dir;
	bool port_used[PORT_NUM];
	for (int ii = 0; ii < PORT_NUM; ++ii) {
		port_used[ii] = false;
	}
	// allocate and initialize pattern array
	for (int z = 0; z < ZSIZE; ++z) {
		for (int y = 0; y < YSIZE; ++y) {
			for (int x = 0; x < XSIZE; ++x) {
				for (int i = 0; i < PORT_NUM; ++i) {
					global_injection_packet_size[i][z][y][x] = 0;
					if (!(pattern[i][z][y][x] = (packet*)malloc(8 * pattern_size * sizeof(packet)))) {//worst case, all the packets are injected in a single injection port
						printf("error when allocating space for pattern\n");
						exit(-1);
					}
					for (int j = 0; j < pattern_size; ++j) {
						pattern[i][z][y][x][j].valid = false;
						pattern[i][z][y][x][j].rcvd = false;
						pattern[i][z][y][x][j].sent = false;
					}
				}
			}
		}
	}
	// assign array elements
	for (int j = 0; j < pattern_size; ++j) {
		for (int z = 0; z < ZSIZE; ++z) {
			for (int y = 0; y < YSIZE; ++y) {
				for (int x = 0; x < XSIZE; ++x) {
					int x_shuf = bit_shuffle_3_bit(x);
					int y_shuf = bit_shuffle_3_bit(y);
					int z_shuf = bit_shuffle_3_bit(z);
					int x_shuf_shuf = bit_shuffle_3_bit(x_shuf);
					int y_shuf_shuf = bit_shuffle_3_bit(y_shuf);
					int z_shuf_shuf = bit_shuffle_3_bit(z_shuf);

					// send to node (x_shuf, y_shuf, z_shuf)
					int dst_x = x_shuf;
					int dst_y = y_shuf;
					int dst_z = z_shuf;
					cur_inject_dir = comp_inject_dir(x, y, z, dst_x, dst_y, dst_z);
					if (cur_inject_dir != DIR_EJECT) {
						port_used[cur_inject_dir - 1] = true;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].inject_dir = cur_inject_dir;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_x = x;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_y = y;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_z = z;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].id = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_x = dst_x;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_y = dst_y;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_z = dst_z;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].mahattan_dist = abs(dst_z) + abs(dst_y) + abs(dst_x);
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].sent = false;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].rcvd = false;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].payload = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].valid = true;
						global_injection_packet_size[cur_inject_dir - 1][z][y][x]++;
						total_packet_sent++;
					}
/*
					// send to node (x_shuf_shuf, y_shuf_shuf, z_shuf_shuf)
					dst_x = x_shuf_shuf;
					dst_y = y_shuf_shuf;
					dst_z = z_shuf_shuf;
					cur_inject_dir = comp_inject_dir(x, y, z, dst_x, dst_y, dst_z);
					if (cur_inject_dir != DIR_EJECT) {
						port_used[cur_inject_dir - 1] = true;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].inject_dir = cur_inject_dir;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_x = x;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_y = y;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].src_z = z;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].id = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_x = dst_x;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_y = dst_y;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].dst_z = dst_z;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].mahattan_dist = abs(dst_z) + abs(dst_y) + abs(dst_x);
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].sent = false;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].rcvd = false;
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].payload = global_injection_packet_size[cur_inject_dir - 1][z][y][x];
						pattern[cur_inject_dir - 1][z][y][x][global_injection_packet_size[cur_inject_dir - 1][z][y][x]].valid = true;
						global_injection_packet_size[cur_inject_dir - 1][z][y][x]++;
						total_packet_sent++;
					}
*/
				}
			}
		}
	}
	int port_used_counter = 0;
	for (int jj = 0; jj < PORT_NUM; ++jj) {
		if (port_used[jj])
			port_used_counter++;
	}
	return 1;
}

void clear_pattern(){
	for (int z = 0; z < ZSIZE; ++z){
		for (int y = 0; y < YSIZE; ++y){
			for (int x = 0; x < XSIZE; ++x){
				for (int i = 0; i < PORT_NUM; ++i){
					for (int j = 0; j < global_injection_packet_size[i][z][y][x]; ++j){
						pattern[i][z][y][x][j].sent = false;
						pattern[i][z][y][x][j].rcvd = false;
					}
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stats collection
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Count how many packets has finished sent and received in total
// Performed in each cycle
bool count_sent_and_rcvd(int *send_num, int* rcvd_num, int* cycle_counter){
    int cur_sent_num = 0;
    int cur_rcvd_num = 0;
    for(int i = 0; i < PORT_NUM; ++i){
		for (int j = 0; j < ZSIZE; ++j){
            for(int k = 0; k < YSIZE; ++k){
                for(int m = 0; m < XSIZE; ++m){
                    for(int n = 0; n < global_injection_packet_size[i][j][k][m]; ++n){
                        if(pattern[i][j][k][m][n].valid && pattern[i][j][k][m][n].sent)
                            cur_sent_num++;
                        if(pattern[i][j][k][m][n].valid && pattern[i][j][k][m][n].rcvd)
                            cur_rcvd_num++;
                    }
                }
            }
        }    
    }
	*send_num = cur_sent_num;
	*rcvd_num = cur_rcvd_num;
    printf("At cycle %d, total %d pckts, sent %d pckts, rcvd %d pckts\n", *cycle_counter, total_packet_sent, cur_sent_num, cur_rcvd_num);
    return cur_rcvd_num == total_packet_sent;
}

// Debugging: print out the packets than have been send out but not been received
bool print_unrcvd() {
	int cur_sent_num = 0;
	int cur_rcvd_num = 0;
	for (int i = 0; i < PORT_NUM; ++i) {
		for (int j = 0; j < ZSIZE; ++j) {
			for (int k = 0; k < YSIZE; ++k) {
				for (int m = 0; m < XSIZE; ++m) {
					for (int n = 0; n < global_injection_packet_size[i][j][k][m]; ++n) {
						if (pattern[i][j][k][m][n].valid && pattern[i][j][k][m][n].sent)
							cur_sent_num++;
						if (pattern[i][j][k][m][n].valid && pattern[i][j][k][m][n].rcvd)
							cur_rcvd_num++;
						if (pattern[i][j][k][m][n].valid && pattern[i][j][k][m][n].sent && !pattern[i][j][k][m][n].rcvd)
							printf("(%d,%d,%d) to (%d,%d,%d), packet id %d,sent at %dth cycle, injection dir %d, is not rcvd\n", pattern[i][j][k][m][n].src_x, pattern[i][j][k][m][n].src_y, pattern[i][j][k][m][n].src_z, \
								pattern[i][j][k][m][n].dst_x, pattern[i][j][k][m][n].dst_y, pattern[i][j][k][m][n].dst_z, pattern[i][j][k][m][n].id, pattern[i][j][k][m][n].send_time_stamp, pattern[i][j][k][m][n].inject_dir);
					}
				}
			}
		}
	}
	printf("total %d pckts, sent %d pckts, rcvd %d pckts\n", total_packet_sent, cur_sent_num, cur_rcvd_num);
	return cur_rcvd_num == total_packet_sent;
}

// Count the total number of packets
int count_packet(){
	int packet_counter = 0;
	for (int i = 0; i < PORT_NUM; ++i){
		for (int j = 0; j < ZSIZE; ++j){
			for (int k = 0; k < YSIZE; ++k){
				for (int m = 0; m < XSIZE; ++m){
					for (int n = 0; n < global_injection_packet_size[i][j][k][m]; ++n){
						if (pattern[i][j][k][m][n].valid){
							packet_counter++;
						}
					}
				}
			}
		}
	}
	return packet_counter;
}

// Performance: Print out the total latency when the simulation is finished
void print_stats(int* total_packet, int total_latency, float* Avg_latency, int* Worst_case_latency, std::string* Worst_case_packet_info){
    int packet_counter = 0;
    float avg_latency = 0;
    int worst_case_latency = 0;
	packet worst_case_packet;
	char worst_case_packet_info[100];
    for(int i = 0; i < PORT_NUM; ++i){
		for (int j = 0; j < ZSIZE; ++j){
            for(int k = 0; k < YSIZE; ++k){
                for(int m = 0; m < XSIZE; ++m){
                    for(int n = 0; n < global_injection_packet_size[i][j][k][m]; ++n){
                        if(pattern[i][j][k][m][n].valid && pattern[i][j][k][m][n].sent && pattern[i][j][k][m][n].rcvd){
                            avg_latency = (avg_latency * packet_counter + pattern[i][j][k][m][n].recv_time_stamp - pattern[i][j][k][m][n].send_time_stamp) / (float)(packet_counter + 1);
							if (worst_case_latency < pattern[i][j][k][m][n].recv_time_stamp - pattern[i][j][k][m][n].send_time_stamp){
								worst_case_latency = pattern[i][j][k][m][n].recv_time_stamp - pattern[i][j][k][m][n].send_time_stamp;
								worst_case_packet = pattern[i][j][k][m][n];
							}
                            packet_counter++;
                        }
                    }
                }
            }
        }    
    }
	printf("among %d packets, total latency is %d, avg latency is %f cycles, worst case packet takes %d cycles\n", packet_counter, total_latency, avg_latency, worst_case_latency);
	sprintf(worst_case_packet_info, "worst packet from (%d, %d, %d) to (%d, %d, %d), packet id is %d", worst_case_packet.src_x, worst_case_packet.src_y, worst_case_packet.src_z, worst_case_packet.dst_x, worst_case_packet.dst_y, worst_case_packet.dst_z, worst_case_packet.id);
	
	
	*total_packet = packet_counter;
	*Avg_latency = avg_latency;
	*Worst_case_latency = worst_case_latency;
	*Worst_case_packet_info = std::string(worst_case_packet_info);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN Function
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]){

	if (argc != 11){
		printf("how to use: ./sim -i [pattern id] -s [pattern size] -p [packet size] -g [injection_gap] -v [VC_num]\n");
		printf("id list: 0: nearest neighbor\n \
			   1: three-hop diagonal nearest neighbor\n \
			   2: cube nearest neighbor\n \
			   3: bit complement\n \
			   4: transpose\n \
			   5: tornado\n \
			   6: all_to_all\n \
			   7: random\n \
			   8: mix cube nearest neighbor and 9-hop nearest neighbor\n \
			   9: shuffle\n \
			   Recommend: packet size smaller than VC_size\n");
		exit(-1);
	}
	
	int pattern_id = atoi(argv[2]);
	int pattern_size = atoi(argv[4]);
	int packet_size = atoi(argv[6]);
	if (packet_size > VC_SIZE){
		printf("Waring! if packet size is bigger than VC size, the performance will significantly drop\n");
	}

	int injection_gap = atoi(argv[8]);			// injection rate = packet_size / (packet_size + injection_gap)

	int VC_num = atoi(argv[10]);					// allowed # of VC to be used in the simulation
	if (VC_num < 2 || VC_num > 9) {
		printf("Warning! the input VC num exceed the range between 2-9, manually set the VC number as 9!\n");
		VC_num = 9;
	}

	//std::string common_path = "/home/jysheng/Documents/dynamic_router_sim/";
	std::string common_path = "/mnt/nokrb/cyang90/results/dynamic_router_sim/";

	char cstr_xsize[5];
	char cstr_ysize[5];
	char cstr_zsize[5];

	sprintf(cstr_xsize, "%d", XSIZE);
	sprintf(cstr_ysize, "%d", YSIZE);
	sprintf(cstr_zsize, "%d", ZSIZE);


	std::string size_path = std::string(cstr_xsize) + "x" + std::string(cstr_ysize) + "x" + std::string(cstr_zsize) + "/";

	std::string pattern_path[10];
	pattern_path[0] = "NN";
	pattern_path[1] = "3H_NN";
	pattern_path[2] = "CUBE_NN";
	pattern_path[3] = "bit_complement";
	pattern_path[4] = "transpose";
	pattern_path[5] = "tornado";
	pattern_path[6] = "all_to_all";
	pattern_path[7] = "random";
	pattern_path[8] = "mix_CUBE_NN_and_9H";
	pattern_path[9] = "shuffle";

	std::string selected_pattern_path = pattern_path[pattern_id];

	std::string filename = selected_pattern_path + "_" + argv[4] + "_" + argv[6] + "_" + argv[8] + "_" + argv[10] + ".csv";
	//:std::string filename = common_path + selected_pattern_path + "_" + argv[4] + "_" + argv[6] + "_" + argv[8] + ".csv";

	//std::string output_path = common_path + size_path + selected_pattern_path + "/" + filename;
	std::string output_path = filename;
	std::ofstream fout;
	fout.open(output_path.c_str());
	if (fout.fail()){
		printf("open %s failed, exiting\n", output_path.c_str());
		exit(-1);
	}

	
	int port_used_num = 0;
	if (pattern_id == 0){
		port_used_num = gen_pattern_nearest_neighbor(pattern_size);
	}
	else if (pattern_id == 1){
		port_used_num = gen_pattern_three_hop_diagonal(pattern_size);
	}
	else if (pattern_id == 2){
		port_used_num = gen_pattern_cube_nearest_neighbor(pattern_size);
	}
	else if (pattern_id == 3){
		port_used_num = gen_pattern_bitcomplement(pattern_size);
	}
	else if (pattern_id == 4){
		port_used_num = gen_pattern_transpose(pattern_size);
	}
	else if (pattern_id == 5){
		port_used_num = gen_pattern_tornado(pattern_size);
	}
	else if (pattern_id == 6){
		port_used_num = gen_pattern_all_to_all(pattern_size);
	}
	else if (pattern_id == 7) {
		port_used_num = gen_pattern_random(pattern_size);
	}
	else if (pattern_id == 8) {
		port_used_num = gen_pattern_neighbor_and_far(pattern_size);
	}
	else if (pattern_id == 9) {
		port_used_num = gen_pattern_shuffle(pattern_size);
	}
	
	

	int global_best_max_VCs = 9;
	int global_worst_max_VCs = 0;
	float global_avg_max_VCs = 0;


	int global_best_total_latency = 1000000;
	int global_worst_total_latency = 0;
	float global_avg_total_latency = 0;

	float global_best_avg_latency = 100000;
	float global_worst_avg_latency = 0;
	float global_avg_avg_latency = 0;

	int global_best_worst_case_latency = 100000;
	int global_worst_worst_case_latency = 0;
	float global_avg_worst_case_latency = 0;

	double global_best_max_thruput = 0;
	double global_worst_max_thruput = 100000;
	double global_avg_max_thruput = 0;

	double global_best_max_sent_thruput = 0;
	double global_worst_max_sent_thruput = 100000;
	double global_avg_max_sent_thruput = 0;

	double global_best_avg_rcvd_thruput = 0;
	double global_worst_avg_rcvd_thruput = 100000;
	double global_avg_avg_rcvd_thruput = 0;

	double global_best_avg_sent_thruput = 0;
	double global_worst_avg_sent_thruput = 100000;
	double global_avg_avg_sent_thruput = 0;


	double max_thruput = 0;  //max receiving thruput during a certain amount of time
	double max_sent_thruput = 0;
	double avg_sent_thruput = 0;
	double avg_rcvd_thruput = 0;

	fout << "pattern name, pattern_size, packet_size, allowed_VC_num" << std::endl;
	fout << selected_pattern_path << ", " << pattern_size << ", " << packet_size << ", " << VC_num << std::endl;

	fout << "routing_mode, " << "SA_mode, " << "total packet num, " << "total latency, " << "avg latency, " << "worst case latency, " << "avg rcv thruput, " \
		<< "avg sent thruput, " << "max rcv thruput, " << " max sent thruput, " << " offered send thruput, " << " max used VC NUM, " << "worst case packet info, " << "saturation status" << std::endl;

	srand((unsigned)time(NULL));

	int case_num = 0;
    

	// clear the staturation status for each algorithm combination, output port, each node
	for (int i = 0; i < 15; ++i) {
		for (int j = 0; j < PORT_NUM; ++j){
			for (int z = 0; z < ZSIZE; ++z) {
				for (int y = 0; y < YSIZE; ++y) {
					for (int x = 0; x < XSIZE; ++x) {
						staturation_status[i][j][z][y][x] = false;
					}
				}
			}
		}
	}

	// clear the injection block counter for each algorithm combination, output port, each node
	for (int i = 0; i < 15; ++i) {
		for (int j = 0; j < PORT_NUM; ++j) {
			for (int z = 0; z < ZSIZE; ++z) {
				for (int y = 0; y < YSIZE; ++y) {
					for (int x = 0; x < XSIZE; ++x) {
						injection_blocked_counter[i][j][z][y][x] = 0;
					}
				}
			}
		}
	}

//	for (int routing_mode_i = 2; routing_mode_i < 3; routing_mode_i++) {
//		for (int SA_mode_j = 0; SA_mode_j < 1; SA_mode_j++) {
	for (int routing_mode_i = 0; routing_mode_i < ROUTING_MODE_NUM; routing_mode_i++){
		for (int SA_mode_j = 0; SA_mode_j < SA_MODE_NUM; SA_mode_j++){
			printf("------------------------------------------Current routing algorithm is %d, swtich algorithm is %d-----------------------------------------------------\n", routing_mode_i, SA_mode_j);
			//result list:
			int cycle_counter = 0;
			int max_VCs = 0;
			int total_packet_num = count_packet();

			int total_latency;
			float avg_latency;
			int worst_case_latency;
			std::string worst_case_packet_info;

			double max_thruput = 0;  //max receiving thruput during a certain amount of time
			double max_sent_thruput = 0;
			double avg_sent_thruput = 0;
			double avg_rcvd_thruput = 0;



			int check_period = 2 * injection_gap > 100 ? 2 * injection_gap : 100;

			
			network network_UUT;
			

			int all_sent_stamp = 0;
			bool all_sent = false;
			bool start_rcvd = false;
			int start_rcvd_stamp = 0;
			int pre_sent_counter = 0;
			int cur_sent_counter = 0;
			int pre_packet_counter = 0;
			int pre_rcvd_counter = 0;
			int cur_received_counter = 0;
			network_UUT.network_init(XSIZE, YSIZE, ZSIZE, 0, routing_mode_i, SA_mode_j, injection_gap, packet_size, VC_num);
			while (1){
				if (network_UUT.consume() == -1){
					break;
				}
				if (network_UUT.produce() == -1){
					break;
				}

				if (cycle_counter % 1 == 0){
//					printf("%dth cycle:\n", cycle_counter);
				}

				// Performance info: collect packet sending and receiving status
				if (count_sent_and_rcvd(&cur_sent_counter, &cur_received_counter, &cycle_counter)){
					break;
				}

				// Performance info: collect the time that all the packet has been successfully send out
				if (cur_sent_counter == total_packet_num){
					all_sent_stamp = cycle_counter;
					all_sent = true;
				}

				// Performance info: Get the first clock cycle that a packet arrived at the destination node, Finish time - start_rcvd_stamp is the total time used for receiving
				if ((cur_received_counter > 0) && (pre_rcvd_counter == 0)){
					start_rcvd_stamp = cycle_counter;
				}

				if (cycle_counter % check_period == 0) {
					double sent_thruput = ((cur_sent_counter - pre_sent_counter) / (double)check_period) * packet_size / (XSIZE * YSIZE * ZSIZE);
					double thruput = ((cur_received_counter - pre_packet_counter) / (double)check_period) * packet_size / (XSIZE * YSIZE * ZSIZE);
					int tmp = network_UUT.network_max_busy_VC_num();
					if (tmp > max_VCs)
						max_VCs = tmp;
					if (thruput > max_thruput)
						max_thruput = thruput;
					if (sent_thruput > max_sent_thruput)
						max_sent_thruput = sent_thruput;
					printf("%dth cycle: max_VCs is %d, thruput is %f\n", cycle_counter, tmp, thruput);
					pre_packet_counter = cur_received_counter;
					pre_sent_counter = cur_sent_counter;
				}

				// Stop the entire simulation process if a certain number of cycles has elapsed
				if (cycle_counter == MAX_SIMULATION_CYCLE){
					printf("Maximum simulation cycles %d has been simulated, stop\n", MAX_SIMULATION_CYCLE);
					if (!all_sent){
						all_sent_stamp = cycle_counter;
					}

					// print out those packets that have been send out but not been received when simulation time out
					printf("\n=========================================================================\n");
					printf("Failed pattern list (sent but not received)\n");
					for (int port = 0; port < PORT_NUM; ++port) {
						for (int z = 0; z < ZSIZE; ++z) {
							for (int y = 0; y < YSIZE; ++y) {
								for (int x = 0; x < XSIZE; ++x) {
									for (int pack_id = 0; pack_id < global_injection_packet_size[port][z][y][x]; ++pack_id) {
										if (pattern[port][z][y][x][pack_id].valid && pattern[port][z][y][x][pack_id].sent && !pattern[port][z][y][x][pack_id].rcvd) {
											printf("Pattern[%d][%d][%d][%d][%d]\n", port, z, y, x, pack_id);
										}
									}
								}
							}
						}
					}
					printf("=========================================================================\n\n");

					break;
				}

				pre_rcvd_counter = cur_received_counter;
				cycle_counter++;
			}	// end of the inner loop, each loop is one cycle

			avg_sent_thruput = (double)cur_sent_counter * (double)packet_size / all_sent_stamp / (XSIZE * YSIZE * ZSIZE);
			avg_rcvd_thruput = (double)cur_received_counter * (double)packet_size / (cycle_counter - start_rcvd_stamp) / (XSIZE * YSIZE * ZSIZE);
			print_stats(&total_packet_num, cycle_counter, &avg_latency, &worst_case_latency, &worst_case_packet_info);
			printf("current run: %s\n", filename.c_str());
			printf("overall max_VCs is %d,port_used %d, offered thruput is %f flits/node/cycle, max sent thruput is %f flits/node/cycle, max thruput is %f flits/node/cycle\n", max_VCs, port_used_num, (double)(packet_size * port_used_num) / (packet_size + injection_gap), max_sent_thruput, max_thruput);
			printf("avg sent thruput is %f flits/node/cycle, avg rcvd thruput is %f flits/node/cycle\n", avg_sent_thruput, avg_rcvd_thruput);

			total_latency = cycle_counter;

			// get the saturation status
			int satured = 0;
			for (int i = 0; i < 15; ++i) {
				for (int j = 0; j < PORT_NUM; ++j) {
					for (int z = 0; z < ZSIZE; ++z) {
						for (int y = 0; y < YSIZE; ++y) {
							for (int x = 0; x < XSIZE; ++x) {
								if (staturation_status[i][j][z][y][x]) {
									satured = 1;
								}
							}
						}
					}
				}
			}

			//////////////////////////////////////////////////////////////////////////////////////
			// Debugging:
			// Generate the output .xls file
			//////////////////////////////////////////////////////////////////////////////////////
			fout << routing_mode_i << ", " << SA_mode_j << ", " << total_packet_num << ", " << total_latency << ", " << avg_latency << ", " << worst_case_latency << ", " \
				<< avg_rcvd_thruput << ", " << avg_sent_thruput << ", " << max_sent_thruput << ", " << max_thruput << ", " \
				<< (double)(packet_size * port_used_num) / (packet_size + injection_gap) << ", " << max_VCs << ", " << worst_case_packet_info << ", " << satured << "\n" << std::endl;

			if (total_latency < global_best_total_latency){
				global_best_total_latency = total_latency;
			}
			if (total_latency > global_worst_total_latency){
				global_worst_total_latency = total_latency;
			}
			global_avg_total_latency = (global_avg_total_latency * case_num + total_latency) / (float)(case_num + 1);

			if (avg_latency < global_best_avg_latency){
				global_best_avg_latency = avg_latency;
			}
			if (avg_latency > global_worst_avg_latency){
				global_worst_avg_latency = avg_latency;
			}
			global_avg_avg_latency = (global_avg_avg_latency * case_num + avg_latency) / (float)(case_num + 1);

			if (worst_case_latency < global_best_worst_case_latency){
				global_best_worst_case_latency = worst_case_latency;
			}
			if (worst_case_latency > global_worst_worst_case_latency){
				global_worst_worst_case_latency = worst_case_latency;
			}
			global_avg_worst_case_latency = (global_avg_worst_case_latency * case_num + worst_case_latency) / (float)(case_num + 1);

			if (max_VCs < global_best_max_VCs){
				global_best_max_VCs = max_VCs;
			}
			if (max_VCs > global_worst_max_VCs){
				global_worst_max_VCs = max_VCs;
			}
			global_avg_max_VCs = (global_avg_max_VCs * case_num + max_VCs) / (float)(case_num + 1);

			if (avg_rcvd_thruput > global_best_avg_rcvd_thruput){
				global_best_avg_rcvd_thruput = avg_rcvd_thruput;
			}
			if (avg_rcvd_thruput < global_worst_avg_rcvd_thruput){
				global_worst_avg_rcvd_thruput = avg_rcvd_thruput;
			}
			global_avg_avg_rcvd_thruput = (global_avg_avg_rcvd_thruput * case_num + avg_rcvd_thruput) / (float)(case_num + 1);

			if (avg_sent_thruput > global_best_avg_sent_thruput){
				global_best_avg_sent_thruput = avg_sent_thruput;
			}
			if (avg_sent_thruput < global_worst_avg_sent_thruput){
				global_worst_avg_sent_thruput = avg_sent_thruput;
			}
			global_avg_avg_sent_thruput = (global_avg_avg_sent_thruput * case_num + avg_sent_thruput) / (float)(case_num + 1);

			if (max_sent_thruput > global_best_max_sent_thruput){
				global_best_max_sent_thruput = max_sent_thruput;
			}
			if (max_sent_thruput < global_worst_max_sent_thruput){
				global_worst_max_sent_thruput = max_sent_thruput;
			}
			global_avg_max_sent_thruput = (global_avg_max_sent_thruput * case_num + max_sent_thruput) / (float)(case_num + 1);

			if (max_thruput > global_best_max_thruput){
				global_best_max_thruput = max_thruput;
			}
			if (max_thruput < global_worst_max_thruput){
				global_worst_max_thruput = max_thruput;
			}
			global_avg_max_thruput = (global_avg_max_thruput * case_num + max_thruput) / (float)(case_num + 1);



			network_UUT.network_free();
			case_num++;
			clear_pattern();
		}
	}
	fout << "best" << ", " << "best" << ", " << " " << ", " << global_best_total_latency << ", " << global_best_avg_latency << ", " << global_best_worst_case_latency << ", " \
		<< global_best_avg_rcvd_thruput << ", " << global_best_avg_sent_thruput << ", " << global_best_max_sent_thruput << ", " << global_best_max_thruput << ", " \
		<< (double)(packet_size * port_used_num) / (packet_size + injection_gap) << ", " << global_best_max_VCs << ", " << " " << std::endl;
	fout << "avg" << ", " << "avg" << ", " << " " << ", " << global_avg_total_latency << ", " << global_avg_avg_latency << ", " << global_avg_worst_case_latency << ", " \
		<< global_avg_avg_rcvd_thruput << ", " << global_avg_avg_sent_thruput << ", " << global_avg_max_sent_thruput << ", " << global_avg_max_thruput << ", " \
		<< (double)(packet_size * port_used_num) / (packet_size + injection_gap) << ", " << global_avg_max_VCs << ", " << " " << std::endl;
	fout << "worst" << ", " << "worst" << ", " << " " << ", " << global_worst_total_latency << ", " << global_worst_avg_latency << ", " << global_worst_worst_case_latency << ", " \
		<< global_worst_avg_rcvd_thruput << ", " << global_worst_avg_sent_thruput << ", " << global_worst_max_sent_thruput << ", " << global_worst_max_thruput << ", " \
		<< (double)(packet_size * port_used_num) / (packet_size + injection_gap) << ", " << global_worst_max_VCs << ", " << " " << std::endl;

	fout.close();
	

	return 0;
}
