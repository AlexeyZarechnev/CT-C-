#ifndef ENCODER
#define ENCODER

#include "vector.h"
#include <libavformat/avformat.h>

ByteVector* resample(AVFormatContext* context, int channel, size_t* sample_size, int sample_rate);

#endif
