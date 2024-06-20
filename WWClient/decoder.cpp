/*
 * Copyright (c) 2001 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "decoder.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void Decoder::setUpDecoder() {
	outfilename = "aSingleFrame";

	pkt = av_packet_alloc();
	if (!pkt)
		exit(1);

	/* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
	memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

	/* find the MPEG-1 video decoder */
	codec = avcodec_find_decoder(AV_CODEC_ID_MPEG4);
	if (!codec) {
		fprintf(stderr, "Codec not found\n");
		exit(1);
	}

	parser = av_parser_init(codec->id);
	if (!parser) {
		fprintf(stderr, "parser not found\n");
		exit(1);
	}

	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	/* For some codecs, such as msmpeg4 and mpeg4, width and height
	   MUST be initialized there because this information is not
	   available in the bitstream. */

	   /* open it */
	if (avcodec_open2(c, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		exit(1);
	}

	// TO DO 
	// Here was file opening

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
}

void Decoder::cleanupDecoder() {
	// flush
	decode(c, frame, NULL, outfilename);

	av_parser_close(parser);
	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);
}

void Decoder::decode(AVCodecContext* dec_ctx, AVFrame* frame, AVPacket* pkt, const char* filename) {
	char buf[1024];
	int ret;

	ret = avcodec_send_packet(dec_ctx, pkt);
	if (ret < 0) {
		fprintf(stderr, "Error sending a packet for decoding\n");
		exit(1);
	}

	//printf("In Decoder::decode , ret: %i\n", ret);

	while (ret >= 0) {
		ret = avcodec_receive_frame(dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;
		else if (ret < 0) {
			fprintf(stderr, "Error during decoding\n");
			exit(1);
		}


		printf("saving frame %" PRId64 "\n", dec_ctx->frame_num);
		fflush(stdout);

		uint8_t* rgba_buffer = (uint8_t*)malloc(frame->width * frame->height * 4);

		// Converting back to RBGA ( from: endocde.cpp; Encoder::encodeFrameFromDataImage(uint8_t* dataImage))
		SwsContext* sws_ctx = sws_getContext(frame->width, frame->height, dec_ctx->pix_fmt, frame->width, frame->height, AV_PIX_FMT_RGBA, 0, NULL, NULL, NULL);
		int inLinesize[1] = { 4 * frame->width };
		sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, &rgba_buffer, inLinesize);
		sws_freeContext(sws_ctx);

		snprintf(buf, sizeof(buf), "%s-%" PRId64 ".png", filename, dec_ctx->frame_num);
		printf("saving frame %s\n", buf);
		int success = stbi_write_png(buf, frame->width, frame->height, 4,
			rgba_buffer, frame->width * 4);

		if (!success) {
			fprintf(stderr, "Error writing PNG file: %s\n", filename);
		}

		free(rgba_buffer);
	}
}

void Decoder::readParseDecode(char* buffer, int len) {
	//printf("In Decoder::readParseDecode\n");
	size_t offset{ 0 };

	do {
		//printf("Entering Do While\n");
		data_size = MIN(INBUF_SIZE, len - offset);
		memcpy(inbuf, buffer + offset, data_size);
		offset += data_size;
		eof = !data_size;
		//printf("right before decoding, eof %i\n", eof);

		data = inbuf;
		while (data_size > 0 || eof) {
			ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size, data, static_cast<int>(data_size), AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
			if (ret < 0) {
				fprintf(stderr, "Error while parsing\n");
				exit(1);
			}
			data += ret;
			data_size -= ret;

			//printf("right before decoding, data_size %i\n", data_size);
			//printf("pkt size: %i ret: %i\n", pkt->size, ret);
			if (pkt->size)
				decode(c, frame, pkt, outfilename);
			else if (eof)
				break;
		}

	} while (!eof);
}


uint8_t* Decoder::decode(AVCodecContext* dec_ctx, AVFrame* frame, AVPacket* pkt) {
	//char buf[1024];
	int ret;

	ret = avcodec_send_packet(dec_ctx, pkt);
	if (ret < 0) {
		fprintf(stderr, "Error sending a packet for decoding\n");
		exit(1);
	}

	//printf("In Decoder::decode , ret: %i\n", ret);

	while (ret >= 0) {
		ret = avcodec_receive_frame(dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;
		else if (ret < 0) {
			fprintf(stderr, "Error during decoding\n");
			exit(1);
		}

		uint8_t* rgba_buffer = (uint8_t*)malloc(frame->width * frame->height * 4);

		// Converting back to RBGA ( from: endocde.cpp; Encoder::encodeFrameFromDataImage(uint8_t* dataImage))
		SwsContext* sws_ctx = sws_getContext(frame->width, frame->height, dec_ctx->pix_fmt, frame->width, frame->height, AV_PIX_FMT_RGBA, 0, NULL, NULL, NULL);
		int inLinesize[1] = { 4 * frame->width };
		sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, &rgba_buffer, inLinesize);
		sws_freeContext(sws_ctx);

		return rgba_buffer;
	}

	return 0;
}

uint8_t* Decoder::readParseDecodeDisplay(char* buffer, int len) {
	//printf("In Decoder::readParseDecode\n");
	size_t offset{ 0 };

	do {
		//printf("Entering Do While\n");
		data_size = MIN(INBUF_SIZE, len - offset);
		memcpy(inbuf, buffer + offset, data_size);
		offset += data_size;
		eof = !data_size;
		//printf("right before decoding, eof %i\n", eof);

		data = inbuf;
		while (data_size > 0 || eof) {
			ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size, data, static_cast<int>(data_size), AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
			if (ret < 0) {
				fprintf(stderr, "Error while parsing\n");
				exit(1);
			}
			data += ret;
			data_size -= ret;

			//printf("right before decoding, data_size %i\n", data_size);
			//printf("pkt size: %i ret: %i\n", pkt->size, ret);
			if (pkt->size)
				return decode(c, frame, pkt);
			else if (eof)
				break;
		}

	} while (!eof);

	return 0;
}
