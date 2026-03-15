#ifndef _VIDEO_DECODER_
#define _VIDEO_DECODER_
#include "Decoder.h"

class VideoDecoder : public Decoder
{
public:
    VideoDecoder(Mediator* Mediator);
    int GetWidth();
    int GetHeight();
    AVPixelFormat GetPixelFormat();
};

#endif /* _VIDEO_DECODER_ */