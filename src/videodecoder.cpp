#include "videodecoder.h"

videodecoder::videodecoder(mediator* mediator) : decoder(mediator)
{
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