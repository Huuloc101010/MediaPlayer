#ifndef _DECODER_
#define _DECODER_

#include <thread>
#include "define.h"
#include "mediator.h"
#include "queue_safe.h"

class decoder
{
public:
    decoder(mediator* mediator);

    virtual ~decoder();
    int init_decoder(const AVCodecID codecID, AVCodecParameters* codec_par);
    const std::string err2str(int errnum);
    void PushPacket(UniquePacketPtr Packet);
    virtual void SetLimitQueueDecoder(const int LimitValue);
protected:
    void ThreadDecode();
    int decode_packet(UniquePacketPtr pkt);
    mediator*                    m_mediator = nullptr;
    AVCodecContext*              m_CodecContext = nullptr;
    queue_safe<UniquePacketPtr>  m_QueueSafe{};
    std::jthread                 m_ThreadDecode{};
};

#endif /* _DECODER */