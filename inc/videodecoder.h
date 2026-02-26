#ifndef _VIDEO_DECODER_
#define _VIDEO_DECODER_
#include "decoder.h"

class videodecoder : public decoder
{
public:
    videodecoder(mediator* mediator);
    const int GetWidth();
    const int GetHeight();
    const AVPixelFormat GetPixelFormat();
};

#endif /* _VIDEO_DECODER_ */