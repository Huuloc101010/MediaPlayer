#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include "define.h"
#include "mediator.h"
#include "videodecoder.h"
#include "audiodecoder.h"
#include "demuxer.h"
#include "controller.h"
#include "view.h"
#include "controlfunction.h"

extern "C"
{
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
 
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

class videooutput;
class audiooutput;

class player : public mediator
             , public controlfunction
{
public:
    player();
    ~player() = default;

    void Config(const std::string& MediaFile);
    int Start();
    void PushEvent(PlayerEvent Event) override;

private:
    double GetAudioClock() override;
    AVRational GetTimeBaseAudio() override;
    AVRational GetTimeBaseVideo() override;
    const std::string err2str(int errnum) override;
    std::atomic<PlayerState>& GetCurrentState() override;
    bool ConfigVideoOutput() override;
    bool ConfigAudioOutput() override;
    bool InitVideoDecoder(const AVCodecID codecID, AVCodecParameters* codec_par) override;
    bool InitAudioDecoder(const AVCodecID codecID, AVCodecParameters* codec_par) override;
    bool InitView() override;
    bool UpdateYUVTexture(const yuv& ndata) override;
    std::string ts2timestr(int64_t ts, AVRational tb);
    int output_video_frame(UniqueFramePtr frame) override;
    int output_audio_frame(UniqueFramePtr frame) override;
    bool config_audio_output();
    int decode_packet(UniquePacketPtr pkt, const bool IsFlushDecoder = false) override;
    void TheadProcessEvent();

    void EventQuit();
    void EventStop();
    void EventNext();
    void EventPause();
    void EventPlay();

    std::unique_ptr<demuxer>        m_Demuxer      = nullptr;
    std::unique_ptr<videooutput>    m_VideoOutput  = nullptr;
    std::unique_ptr<audiooutput>    m_AudioOutput  = nullptr;
    std::unique_ptr<videodecoder>   m_VideoDecoder = nullptr;
    std::unique_ptr<audiodecoder>   m_AudioDecoder = nullptr;
    std::unique_ptr<controller>     m_Controller   = nullptr;
    std::unique_ptr<view>           m_View         = nullptr;
    queue_safe<PlayerEvent>         m_PlayerEvent  = {};
    std::string                     m_CurrentMedia = {};
    std::jthread                    m_TheadProcessEvent;

    std::map<PlayerEvent, std::function<void()>>     m_MapProcessing = {};

};

#endif /*_PLAYER_H_*/