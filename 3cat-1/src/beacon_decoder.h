#ifndef __BEACON_DECODER_H__
#define __BEACON_DECODER_H__

#include "beacon_3cat1.h"
#include "socket_utils.h"
#include <mysql.h>


#define M_USRP_HOST_IP "127.0.0.1"

void print_beacon_data(unsigned char *buf, size_t size);
void save_beacon_data(unsigned char *buf, size_t size);
int beacon_receive_packet(int spifd, unsigned char *data, int *len);

#endif
