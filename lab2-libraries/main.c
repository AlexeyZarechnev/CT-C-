#include "cross_correlation.h"
#include "decoder.h"
#include "encoder.h"
#include "reader.h"
#include "return_codes.h"
#include "vector.h"

#include <stdio.h>

#define my_max(a, b) a > b ? a : b

typedef struct Result
{
	int samples_delta;
	int sample_rate;
	int time_delta;
} Result;

int main(int argc, char** argv)
{
	int return_code = 0;
	if (argc != 2 && argc != 3)
	{
		fprintf(stderr, "expected <file> or <file1> <file2>");
		return ERROR_ARGUMENTS_INVALID;
	}
	AVFormatContext* context1 = NULL;
	av_log_set_level(AV_LOG_QUIET);
	if ((return_code = open_input(&context1, argv[1])))
	{
		return return_code;
	}
	Result result = { 0, 0, 0 };
	size_t sample_size2;
	size_t sample_size1;
	ByteVector* data2;
	ByteVector* data1;
	int stream_index1 = av_find_best_stream(context1, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	result.sample_rate = context1->streams[stream_index1]->codecpar->sample_rate;
	if (argc > 2)
	{
		AVFormatContext* context2 = NULL;
		if ((return_code = open_input(&context2, argv[2])))
		{
			return return_code;
		}
		int stream_index2 = av_find_best_stream(context2, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
		int sample_rate2 = context2->streams[stream_index2]->codecpar->sample_rate;
		if (result.sample_rate != sample_rate2)
		{
			result.sample_rate = my_max(result.sample_rate, sample_rate2);
			if (result.sample_rate == sample_rate2)
			{
				data1 = resample(context1, 0, &sample_size1, result.sample_rate);
				data2 = decode(context2, 0, &sample_size2);
			}
			else
			{
				data1 = decode(context1, 0, &sample_size1);
				data2 = resample(context2, 0, &sample_size2, result.sample_rate);
			}
		}
		else
		{
			data1 = decode(context1, 0, &sample_size1);
			data2 = decode(context2, 0, &sample_size2);
		}
		avformat_close_input(&context2);
	}
	else
	{
		if (context1->streams[stream_index1]->codecpar->ch_layout.nb_channels != 2)
		{
			fprintf(stderr, "expected 2 channels, found %i", context1->streams[stream_index1]->codecpar->ch_layout.nb_channels);
			return ERROR_FORMAT_INVALID;
		}
		data1 = decode(context1, 0, &sample_size1);
		avformat_close_input(&context1);
		open_input(&context1, argv[1]);
		data2 = decode(context1, 1, &sample_size2);
	}
	avformat_close_input(&context1);
	if (!data1 || !data2)
	{
		fprintf(stderr, "Not enough memory for this files");
		return ERROR_NOTENOUGH_MEMORY;
	}
	result.samples_delta = cc_time_count(data1, sample_size1, data2, sample_size2);
	result.time_delta = result.samples_delta * 1000 / result.sample_rate;
	printf("delta: %i samples\nsample rate: %i Hz\ndelta time: %i ms\n",
		   result.samples_delta,
		   result.sample_rate,
		   result.time_delta);

	return SUCCESS;
}
