#ifndef _AUDIO_DECODER_
#define _AUDIO_DECODER_
#include "Decoder.h"

class AudioDecoder : public Decoder
{
public:
    AudioDecoder(Mediator* Mediator);
    AVSampleFormat GetSampleFormat();
};

#endif /* _AUDIO_DECODER_ */