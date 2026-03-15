#include "videodecoder.h"

videodecoder::videodecoder(mediator* mediator) : decoder(mediator)
{
    SetLimitQueueDecoder(LIMIT_QUEUE_VIDEO_PACKET);
}

int videodecoder::GetWidth()
{
    if(m_CodecContext == nullptr)
    {
        LOGE("m_CodecContext = nullptr");
        return {};
    }
    return m_CodecContext->width;
}

int videodecoder::GetHeight()
{
    if(m_CodecContext == nullptr)
    {
        LOGE("m_CodecContext = nullptr");
        return {};
    }
    return m_CodecContext->height;
}

AVPixelFormat videodecoder::GetPixelFormat()
{
    if(m_CodecContext == nullptr)
    {
        LOGE("m_CodecContext = nullptr");
        return {};
    }
    return m_CodecContext->pix_fmt;
}