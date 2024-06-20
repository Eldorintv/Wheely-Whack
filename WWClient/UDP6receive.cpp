extern "C" {
#include <stdio.h>
#include <time.h>
#include <cstdint>
}
#include <iostream>
#include <vector>
#include <chrono>
#include <map>

#include <thread>

#include "UDP6Receive.h"


typedef struct RTHeader {
	double		  time;
	unsigned long packetnum;
} RTHeader_t;


UDPReceive6::UDPReceive6() {
	recbuffer = new char[65000];
	recvPacketCount = 0;
}


void UDPReceive6::init(int port) {
	sock = static_cast<int>(socket(AF_INET6, SOCK_DGRAM, 0));

	memset(&addr, 0, sizeof(addr));

	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(port);
	addr.sin6_addr = in6addr_any;
	auto ret = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
	//printf("Binding port %d return %d errno %d\n", port, ret, WSAGetLastError());
	printf("Startup of receiver done\n");
}

//bool isUndefined = true;
int UDPReceive6::receive(char* buffer, int len, double* ptime) {
	struct sockaddr_in6 si_other;
	socklen_t slen = sizeof(si_other);

	RTHeader_t* pheader = (RTHeader_t*)recbuffer;
	//printf("Packet number %i\n", pheader->packetnum);

	while (true) {
		auto ret = recvfrom(sock, recbuffer, 65000, 0, (sockaddr*)&si_other, &slen);

		if (ret > sizeof RTHeader_t) {
			memcpy(buffer, recbuffer + sizeof RTHeader_t, ret - sizeof RTHeader_t);

			// this keeps track of how many packets got received
			if (receivedPackets.find(pheader->packetnum) == receivedPackets.end()) {
				receivedPackets.insert(pheader->packetnum);
				recvPacketCount += 1;
				lastPacketNum = pheader->packetnum;
			}
			return ret - sizeof RTHeader_t;
		}
	}
	return 0;
}


void UDPReceive6::closeSock() {
	closesocket(sock);
	sock = 0;
}