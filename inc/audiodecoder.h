#ifndef _AUDIO_DECODER_
#define _AUDIO_DECODER_
#include "decoder.h"

class audiodecoder : public decoder
{
public:
    audiodecoder(mediator* mediator);
    AVSampleFormat GetSampleFormat();
};

#endif /* _AUDIO_DECODER_ */