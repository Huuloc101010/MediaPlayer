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


int player::get_format_from_sample_fmt(const char **fmt,
                                      enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry
    {
        enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    };
    sample_fmt_entry sample_fmt_entries[] = {
        { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };
    *fmt = NULL;
 
    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++)
    {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }
 
    LOGE("sample format {} is not supported as output format", av_get_sample_fmt_name(sample_fmt));
    return -1;
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
    if (avformat_open_input(&m_FormatContext, m_SourceFileName.c_str(), NULL, NULL) < 0)
    {
        LOGE("Could not open source file {}", m_SourceFileName);
        exit(1);
    }
 
    /* retrieve stream information */
    if (avformat_find_stream_info(m_FormatContext, NULL) < 0)
    {
        LOGE("Could not find stream information");
        exit(1);
    }
 
    if(open_codec_context(&m_VideoStreamIndex, m_FormatContext, AVMEDIA_TYPE_VIDEO) >= 0)
    {
        m_VideoStream = m_FormatContext->streams[m_VideoStreamIndex];
        
        
        /* allocate image where the decoded image will be put */
        if(m_VideoDecoder)
        {
            m_Width = m_VideoDecoder->GetWidth();
            m_Height = m_VideoDecoder->GetHeight();
            m_PixelFormat = m_VideoDecoder->GetPixelFormat();
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
        ret = av_image_alloc(m_VideoDtsData, m_VideoDtsLineSize, m_Width, m_Height, m_PixelFormat, 1);
        if (ret < 0)
        {
            LOGE("Could not allocate raw video buffer");
            clean_resource();
            return -1;
        }
        m_VideoDtsBuffSize = ret;
    }
    
    if (open_codec_context(&m_AudioStreamIndex, m_FormatContext, AVMEDIA_TYPE_AUDIO) >= 0)
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
    av_dump_format(m_FormatContext, 0, m_SourceFileName.c_str(), 0);
 
    if (!m_AudioStream && !m_VideoStream)
    {
        LOGE("Could not find audio or video stream in the input, aborting");
        ret = 1;
        clean_resource();
        return 1;
    }
    m_Frame.reset(av_frame_alloc());
    if(!m_Frame)
    {
        LOGE("Could not allocate frame");
        ret = AVERROR(ENOMEM);
        clean_resource();
        return -1;
    }
 
    m_Packet.reset(av_packet_alloc());
    if(!m_Packet)
    {
        LOGE("Could not allocate packet");
        ret = AVERROR(ENOMEM);
        clean_resource();
        return -1;
    }
    loop_read_frame();
    LOGI("Demuxing succeeded");
    while(true);
    if (m_VideoStream)
    {
        LOGI("Play the output video file with the command:"
               "ffplay -f rawvideo -pix_fmt {} -video_size {}x{}",
               av_get_pix_fmt_name(m_PixelFormat), m_Width, m_Height);
    }
    enum AVSampleFormat sfmt = AV_SAMPLE_FMT_NONE;
    if (m_AudioStream)
    {
        if(m_AudioDecoder)
        {
            sfmt = m_AudioDecoder->GetSampleFormat();
        }
        int n_channels = /*m_AudioDecodeContext->ch_layout.nb_channels;*/ 1;
       
// int n_channels = m_AudioDecodeContext->ch_layout.nb_channels > 0
//                          ? m_AudioDecodeContext->ch_layout.nb_channels
//                          : m_AudioDecodeContext->channels;
 
        const char *fmt;
 
        if (av_sample_fmt_is_planar(sfmt))
        {
            const char *packed = av_get_sample_fmt_name(sfmt);
            LOGW("Warning: the sample format the decoder produced is planar "
                   "({}). This example will output the first channel only.",
                   packed ? packed : "?");
            sfmt = av_get_packed_sample_fmt(sfmt);
            n_channels = 1;
        }
 
        if((ret = get_format_from_sample_fmt(&fmt, sfmt)) < 0)
        {
            clean_resource();
        }
    }
 
    return ret < 0;
}

void player::loop_read_frame()
{
    int ret = 0;
    /* read frames from the file */
    while (av_read_frame(m_FormatContext, m_Packet.get()) >= 0)
    {
        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
        if(m_Packet->stream_index == m_VideoStreamIndex)
        {
            ret = m_VideoDecoder->decode_packet(std::move(m_Packet), m_Frame);
        }
        else if(m_Packet->stream_index == m_AudioStreamIndex)
        {
            ret = m_AudioDecoder->decode_packet(std::move(m_Packet), m_Frame);
        }

        // realocate
        m_Packet.reset(av_packet_alloc());

        if(ret < 0) break;
    }
 
    /* flush the decoders */
    if(m_VideoDecoder)
    {
        m_VideoDecoder->decode_packet(nullptr, m_Frame);
    }
    if(m_AudioDecoder)
    {
        m_AudioDecoder->decode_packet(nullptr, m_Frame);
    }
}

void player::clean_resource()
{
    avformat_close_input(&m_FormatContext);
    av_free(m_VideoDtsData[0]);
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