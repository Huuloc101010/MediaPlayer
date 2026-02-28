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
int player::output_video_frame()
{
    if(m_Frame == nullptr)
    {
        LOGE("m_Frame is null");
        return -1;
    }
    if(m_VideoOutput)
    {
        m_VideoOutput->push_queue(std::move(m_Frame));
        /* reallocate */
        m_Frame.reset(av_frame_alloc());
    }
    else
    {
        LOGE("video output is nullptr");
        return -1;
    }
    return 0;
}

int player::output_audio_frame()
{
    if(m_AudioOutput)
    {
        m_AudioOutput->push_queue(std::move(m_Frame));
        m_Frame.reset(av_frame_alloc());
    }
    else
    {
        LOGE("m_AudioOutput is null");
    }
    return 0;
}



int player::open_codec_context(int *stream_idx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    //const AVCodec *dec = NULL;
 
    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0)
    {
        LOGE("Could not find {} stream in input file {}", av_get_media_type_string(type), m_SourceFileName);
        return ret;
    }
    else
    {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];
        if(type == AVMEDIA_TYPE_VIDEO)
        {
            if(m_VideoDecoder->init_decoder(st->codecpar->codec_id, st->codecpar) == 0)
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
            if(m_AudioDecoder->init_decoder(st->codecpar->codec_id, st->codecpar) == 0)
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

int player::run(int argc, char **argv)
{
    m_VideoDecoder = std::make_unique<videodecoder>(this);
    m_AudioDecoder = std::make_unique<audiodecoder>(this);
    int ret = 0;
    if (argc != 2)
    {
        LOGE("In valid parameter: usage {} video.mp4", argv[0]);
        exit(1);
    }
    m_SourceFileName = argv[1];
    /* open input file, and allocate format context */
    AVFormatContext* FormatContext = nullptr;
    if (avformat_open_input(&FormatContext, m_SourceFileName.c_str(), NULL, NULL) < 0)
    {
        LOGE("Could not open source file {}", m_SourceFileName);
        exit(1);
    }
    m_FormatContext.reset(FormatContext);
    /* retrieve stream information */
    if (avformat_find_stream_info(m_FormatContext.get(), NULL) < 0)
    {
        LOGE("Could not find stream information");
        exit(1);
    }
 
    if(open_codec_context(&m_VideoStreamIndex, m_FormatContext.get(), AVMEDIA_TYPE_VIDEO) >= 0)
    {
        m_VideoStream = m_FormatContext->streams[m_VideoStreamIndex];
        
        
        /* allocate image where the decoded image will be put */
        if(m_VideoDecoder)
        {
            m_Width = m_VideoDecoder->GetWidth();
            m_Height = m_VideoDecoder->GetHeight();
        }

        // Create windows
        m_VideoOutput = std::make_unique<videooutput>(m_Width, m_Height, this);
        if(m_VideoOutput)
        {
            if(!m_VideoOutput->StartThread())
            {
                LOGE("start thread fail");
                return -1;
            }
        }
        if (ret < 0)
        {
            LOGE("Could not allocate raw video buffer");
            return -1;
        }
        m_VideoDtsBuffSize = ret;
    }
    
    if (open_codec_context(&m_AudioStreamIndex, m_FormatContext.get(), AVMEDIA_TYPE_AUDIO) >= 0)
    {
        m_AudioOutput = std::make_unique<audiooutput>(this);
        if(m_AudioOutput)
        {
            if(!m_AudioOutput->StartThread())
            {
                LOGE("start thread fail");
                return -1;
            }
        }
        m_AudioStream = m_FormatContext->streams[m_AudioStreamIndex];
    }
 
    /* dump input information to stderr */
    av_dump_format(m_FormatContext.get(), 0, m_SourceFileName.c_str(), 0);
 
    if (!m_AudioStream && !m_VideoStream)
    {
        LOGE("Could not find audio or video stream in the input, aborting");
        return 1;
    }
    m_Frame.reset(av_frame_alloc());
    if(!m_Frame)
    {
        LOGE("Could not allocate frame");
        ret = AVERROR(ENOMEM);
        return -1;
    }
 
    loop_read_frame();
    LOGI("Demuxing succeeded");
    while(true);
    return ret;
}

void player::loop_read_frame()
{
    // first allocate
    UniquePacketPtr Packet(av_packet_alloc());
    int ret = 0;
    /* read frames from the file */
    while (av_read_frame(m_FormatContext.get(), Packet.get()) >= 0)
    {
        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
        if(Packet == nullptr)
        {
            
        }
        ret = decode_packet(std::move(Packet), m_Frame);
        if(ret != 0)
        {
            LOGE("decode fail");
            break;
        }
        // realocate
        Packet.reset(av_packet_alloc());
    }
 
    /* flush the decoders */
    if(decode_packet(nullptr, m_Frame, true) != 0)
    {
        LOGE("Flush decoder fail");
    }
}

int player::decode_packet(UniquePacketPtr pkt, UniqueFramePtr& frame, const bool IsFlushDecoder)
{
    if(IsFlushDecoder)
    {
        /* flush the decoders */
        if(m_VideoDecoder) m_VideoDecoder->decode_packet(nullptr, frame);
        if(m_AudioDecoder) m_AudioDecoder->decode_packet(nullptr, frame);
        return 0;
    }
    int ret = -1;
    if((pkt->stream_index == m_VideoStreamIndex) && (m_VideoDecoder))
    {
        ret = m_VideoDecoder->decode_packet(std::move(pkt), frame);
    }
    else if((pkt->stream_index == m_AudioStreamIndex) && (m_AudioDecoder))
    {
        ret = m_AudioDecoder->decode_packet(std::move(pkt), frame);
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
    return m_AudioStream->time_base;
}

AVRational player::GetTimeBaseVideo()
{
    return m_VideoStream->time_base;
}

std::atomic<PlayerState>& player::GetCurrentState()
{
    return m_PlayerState;
}