#ifndef _TRANSPORT_H_
#define	_TRANSPORT_H_

#include "stm32f4xx.h"

int transport_sendPacketBuffer(unsigned char* buf, int buflen);
int transport_getdata(unsigned char* buf, int count);
int transport_getdatanb(void *sck, unsigned char* buf, int count);
int transport_open(char* host, int port);
int transport_close(int sock);


#endif

