#include "UDP6receive.h"
#include "UDP6send.h"
#include <cstdio>

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
#include "decoder.h"
#include <queue>
#include <mutex>
#include <condition_variable>

#include "videoDisplay.h"


constexpr int RECV_PORT_NUMBER{ 50'000 };
constexpr int SEND_PORT_NUMBER{ 51'337 };

typedef struct RTHeader {
	double		  time;
	unsigned long packetnum;
} RTHeader_t;

constexpr int MAX_PACKET_SIZE{ 250'000 };
std::queue<std::vector<char>> frameQueue;
std::mutex queueMutex;
std::condition_variable queueCondition;

bool isFirstReport{ true };
size_t bytesReceivedLast10Seconds{ 0 };
auto last = std::chrono::system_clock::now();
int framesReceived{ 0 };


static void sendMessage(char byteRate[], UDPSend6& sender) {


	std::cout << "Sending receiver report\n";
	//char buf[100] = "My Receiver sends something back!\n";
	sender.send(byteRate, static_cast<int>(strlen(byteRate)));

	//std::this_thread::sleep_for(std::chrono::seconds(5));

}

const std::chrono::seconds updateTimer(5);

static void sendReceiverReport(int len, UDPReceive6& receiver, UDPSend6& sender) {
	bytesReceivedLast10Seconds += len;
	framesReceived += 1;

	if (isFirstReport) {
		last = std::chrono::system_clock::now();
		isFirstReport = false;
	}
	auto now = std::chrono::system_clock::now();

	if (now - last >= updateTimer) {
		auto duration = now - last;
		auto elapsedSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
		last = now;

		char msg[500];

		size_t byteRate = static_cast<size_t>(bytesReceivedLast10Seconds / elapsedSeconds);
		bytesReceivedLast10Seconds = 0;

		double fps = static_cast<double>(framesReceived / elapsedSeconds);
		framesReceived = 0;

		double lostPackets = receiver.lastPacketNum - receiver.recvPacketCount;
		double packetLossRate = lostPackets / receiver.lastPacketNum;
		// to make last 10 seconds rather than overall
		receiver.recvPacketCount = receiver.lastPacketNum;

		sprintf_s(msg, "<< ! >>\ncurrent byte rate: %zu\ncurrent fps: %.2f\npacket loss rate: %.2f\n<< ! >>\n", byteRate, fps, packetLossRate);

		sendMessage(msg, sender);

	}
}

void receiveFrames(UDPReceive6& receiver, UDPSend6& sender) {

	char buf[65'000];
	// might need to increase, moved to heap and using memcpy_s
	char* tempBuffer = (char*)malloc(MAX_PACKET_SIZE * sizeof(char));
	if (tempBuffer == NULL) {
		fprintf(stderr, "Could not allocate tempBuffer\n");
	} _Analysis_assume_(tempBuffer != NULL);

	//int packetCounter{ 1 };

	while (true) {
		double ptime;
		auto ret = receiver.receive(buf, sizeof buf, &ptime);
		RTHeader_t* pheader = (RTHeader_t*)buf;
		int len{ ret };
		if (ret == 60000) {

			memcpy_s(tempBuffer, MAX_PACKET_SIZE, buf, ret);

			while (ret == 60000) {
				ret = receiver.receive(buf, sizeof buf, &ptime);
				memcpy_s(tempBuffer + len, static_cast<size_t>(MAX_PACKET_SIZE) - len, buf, ret);
				len += ret;
			}
			//printf("Packet % i recevied, size: % d \n", packetCounter++, len);
			//decoder->readParseDecode(tempBuffer, len);
			sendReceiverReport(len, receiver, sender);

			std::vector<char> bufferForQueue(len);
			std::copy(tempBuffer, tempBuffer + len, bufferForQueue.begin());

			{
				std::lock_guard<std::mutex> lock(queueMutex);
				frameQueue.push(std::move(bufferForQueue));
			}
			queueCondition.notify_one();

		}
		else {
			//printf("Packet % i recevied, size: % d \n", packetCounter++, ret);
			//decoder->readParseDecode(buf, ret);
			sendReceiverReport(ret, receiver, sender);

			std::vector<char> bufferForQueue(ret);
			std::copy(buf, buf + ret, bufferForQueue.begin());

			{
				std::lock_guard<std::mutex> lock(queueMutex);
				frameQueue.push(std::move(bufferForQueue));
			}
			queueCondition.notify_one();
		}
	}
	free(tempBuffer);
}

static void queueToPNG() {
	Decoder* decoder = new Decoder();
	int pauseBetweenQueuePopInMiliseconds{ 1000 };

	while (true) {
		std::vector<char> frame;

		{
			std::unique_lock<std::mutex> lock(queueMutex);
			queueCondition.wait(lock, [] { return !frameQueue.empty(); });

			frame = frameQueue.front();
			frameQueue.pop();
		}

		if (!frame.empty()) {
			//std::cout << "Here, strlen frame: " << frame.size() << "\n";
			decoder->readParseDecode(frame.data(), static_cast<int>(frame.size()));
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(pauseBetweenQueuePopInMiliseconds));
	}

	// delete decoder on heap
	delete decoder;
}

void queueToUINT8() {
	Decoder* decoder = new Decoder();
	int pauseBetweenQueuePopInMiliseconds{ 20 };

	while (true) {
		std::vector<char> frame;

		{
			std::unique_lock<std::mutex> lock(queueMutex);
			queueCondition.wait(lock, [] { return !frameQueue.empty(); });

			frame = frameQueue.front();
			frameQueue.pop();
			std::cout << frameQueue.size() << "\n";
		}

		if (!frame.empty()) {
			//std::cout << "Here, strlen frame: " << frame.size() << "\n";
			auto pixels = decoder->readParseDecodeDisplay(frame.data(), static_cast<int>(frame.size()));
			updateTexture(pixels);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(pauseBetweenQueuePopInMiliseconds));
	}

	// delete decoder on heap
	delete decoder;
}


int main() {

	startWinsock();

	UDPReceive6 receiver;
	receiver.init(RECV_PORT_NUMBER);

	UDPSend6 sender;
	sender.init("::1", SEND_PORT_NUMBER);

	UDPSend6 senderMovement;
	senderMovement.init("::1", SEND_PORT_NUMBER + 1);


	// Threads A -> Receiver, Thread B -> Presenter
	std::thread A(receiveFrames, std::ref(receiver), std::ref(sender));
	//std::thread B(queueToPNG);
	std::thread B(queueToUINT8);
	std::thread C(displayWindow, std::ref(senderMovement));

	A.join();
	B.join();
	C.join();

	WSACleanup();
}