#include "decoder.h"

#include <libavcodec/avcodec.h>

ByteVector* decode(AVFormatContext* context, int channel, size_t* sample_size)
{
	ByteVector* vector = NULL;
	AVPacket* packet = av_packet_alloc();
	if (!packet)
		goto cleanup;
	int stream_index = av_find_best_stream(context, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	const AVCodec* codec = avcodec_find_decoder(context->streams[stream_index]->codecpar->codec_id);
	AVCodecContext* codec_context = avcodec_alloc_context3(codec);
	if (!codec_context)
		goto cleanup;
	codec_context->pkt_timebase = context->streams[stream_index]->time_base;
	if (avcodec_open2(codec_context, codec, NULL))
		goto cleanup;

	*sample_size = av_get_bytes_per_sample(codec_context->sample_fmt);

	AVFrame* frame = av_frame_alloc();
	if (!frame)
		goto cleanup;
	vector = alloc_vector();
	if (!vector)
		goto cleanup;
	while (av_read_frame(context, packet) >= 0)
	{
		if (packet->stream_index == stream_index)
		{
			avcodec_send_packet(codec_context, packet);
			while (!avcodec_receive_frame(codec_context, frame))
			{
				size_t bytes = *sample_size * frame->nb_samples;
				if (push_back(vector, frame->data[channel], bytes))
				{
					free_vector(&vector);
					goto cleanup;
				}
			}
		}
		av_packet_unref(packet);
	}
cleanup:
	if (codec_context)
		avcodec_free_context(&codec_context);
	if (frame)
		av_frame_free(&frame);
	if (packet)
		av_packet_free(&packet);
	return vector;
}