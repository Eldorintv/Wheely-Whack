#ifndef ENCODE_H
#define ENCODE_H

#include "WWInclude.h"


void encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt, FILE* outfile);


class Encoder {

public:
	const char* filename{ "../../media/screenshots/task05.ES" }, * codec_name{ "mpeg4" };
	const AVCodec* codec;
	AVCodecContext* c = NULL;
	int i, ret, x, y;
	FILE* f;
	AVFrame* frame;
	AVPacket* pkt;
	bool encodeFrame{ false };
	bool codecReady{ false };
	int frameTimestamp{ 0 };
	uint8_t endcode[4] = { 0, 0, 1, 0xb7 };


	Encoder() {};
	~Encoder() {};

	bool setUpCodec(uint32_t extentWidth, uint32_t extentHeight);

	void encodeFrameFromDataImage(uint8_t* dataImage);

	void encoderFinishProcess();
};



#endif