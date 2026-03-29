#ifndef _DECODER_
#define _DECODER_

#include <thread>
#include <functional>
#include "Define.h"
#include "Mediator.h"
#include "SafeQueue.h"
#include "ControlFunction.h"

class Decoder : public ControlFunction
{
public:
    Decoder(Mediator* Mediator);

    virtual ~Decoder();
    int ConfigDecoder(const AVCodecID codecID, AVCodecParameters* codec_par);
    std::string err2str(int errnum);
    void PushPacket(UniquePacketPtr Packet);
    virtual void SetLimitQueueDecoder(const int LimitValue);
    void FlushDecoder();
    void Stop() override;
    void Exit() override;
    void FlushData() override;
protected:
    void ThreadDecode();
    int DecodePacket(UniquePacketPtr pkt);
    Mediator*                    m_mediator = nullptr;
    AVCodecContext*              m_CodecContext = nullptr;
    SafeQueue<UniquePacketPtr>   m_QueueSafe{};
    std::jthread                 m_ThreadDecode{};
};

#endif /* _DECODER */