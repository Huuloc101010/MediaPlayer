#include <iostream>
#include <unordered_map>
#include "player.h"
#include "videooutput.h"
#include "audiooutput.h"
#include "log.h"

player::~player()
{
    swr_free(&m_swr);
}

std::string player::err2str(int errnum)
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
    if(m_videooutput)
    {
        m_videooutput->show2(m_frame);
    }
    else
    {
        LOGE("video output is nullptr");
        return -1;
    }
    return 0;
    // if(frame == nullptr)
    // {
    //     LOGE("fail to show video frame");
    //     return -1;
    // }
    // if(frame->format == AV_PIX_FMT_YUV420P)
    // {
    //     double pts = frame->best_effort_timestamp * av_q2d(m_video_stream->time_base);
    //     LOGI("video pts:{}", pts);
    //     yuv lyuv =
    //     {
    //         frame->data[0],
    //         frame->data[1],
    //         frame->data[2],
    //         frame->linesize[0],
    //         frame->linesize[1],
    //         frame->linesize[2]
    //     };
        
    //     if(m_videooutput)
    //     {
    //         m_videooutput->show(lyuv);
    //     }
    // }
    // return 0;
}

int player::output_audio_frame()
{
    if(m_frame == nullptr)
    {
        return -1;
    }
    size_t unpadded_linesize = m_frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)m_frame->format);
 
    double pts = m_frame->best_effort_timestamp * av_q2d(m_audio_stream->time_base);
    //LOGI("Audio pts:{}", pts);
//    LOGI("Audio frame->nb_samples={}", frame->nb_samples);
//    LOGE("{}", frame->pts);
    std::call_once(m_once_flag,
    [&](void)->void
    {
        if(!config_audio_output())
        {
            LOGE("config audio fail");
        }
        else
        {
            LOGI("config audio success");
        }
    });

    int out_samples = av_rescale_rnd(
        swr_get_delay(m_swr, m_frame->sample_rate) + m_frame->nb_samples,
        m_frame->sample_rate,
        m_frame->sample_rate,
        AV_ROUND_UP
    );

    /* allocate buffer output */
    int out_linesize = 0;
    AudioS16Buffer out{};
    uint64_t ch_layout =
        m_frame->channel_layout ?
        m_frame->channel_layout :
        av_get_default_channel_layout(m_frame->channels);
    int out_channels = av_get_channel_layout_nb_channels(ch_layout);
    av_samples_alloc(
        &out.data,
        &out_linesize,
        out_channels,
        out_samples,
        AV_SAMPLE_FMT_S16,
        0
    );

    /*  Convert */
    int samples = swr_convert(
        m_swr,
        &out.data,
        out_samples,
        (const uint8_t**)m_frame->data,
        m_frame->nb_samples
    );

    if (samples <= 0)
    {
        av_freep(&out.data);
        LOGE("fail to convert data");
        return false;
    }
    out.size = samples * out_channels * sizeof(int16_t);

    if(out.data && out.size > 0)
    {
        m_audiooutput->push(out.data, out.size);
        av_freep(&out.data);
    }
    // std::unordered_map<uint8_t, std::string> table =
    // {
    //     {0, "No Audio"},
    //     {1, "Mono"},
    //     {2, "Stereo"},
    //     {3, "Suround"},
    // };
    // std::cout << "Channels:" << table[frame->channels] << std::endl;
    // Need check:
    // sample rate 44000 Hz
    // sample format s16, each sample use signed int data
    // sample channel : mono -> single, stereo -> double channel
    // plannar or packed -> plannar (separate channel left right), packed (data packed in format R L R L R L)
    /* Write the raw audio data samples of the first plane. This works
     * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
     * most audio decoders output planar audio, which uses a separate
     * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
     * In other words, this code will write only the first audio channel
     * in these cases.
     * You should use libswresample or libavfilter to convert the frame
     * to packed data. */
    return 0;
}

bool player::config_audio_output()
{
    if(m_frame == nullptr)
    {
        return false;
    }
    double first_pts = m_frame->best_effort_timestamp * av_q2d(m_audio_stream->time_base);
    uint64_t ch_layout =
    m_frame->channel_layout ?
    m_frame->channel_layout :
    av_get_default_channel_layout(m_frame->channels);
    m_audiooutput = std::make_unique<audiooutput>(this);
    if(m_audiooutput == nullptr)
    {
        LOGE("m_audiooutput = nullptr");
        return false;
    }

    if(!m_audiooutput->config(m_frame->sample_rate,m_frame->channels ,AUDIO_S16SYS, first_pts))
    {
        std::cerr << "config audio error" << std::endl;
        return false;
    }
    m_audiooutput->start();
    // init software context
    
    m_swr = swr_alloc_set_opts(
    nullptr,
    ch_layout,
    AV_SAMPLE_FMT_S16,
    m_frame->sample_rate,
    m_frame->channel_layout,
    (AVSampleFormat)m_frame->format,
    m_frame->sample_rate,

    0, nullptr);
    if(!m_swr || swr_init(m_swr))
    {
        swr_free(&m_swr);
        LOGE("!m_swr || swr_init(m_swr)");
        return false;
    }
    return true;
}

int player::decode_packet(AVCodecContext *dec, const AVPacket *pkt)
{
    int ret = 0;
 
    // submit the packet to the decoder
    ret = avcodec_send_packet(dec, pkt);
    if (ret < 0)
    {
        LOGE("Error submitting a packet for decoding {}", err2str(ret));
        return ret;
    }
 
    // get all the available frames from the decoder
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(dec, m_frame);
        if (ret < 0)
        {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;
 
            LOGE("Error during decoding {}", err2str(ret));
            return ret;
        }
 
        // write the frame data to output file
        if(dec->codec->type == AVMEDIA_TYPE_VIDEO)
        {
            ret = output_video_frame();
        }
        else if(dec->codec->type == AVMEDIA_TYPE_AUDIO)
        {
            // TBD
            ret = output_audio_frame();
        }
        else
        {
            LOGW("Not support this packet");
        }
        av_frame_unref(m_frame);
    }
    
    return ret;
}

int player::open_codec_context(int *stream_idx,
                              AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    const AVCodec *dec = NULL;
 
    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0)
    {
        LOGE("Could not find {} stream in input file {}", av_get_media_type_string(type), m_src_filename);
        return ret;
    }
    else
    {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];
 
        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec)
        {
            LOGE("Failed to find {} codec", av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }
 
        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx)
        {
            LOGE("Failed to allocate the {} codec context", av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }
 
        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0)
        {
            LOGE("Failed to copy {} codec parameters to decoder context", av_get_media_type_string(type));
            return ret;
        }
 
        /* Init the decoders */
        if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0)
        {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            LOGE("Failed to open {} codec", av_get_media_type_string(type));
            return ret;
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
    int ret = 0;
    if (argc != 2)
    {
        LOGE("In valid parameter: usage {} video.mp4", argv[0]);
        exit(1);
    }
    m_src_filename = argv[1];
    /* open input file, and allocate format context */
    if (avformat_open_input(&m_fmt_ctx, m_src_filename.c_str(), NULL, NULL) < 0)
    {
        LOGE("Could not open source file {}", m_src_filename);
        exit(1);
    }
 
    /* retrieve stream information */
    if (avformat_find_stream_info(m_fmt_ctx, NULL) < 0)
    {
        LOGE("Could not find stream information");
        exit(1);
    }
 
    if(open_codec_context(&m_video_stream_idx, &m_video_dec_ctx, m_fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0)
    {
        m_video_stream = m_fmt_ctx->streams[m_video_stream_idx];
        
        
        /* allocate image where the decoded image will be put */
        m_width = m_video_dec_ctx->width;
        m_height = m_video_dec_ctx->height;
        // Create windows
        m_videooutput = std::make_unique<videooutput>(m_width, m_height, this);

        m_pix_fmt = m_video_dec_ctx->pix_fmt;
        ret = av_image_alloc(m_video_dst_data, m_video_dst_linesize, m_width, m_height, m_pix_fmt, 1);
        if (ret < 0)
        {
            LOGE("Could not allocate raw video buffer");
            clean_resource();
            return -1;
        }
        m_video_dst_bufsize = ret;
    }
 
    if (open_codec_context(&m_audio_stream_idx, &m_audio_dec_ctx, m_fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0)
    {
        m_audio_stream = m_fmt_ctx->streams[m_audio_stream_idx];
        LOGI("sample rate: {}", m_audio_dec_ctx->sample_rate);
        LOGI("channel layout: {}", m_audio_dec_ctx->channel_layout);
    }
 
    /* dump input information to stderr */
    av_dump_format(m_fmt_ctx, 0, m_src_filename.c_str(), 0);
 
    if (!m_audio_stream && !m_video_stream)
    {
        LOGE("Could not find audio or video stream in the input, aborting");
        ret = 1;
        clean_resource();
        return 1;
    }
 
    m_frame = av_frame_alloc();
    if(!m_frame)
    {
        LOGE("Could not allocate frame");
        ret = AVERROR(ENOMEM);
        clean_resource();
        return -1;
    }
 
    m_pkt = av_packet_alloc();
    if(!m_pkt)
    {
        LOGE("Could not allocate packet");
        ret = AVERROR(ENOMEM);
        clean_resource();
        return -1;
    }
 
    /* read frames from the file */
    while (av_read_frame(m_fmt_ctx, m_pkt) >= 0)
    {
        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
        if (m_pkt->stream_index == m_video_stream_idx)
            ret = decode_packet(m_video_dec_ctx, m_pkt);
        else if (m_pkt->stream_index == m_audio_stream_idx)
            ret = decode_packet(m_audio_dec_ctx, m_pkt);
        av_packet_unref(m_pkt);
        if (ret < 0)
            break;
    }
 
    /* flush the decoders */
    if (m_video_dec_ctx)
    {
        decode_packet(m_video_dec_ctx, NULL);
    }
    if (m_audio_dec_ctx)
    {
        decode_packet(m_audio_dec_ctx, NULL);
    }
    LOGI("Demuxing succeeded");
    while(true);
    if (m_video_stream)
    {
        LOGI("Play the output video file with the command:"
               "ffplay -f rawvideo -pix_fmt {} -video_size {}x{}",
               av_get_pix_fmt_name(m_pix_fmt), m_width, m_height);
    }
 
    if (m_audio_stream)
    {
        enum AVSampleFormat sfmt = m_audio_dec_ctx->sample_fmt;
        int n_channels = /*m_audio_dec_ctx->ch_layout.nb_channels;*/ 1;
       
// int n_channels = m_audio_dec_ctx->ch_layout.nb_channels > 0
//                          ? m_audio_dec_ctx->ch_layout.nb_channels
//                          : m_audio_dec_ctx->channels;
 
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

void player::clean_resource()
{
    avcodec_free_context(&m_video_dec_ctx);
    avcodec_free_context(&m_audio_dec_ctx);
    avformat_close_input(&m_fmt_ctx);
    av_packet_free(&m_pkt);
    av_frame_free(&m_frame);
    av_free(m_video_dst_data[0]);
}

double player::GetAudioClock()
{
    double audioclock{};
    if(m_audiooutput != nullptr)
    {
        audioclock = m_audiooutput->get_clock();
    }
    return audioclock;
}