#include "AudioDecoder.h"

AudioDecoder::AudioDecoder(Mediator* Mediator) : Decoder(Mediator)
{
    SetLimitQueueDecoder(LIMIT_QUEUE_AUDIO_PACKET);
}

AVSampleFormat AudioDecoder::GetSampleFormat()
{
    if(m_CodecContext == nullptr)
    {
        return {};
    }
    return m_CodecContext->sample_fmt;
}