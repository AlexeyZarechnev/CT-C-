#include "encoder.h"

#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>

ByteVector* resample(AVFormatContext* context, int channel, size_t* sample_size, int sample_rate)
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

	SwrContext* swr_context = NULL;
	if (swr_alloc_set_opts2(
			&swr_context,
			&context->streams[stream_index]->codecpar->ch_layout,
			codec_context->sample_fmt,
			sample_rate,
			&context->streams[stream_index]->codecpar->ch_layout,
			codec_context->sample_fmt,
			context->streams[stream_index]->codecpar->sample_rate,
			0,
			NULL))
		goto cleanup;
	if (swr_init(swr_context))
		goto cleanup;

	AVFrame* input_frame = av_frame_alloc();
	if (!input_frame)
		goto cleanup;
	AVFrame* output_frame = av_frame_alloc();
	if (!output_frame)
		goto cleanup;
	output_frame->format = codec_context->sample_fmt;
	output_frame->sample_rate = sample_rate;
	output_frame->ch_layout = context->streams[stream_index]->codecpar->ch_layout;
	if (av_samples_alloc(output_frame->data, NULL, output_frame->ch_layout.nb_channels, 20000, output_frame->format, 0) < 0)
		goto cleanup;
	vector = alloc_vector();
	if (!vector)
		goto cleanup;
	while (av_read_frame(context, packet) >= 0)
	{
		if (packet->stream_index == stream_index)
		{
			avcodec_send_packet(codec_context, packet);
			while (!avcodec_receive_frame(codec_context, input_frame))
			{
				swr_convert_frame(swr_context, output_frame, input_frame);
				size_t output_size = *sample_size * output_frame->nb_samples;
				if (push_back(vector, output_frame->data[channel], output_size))
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
	if (swr_context)
		swr_free(&swr_context);
	if (input_frame)
		av_frame_free(&input_frame);
	if (output_frame)
		av_frame_free(&output_frame);
	if (packet)
		av_packet_free(&packet);

	return vector;
}
