#include "UDP6Send.h"

extern "C" {
#include <stdio.h>
#include <time.h>
}


typedef struct RTHeader {
	double		  time;
	unsigned long packetnum;
} RTHeader_t;


int startWinsock(void)
{
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2, 0), &wsa);
}


void UDPSend6::init(const char* address, int port) {
	sock = static_cast<int>(socket(AF_INET6, SOCK_DGRAM, 0));

	struct addrinfo hints;

	memset(&addr, 0, sizeof(addr));
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;

	struct addrinfo* result = NULL;
	auto dwRetval = getaddrinfo(address, nullptr, &hints, &result);
	if (dwRetval != 0) {
		printf("getaddrinfo failed with error: %d\n", dwRetval);
		return;
	}
	for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		if (ptr->ai_family == AF_INET6) {
			memcpy(&addr, ptr->ai_addr, ptr->ai_addrlen);
			addr.sin6_port = htons(port);
			addr.sin6_family = AF_INET6;
		}
	}
	freeaddrinfo(result);
}


const uint16_t MAX_PACKET_SIZE{ 60'000 };

int UDPSend6::send(char* buffer, int len) {
	char sendbuffer[65000];

	packetnum++;
	RTHeader_t header;
	header.time = clock() / (double)CLOCKS_PER_SEC;
	header.packetnum = packetnum;
	//header.packetCount = 1;

	int ret{ 0 };
	size_t bytesSent{ 0 };

	while (bytesSent < len) {
		int packetLength = (len - bytesSent) < MAX_PACKET_SIZE ? (len - bytesSent) : MAX_PACKET_SIZE;

		memcpy(sendbuffer, &header, sizeof(header));
		memcpy(sendbuffer + sizeof(header), buffer + bytesSent, packetLength);

		ret = sendto(sock, sendbuffer, sizeof(header) + packetLength, 0, (const struct sockaddr*)&addr, sizeof(addr));

		bytesSent += packetLength;
	}

	return ret;
}

void UDPSend6::closeSock() {
	closesocket(sock);
	sock = 0;
}