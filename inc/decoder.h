#ifndef _DECODER_
#define _DECODER_

#include "define.h"
#include "mediator.h"

class decoder
{
public:
    decoder(mediator* mediator);

    virtual ~decoder() = default;
    int init_decoder(AVCodecID codecID, AVCodecContext **dec_ctx, AVCodecParameters* codec_par);
    int decode_packet(AVCodecContext *dec, UniquePacketPtr pkt, UniqueFramePtr& frame);

private:
    mediator* m_mediator = nullptr;
};

#endif /* _DECODER */