#ifndef _DECODER_
#define _DECODER_

#include "define.h"
#include "mediator.h"

class decoder
{
public:
    decoder(mediator* mediator)
    {
        m_mediator = mediator;
    }

    ~decoder()
    {
        
    }

    int init_decoder(AVCodecID codecID, AVCodecContext **dec_ctx, AVCodecParameters* codec_par)
    {
        int ret = -1;
        const AVCodec *dec = NULL;
        /* find decoder for the stream */
        dec = avcodec_find_decoder(codecID);
        if (!dec)
        {
            LOGE("Failed to find {} codec", 1/*, av_get_media_type_string(type)*/);
            return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx)
        {
            LOGE("Failed to allocate the {} codec context", 1/*,av_get_media_type_string(type)*/);
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, codec_par)) < 0)
        {
            LOGE("Failed to copy {} codec parameters to decoder context", 1/*, av_get_media_type_string(type)*/);
            return ret;
        }

        /* Init the decoders */
        if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0)
        {
            // fprintf(stderr, "Failed to open %s codec\n",
            //         av_get_media_type_string(type));
            LOGE("Failed to open {} codec", 1/*, av_get_media_type_string(type)*/);
            return ret;
        }
        return 0;
    }


    int decode_packet(AVCodecContext *dec, UniquePacketPtr pkt, UniqueFramePtr& frame)
    {
        int ret = 0;
    
        // submit the packet to the decoder
        ret = avcodec_send_packet(dec, pkt.get());
        if (ret < 0)
        {
            LOGE("Error submitting a packet for decoding {}", 1/*err2str(ret)*/);
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
            ret = avcodec_receive_frame(dec, frame.get());
            if (ret < 0)
            {
                // those two return values are special and mean there is no output
                // frame available, but there were no errors during decoding
                if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                    return 0;
    
                LOGE("Error during decoding {}",1 /*err2str(ret)*/);
                return ret;
            }
    
            // write the frame data to output file
            if(dec->codec->type == AVMEDIA_TYPE_VIDEO)
            {
                ret = m_mediator->output_video_frame();
            }
            else if(dec->codec->type == AVMEDIA_TYPE_AUDIO)
            {
                // TBD
                ret = m_mediator->output_audio_frame();
            }
            else
            {
                LOGW("Not support this packet");
            }
        }
        
        return ret;
    }

   

private:
    mediator* m_mediator = nullptr;
};

#endif /* _DECODER */