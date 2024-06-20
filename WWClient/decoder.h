#ifndef DECODER_H
#define DECODER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <iostream>


extern "C" {
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavformat/avformat.h"
#include "libavutil/opt.h"
#include "libavutil/timestamp.h"
}

#define INBUF_SIZE 4096

class Decoder {
public:
	const AVCodec* codec;
	AVCodecParserContext* parser;
	AVCodecContext* c = NULL;
	AVFrame* frame;
	uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
	uint8_t* data;
	size_t   data_size;
	int ret;
	int eof;
	AVPacket* pkt;

	// outfile name of the png files, maybe not used later
	const char* outfilename;

	Decoder() {
		setUpDecoder();
	};
	~Decoder() {
		cleanupDecoder();
	};

	void readParseDecode(char* buffer, int len);
	uint8_t* readParseDecodeDisplay(char* buffer, int len);

private:
	void setUpDecoder();
	void decode(AVCodecContext* dec_ctx, AVFrame* frame, AVPacket* pkt, const char* filename);
	uint8_t* decode(AVCodecContext* dec_ctx, AVFrame* frame, AVPacket* pkt);
	void cleanupDecoder();
};


#endif
