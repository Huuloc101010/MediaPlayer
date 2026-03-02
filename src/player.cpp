#include <iostream>
#include <unordered_map>
#include "player.h"
#include "videooutput.h"
#include "audiooutput.h"
#include "log.h"


const std::string player::err2str(int errnum)
{
    std::string buf(AV_ERROR_MAX_STRING_SIZE, '\0');
    av_strerror(errnum, buf.data(), buf.size());
    return buf;
}

std::string player::ts2timestr(int64_t ts, AVRational tb)
{
    std::string buf(AV_TS_MAX_STRING_SIZE, '\0');
    av_ts_make_time_string(buf.data(), ts, &tb);
    return buf;
}

bool player::InitView()
{
    bool RetVal = false;
    if(m_View)
    {
        RetVal = m_View->init();
    }
    if(RetVal == false)
    {
        return false;
    }
    return true;
}

bool player::UpdateYUVTexture(const yuv& ndata)
{
    bool RetVal = false;
    if(m_View)
    {
        RetVal = m_View->UpdateYUVTexture(ndata);
    }
    return RetVal;
}

int player::output_video_frame(UniqueFramePtr frame)
{
    if(frame == nullptr)
    {
        LOGE("frame is null");
        return -1;
    }
    if(m_VideoOutput)
    {
        m_VideoOutput->push_queue(std::move(frame));
        /* reallocate */
    }
    else
    {
        LOGE("video output is nullptr");
        return -1;
    }
    return 0;
}

int player::output_audio_frame(UniqueFramePtr frame)
{
    if(m_AudioOutput)
    {
        m_AudioOutput->push_queue(std::move(frame));
    }
    else
    {
        LOGE("m_AudioOutput is null");
    }
    return 0;
}

int player::Start(const std::string& Mediafile)
{
    m_Demuxer      = std::make_unique<demuxer>(this);
    m_VideoDecoder = std::make_unique<videodecoder>(this);
    m_AudioDecoder = std::make_unique<audiodecoder>(this);
    m_VideoOutput  = std::make_unique<videooutput>(this);
    m_AudioOutput  = std::make_unique<audiooutput>(this);
    m_Controller   = std::make_unique<controller>(this);
    m_View         = std::make_unique<view>();
    int Ret = -1;
    if(m_Demuxer != nullptr)
    {
        Ret = m_Demuxer->Play(Mediafile);
    }
    
    LOGI("Demuxing succeeded");
    return 0;
}

bool player::ConfigVideoOutput()
{
    /* allocate image where the decoded image will be put */
    if(m_VideoOutput)
    {
        int m_Width = m_VideoDecoder->GetWidth();
        int m_Height = m_VideoDecoder->GetHeight();
        m_View->Config(m_Width, m_Height);
        if(!m_VideoOutput->StartThread())
        {
            LOGE("start thread fail");
            return false;
        }
    }
    else
    {
        LOGE("m_VideoOutput = nullptr");
        return false;
    }
    return true;
}

bool player::ConfigAudioOutput()
{
    if(m_AudioOutput)
    {
        if(!m_AudioOutput->StartThread())
        {
            LOGE("start thread fail");
            return false;
        }
    }
    return true;
}

int player::decode_packet(UniquePacketPtr pkt, const bool IsFlushDecoder)
{
    if((pkt == nullptr) && (IsFlushDecoder == false))
    {
        LOGE("packet is nullptr");
        return -1;
    }
    if(m_Demuxer == nullptr)
    {
        LOGE("m_Demuxer is nullptr");
        return -1;
    }
    if(IsFlushDecoder)
    {
        /* flush the decoders */
        if(m_VideoDecoder) m_VideoDecoder->decode_packet(nullptr);
        if(m_AudioDecoder) m_AudioDecoder->decode_packet(nullptr);
        return 0;
    }
    int ret = -1;
    if((pkt->stream_index == m_Demuxer->GetVideoStreamIndex()) && (m_VideoDecoder))
    {
        ret = m_VideoDecoder->decode_packet(std::move(pkt));
    }
    else if((pkt->stream_index == m_Demuxer->GetAudioStreamIndex()) && (m_AudioDecoder))
    {
        ret = m_AudioDecoder->decode_packet(std::move(pkt));
    }
    return ret;
}

double player::GetAudioClock()
{
    double audioclock{};
    if(m_AudioOutput != nullptr)
    {
        audioclock = m_AudioOutput->get_clock();
    }
    return audioclock;
}

AVRational player::GetTimeBaseAudio()
{
    if(m_Demuxer == nullptr)
    {
        LOGE("Demuxer is nullptr");
        return {};
    }
    return m_Demuxer->GetTimeBaseAudio();
}

AVRational player::GetTimeBaseVideo()
{
    if(m_Demuxer == nullptr)
    {
        LOGE("Demuxer is nullptr");
        return {};
    }
    return m_Demuxer->GetTimeBaseVideo();
}

std::atomic<PlayerState>& player::GetCurrentState()
{
    return m_PlayerState;
}

bool player::InitVideoDecoder(const AVCodecID codecID, AVCodecParameters* codec_par)
{
    if(m_VideoDecoder && (m_VideoDecoder->init_decoder(codecID, codec_par) != 0))
    {
        LOGE("Init Video Decoder fail");
        return false;
    }
    return true;
}

bool player::InitAudioDecoder(const AVCodecID codecID, AVCodecParameters* codec_par)
{
    if(m_AudioDecoder && (m_AudioDecoder->init_decoder(codecID, codec_par) != 0))
    {
        LOGE("Init Audio Decoder fail");
        return false;
    }
    return true;
}
