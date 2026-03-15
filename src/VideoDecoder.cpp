#include "VideoDecoder.h"

VideoDecoder::VideoDecoder(Mediator* Mediator) : Decoder(Mediator)
{
    SetLimitQueueDecoder(LIMIT_QUEUE_VIDEO_PACKET);
}

int VideoDecoder::GetWidth()
{
    if(m_CodecContext == nullptr)
    {
        LOGE("m_CodecContext = nullptr");
        return {};
    }
    return m_CodecContext->width;
}

int VideoDecoder::GetHeight()
{
    if(m_CodecContext == nullptr)
    {
        LOGE("m_CodecContext = nullptr");
        return {};
    }
    return m_CodecContext->height;
}

AVPixelFormat VideoDecoder::GetPixelFormat()
{
    if(m_CodecContext == nullptr)
    {
        LOGE("m_CodecContext = nullptr");
        return {};
    }
    return m_CodecContext->pix_fmt;
}