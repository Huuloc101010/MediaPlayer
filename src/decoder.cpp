#include "decoder.h"


decoder::decoder(mediator* mediator): m_mediator(mediator)
{
}

decoder::~decoder()
{
    if(m_ThreadDecode.joinable())
    {
        m_ThreadDecode.join();
    }
    if(m_CodecContext)
    {
        avcodec_free_context(&m_CodecContext);
    }
}
int decoder::init_decoder(const AVCodecID codecID, AVCodecParameters* codec_par)
{
    int ret = -1;
    const AVCodec *dec = NULL;
    /* find decoder for the stream */
    dec = avcodec_find_decoder(codecID);
    if (!dec)
    {
        LOGE("Failed to find {} codec", av_get_media_type_string(codec_par->codec_type));
        return AVERROR(EINVAL);
    }

    /* Allocate a codec context for the decoder */
    m_CodecContext = avcodec_alloc_context3(dec);
    if (!m_CodecContext)
    {
        LOGE("Failed to allocate the {} codec context", av_get_media_type_string(codec_par->codec_type));
        return AVERROR(ENOMEM);
    }

    /* Copy codec parameters from input stream to output codec context */
    if ((ret = avcodec_parameters_to_context(m_CodecContext, codec_par)) < 0)
    {
        LOGE("Failed to copy {} codec parameters to decoder context", av_get_media_type_string(codec_par->codec_type));
        return ret;
    }

    /* Init the decoders */
    if ((ret = avcodec_open2(m_CodecContext, dec, NULL)) < 0)
    {
        LOGE("Failed to open {} codec", av_get_media_type_string(codec_par->codec_type));
        return ret;
    }
    m_ThreadDecode = std::jthread(&decoder::ThreadDecode, this);
    return 0;
}

void decoder::ThreadDecode()
{
    UniquePacketPtr Packet = nullptr;
    while(m_PlayerState.load() != PlayerState::EXITING)
    {
        auto PacketOpt = m_QueueSafe.pop();
        if(PacketOpt == std::nullopt)
        {
            LOGW("Null opt");
            continue;
        }
        Packet = std::move(PacketOpt.value());
        decode_packet(std::move(Packet));
    }

}

void decoder::PushPacket(UniquePacketPtr Packet)
{
    m_QueueSafe.push(std::move(Packet));
}

void decoder::SetLimitQueueDecoder(const int LimitValue)
{
    m_QueueSafe.SetLimitQueue(LimitValue);
}

int decoder::decode_packet(UniquePacketPtr pkt)
{
    if((m_CodecContext == nullptr) || (m_mediator == nullptr))
    {
        LOGE("m_CodecContext or m_mediator is nullptr");
        return -1;
    }
    int ret = 0;
    UniqueFramePtr frame(av_frame_alloc());
    if(frame == nullptr)
    {
        LOGE("AVFrame pointer is null");
        return -1;
    }

    //control flow here
    if(CheckStateExit())
    {
        return 0;
    }
    CheckStateSleep();

    // submit the packet to the decoder
    ret = avcodec_send_packet(m_CodecContext, pkt.get());
    if (ret < 0)
    {
        LOGE("Error submitting a packet for decoding {}", err2str(ret));
        return ret;
    }

    // get all the available frames from the decoder
    while (ret >= 0)
    {
        if(frame == nullptr)
        {
            LOGE("null");
            return -1;
        }
        ret = avcodec_receive_frame(m_CodecContext, frame.get());
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
        if(m_CodecContext->codec->type == AVMEDIA_TYPE_VIDEO)
        {
            if(m_mediator != nullptr)
            {
                ret = m_mediator->output_video_frame(std::move(frame));
                frame.reset(av_frame_alloc());
            }
        }
        else if(m_CodecContext->codec->type == AVMEDIA_TYPE_AUDIO)
        {
            if(m_mediator != nullptr)
            {
                ret = m_mediator->output_audio_frame(std::move(frame));
                frame.reset(av_frame_alloc());
            }
        }
        else
        {
            LOGW("Not support this packet");
        }
    }
    
    return ret;
}

const std::string decoder::err2str(int errnum)
{
    if(m_mediator == nullptr)
    {
        return {};
    }
    return m_mediator->err2str(errnum);
}

void decoder::Exit()
{
    controlfunction::Exit();
    m_QueueSafe.release();
    if(m_ThreadDecode.joinable())
    {
        m_ThreadDecode.join();
    }
}