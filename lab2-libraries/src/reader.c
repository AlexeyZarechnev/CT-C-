#include "reader.h"

#include "../return_codes.h"

int open_input(AVFormatContext** context, const char* filename)
{
	if (*context != NULL)
	{
		fprintf(stderr, "Please free context before reusing");
		return ERROR_UNKNOWN;
	}
	if (avformat_open_input(context, filename, NULL, NULL))
	{
		fprintf(stderr, "Cannot open file %s", filename);
		return ERROR_CANNOT_OPEN_FILE;
	}
	if (avformat_find_stream_info(*context, NULL) < 0)
	{
		fprintf(stderr, "cannot read stream info");
		return ERROR_DATA_INVALID;
	}
	int stream_index = av_find_best_stream(*context, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (stream_index < 0)
	{
		fprintf(stderr, "File should have an audiostream");
		return ERROR_FORMAT_INVALID;
	}
	int codec_id = (*context)->streams[stream_index]->codecpar->codec_id;
	if (codec_id != AV_CODEC_ID_MP3 && codec_id != AV_CODEC_ID_MP2 && codec_id != AV_CODEC_ID_AAC &&
		codec_id != AV_CODEC_ID_FLAC && codec_id != AV_CODEC_ID_OPUS)
	{
		fprintf(stderr, "Invalid file format");
		return ERROR_FORMAT_INVALID;
	}
	return SUCCESS;
}