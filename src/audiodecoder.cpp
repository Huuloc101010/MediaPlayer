#include "audiodecoder.h"

audiodecoder::audiodecoder(mediator* mediator) : decoder(mediator)
{
    SetLimitQueueDecoder(LIMIT_QUEUE_AUDIO_PACKET);
}

AVSampleFormat audiodecoder::GetSampleFormat()
{
    if(m_CodecContext == nullptr)
    {
        return {};
    }
    return m_CodecContext->sample_fmt;
}