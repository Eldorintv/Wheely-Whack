#include "WWencoder.h"


UDPSend6 sender;
//UDPReceive6 receiver;
SwsContext* ctx = nullptr;

// size of the chunks that get cut out of the ffmpeg output
constexpr size_t MAX_PACKET_SIZE = 1400;

bool Encoder::setUpCodec(uint32_t extentWidth, uint32_t extentHeight) {
	/* find the mpeg1video encoder */
	codec = avcodec_find_encoder_by_name(codec_name);
	if (!codec) {
		fprintf(stderr, "Codec '%s' not found\n", codec_name);
		exit(1);
	}

	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	pkt = av_packet_alloc();
	if (!pkt)
		exit(1);

	/* put sample parameters */
	c->bit_rate = 100000;
	/* resolution must be a multiple of two */
	c->width = extentWidth;
	c->height = extentHeight;
	/* frames per second */
	c->time_base = { 1, 25 };
	c->framerate = { 25, 1 };

	/* emit one intra frame every ten frames
	 * check frame pict_type before passing frame
	 * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	 * then gop_size is ignored and the output of encoder
	 * will always be I frame irrespective to gop_size
	 */
	c->gop_size = 4;
	c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec->id == AV_CODEC_ID_H264)
		av_opt_set(c->priv_data, "preset", "faster", 0);

	/* open it */
	ret = avcodec_open2(c, codec, NULL);
	if (ret < 0) {
		//std::cout << "Could not open codec:" << av_err2str(ret) << "\n";
		//fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
		exit(1);
	}

	//f = fopen(filename, "wb");
	//if (!f) {
	//	fprintf(stderr, "Could not open %s\n", filename);
	//	exit(1);
	//}
	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	frame->format = AV_PIX_FMT_YUV420P;
	frame->width = c->width;
	frame->height = c->height;

	ret = av_frame_get_buffer(frame, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate the video frame data\n");
		exit(1);
	}

	ret = av_frame_make_writable(frame);
	if (ret < 0)
		exit(1);

	sender.init("::1", 50000);
	//receiver.init(51337);

	ctx = sws_getContext(c->width, c->height, AV_PIX_FMT_BGRA, c->width, c->height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, 0, 0, 0);

	return true;
}

void encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt,
	FILE* outfile)
{
	int ret;

	/* send the frame to the encoder */
	if (frame) {
		//std::cout << "Send frame " << frame->pts << "\n";
	}
	//frame->pts;

	//printf("Send frame %3"PRId64"\n", frame->pts);

	ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0) {
		fprintf(stderr, "Error sending a frame for encoding\n");
		exit(1);
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(enc_ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error during encoding\n");
			exit(1);
		}


		std::cout << "Sending with UDP: packet " << pkt->pts << " size:" << pkt->size << "\n";
		sender.send((char*)pkt->data, pkt->size);


		av_packet_unref(pkt);
	}
}

void Encoder::encodeFrameFromDataImage(uint8_t* dataImage) {

	int inLinesize[1] = { 4 * c->width };
	sws_scale(ctx, &dataImage, inLinesize, 0, c->height,
		frame->data, frame->linesize);

	frame->pts = frameTimestamp++;

	encode(c, frame, pkt, f);
}

void Encoder::encoderFinishProcess() {

	/* flush the encoder */
	encode(c, NULL, pkt, f);

	/* Add sequence end code to have a real MPEG file.
	   It makes only sense because this tiny examples writes packets
	   directly. This is called "elementary stream" and only works for some
	   codecs. To create a valid file, you usually need to write packets
	   into a proper file format or protocol; see muxing.c.
	 */
	 /*if (codec->id == AV_CODEC_ID_MPEG1VIDEO || codec->id == AV_CODEC_ID_MPEG2VIDEO)
		 fwrite(endcode, 1, sizeof(endcode), f);
	 fclose(f);*/

	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);

	sws_freeContext(ctx);

	// UDP Send socket clean up
	//WSACleanup();

	std::cout << " done!\n";
}
