#include "audiodecoder.h"

audiodecoder::audiodecoder(mediator* mediator) : decoder(mediator)
{
}

AVSampleFormat audiodecoder::GetSampleFormat()
{
    if(m_CodecContext == nullptr)
    {
        return {};
    }
    return m_CodecContext->sample_fmt;
}