#ifndef _VIDEO_DECODER_
#define _VIDEO_DECODER_
#include "Decoder.h"

class videodecoder : public decoder
{
public:
    videodecoder(mediator* mediator);
    int GetWidth();
    int GetHeight();
    AVPixelFormat GetPixelFormat();
};

#endif /* _VIDEO_DECODER_ */