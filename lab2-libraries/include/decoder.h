#ifndef DECODER
#define DECODER

#include "vector.h"
#include <libavformat/avformat.h>

ByteVector* decode(AVFormatContext* context, int channel, size_t* sample_size);

#endif