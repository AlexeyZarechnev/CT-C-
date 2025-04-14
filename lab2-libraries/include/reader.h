#ifndef READER
#define READER

#include <libavformat/avformat.h>

int open_input(AVFormatContext** context, const char* filename);

#endif
