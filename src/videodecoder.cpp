#include "videodecoder.h"

videodecoder::videodecoder(mediator* mediator) : decoder(mediator)
{
    SetLimitQueueDecoder(LIMIT_QUEUE_VIDEO_PACKET);
}

const int videodecoder::GetWidth()
{
    if(m_CodecContext == nullptr)
    {
        LOGE("m_CodecContext = nullptr");
        return {};
    }
    return m_CodecContext->width;
}

const int videodecoder::GetHeight()
{
    if(m_CodecContext == nullptr)
    {
        LOGE("m_CodecContext = nullptr");
        return {};
    }
    return m_CodecContext->height;
}

const AVPixelFormat videodecoder::GetPixelFormat()
{
    if(m_CodecContext == nullptr)
    {
        LOGE("m_CodecContext = nullptr");
        return {};
    }
    return m_CodecContext->pix_fmt;
}