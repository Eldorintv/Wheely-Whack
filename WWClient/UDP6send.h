#ifndef UDPSEND_H
#define UDPSEND_H

#pragma comment(lib, "ws2_32.lib")

/*
 *  UDPSend6.h
 *
 *  Created by Helmut Hlavacs (2022).
 *
 */


extern "C" {
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <in6addr.h>
#include <ws2ipdef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <WS2tcpip.h>
}

#include <cstdint>


int startWinsock(void);



class UDPSend6 {

public:
	int sock = 0;
	struct sockaddr_in6 addr;
	unsigned int packetnum = 0;

	UDPSend6() {};
	~UDPSend6() {};
	void init(const char* address, int port);
	int send(char* buffer, int len);
	void closeSock();
};

#endif


