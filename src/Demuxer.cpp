#include <thread>
#include "Demuxer.h"
#include "Define.h"
#include "Mediator.h"

Demuxer::Demuxer(Mediator* Mediator)
{
    m_Mediator = Mediator;
}

int Demuxer::StartPlay(const std::string& Mediafile)
{
    int ret = 0;
    /* open input file, and allocate format context */
    AVFormatContext* FormatContext = nullptr;
    if (avformat_open_input(&FormatContext, Mediafile.c_str(), NULL, NULL) < 0)
    {
        LOGE("Could not open source file {}", Mediafile);
        return -1;
    }
    m_FormatContext.reset(FormatContext);
    if((m_FormatContext == nullptr) || (m_Mediator == nullptr))
    {
        LOGE("m_FormatContext or m_Mediator is nullptr");
        return -1;
    }
    /* retrieve stream information */
    if (avformat_find_stream_info(m_FormatContext.get(), NULL) < 0)
    {
        LOGE("Could not find stream information");
        return -1;
    }
 
    if(open_codec_context(&m_VideoStreamIndex, m_FormatContext.get(), AVMEDIA_TYPE_VIDEO) >= 0)
    {
        m_VideoStream = m_FormatContext->streams[m_VideoStreamIndex];
        
        // Create windows
        if(m_Mediator->ConfigVideoOutput() == false)
        {
            return -1;
        }
    }
    
    if (open_codec_context(&m_AudioStreamIndex, m_FormatContext.get(), AVMEDIA_TYPE_AUDIO) >= 0)
    {
        if(m_Mediator->ConfigAudioOutput() == false)
        {
            LOGE("config audio output fail");
            return -1;
        }
        m_AudioStream = m_FormatContext->streams[m_AudioStreamIndex];
    }
 
    /* dump input information to stderr */
    av_dump_format(m_FormatContext.get(), 0, Mediafile.c_str(), 0);
 
    if (!m_AudioStream && !m_VideoStream)
    {
        LOGE("Could not find audio or video stream in the input, aborting");
        return 1;
    }
 
    m_ThreadReadFrame = std::jthread(&Demuxer::loop_read_frame, this);
    LOGI("Demuxing succeeded");
    return ret;
}

void Demuxer::loop_read_frame()
{
    // first allocate
    UniquePacketPtr Packet(av_packet_alloc());
    int ret = 0;
    /* read frames from the file */
    if(m_FormatContext == nullptr)
    {
        LOGE("FormatContext is null");
        return;
    }
    while (av_read_frame(m_FormatContext.get(), Packet.get()) >= 0)
    {
        if(CheckStateExit())
        {
            return;
        }
        CheckStateSleep();

        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
        if((Packet == nullptr) || (m_Mediator == nullptr))
        {
            LOGE("Packet or m_Mediator is null");
            continue;
        }
        ret = m_Mediator->decode_packet(std::move(Packet));
        if(ret != 0)
        {
            LOGE("decode fail");
            break;
        }
        // realocate
        Packet.reset(av_packet_alloc());
    }
 
    /* flush the decoders */
    if(m_Mediator->decode_packet(nullptr, true) != 0)
    {
        LOGE("Flush decoder fail");
    }
}

void Demuxer::Exit()
{
    ControlFunction::Exit();
    if(m_ThreadReadFrame.joinable())
    {
        m_ThreadReadFrame.join();
    }
}

int Demuxer::open_codec_context(int *stream_idx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    if((stream_idx == nullptr) || (fmt_ctx == nullptr))
    {
        LOGE("stream_idx or fmt_ctx is nullptr");
        return -1;
    }
    int ret, stream_index;
    AVStream *st;
    //const AVCodec *dec = NULL;
 
    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0)
    {
        LOGE("Could not find {} stream in input file ", av_get_media_type_string(type)/*, SourceFileName*/);
        return ret;
    }
    else
    {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];
        if(type == AVMEDIA_TYPE_VIDEO)
        {
            if(m_Mediator->InitVideoDecoder(st->codecpar->codec_id, st->codecpar) == true)
            {
                LOGI("Init video decoder success");
            }
            else
            {
                LOGE("Init video decoder fail");
            } 
        }
        if(type == AVMEDIA_TYPE_AUDIO)
        {
            if(m_Mediator->InitAudioDecoder(st->codecpar->codec_id, st->codecpar) == true)
            {
                LOGI("Init audio decoder success");
            }
            else
            {
                LOGE("Init audio decoder fail");
            }
        }
        *stream_idx = stream_index;
    }
 
    return 0;
}

AVRational Demuxer::GetTimeBaseVideo()
{
    if(m_VideoStream == nullptr)
    {
        LOGE("m_VideoStream = nullptr");
        return {};
    }
    return m_VideoStream->time_base;
}

AVRational Demuxer::GetTimeBaseAudio()
{
    if(m_AudioStream == nullptr)
    {
        LOGE("m_AudioStream = nullptr");
        return {};
    }
    return m_AudioStream->time_base;
}

int Demuxer::GetVideoStreamIndex()
{
    return m_VideoStreamIndex;
}

int Demuxer::GetAudioStreamIndex()
{
    return m_AudioStreamIndex;
}
