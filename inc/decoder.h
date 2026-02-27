#ifndef _DECODER_
#define _DECODER_

#include "define.h"
#include "mediator.h"

class decoder
{
public:
    decoder(mediator* mediator);

    virtual ~decoder();
    int init_decoder(const AVCodecID codecID, AVCodecParameters* codec_par);
    int decode_packet(UniquePacketPtr pkt, UniqueFramePtr& frame);
    const std::string err2str(int errnum);
protected:
    mediator*        m_mediator = nullptr;
    AVCodecContext*  m_CodecContext = nullptr;
};

#endif /* _DECODER */