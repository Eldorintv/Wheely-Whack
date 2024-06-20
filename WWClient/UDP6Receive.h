#ifndef UDP6RECIEVE_H
#define UDP6RECIEVE_H


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

#include <unordered_set>

class UDPReceive6 {

public:
	int sock;
	struct sockaddr_in6 addr;
	char* recbuffer;
	std::unordered_set<int> receivedPackets;
	int recvPacketCount;
	int lastPacketNum;

	UDPReceive6();
	~UDPReceive6() { delete recbuffer; };
	void init(int port);
	int receive(char* buffer, int len, double* ptime);
	void closeSock();
};

#endif
