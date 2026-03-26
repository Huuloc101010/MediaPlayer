#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include "Define.h"
#include "Mediator.h"
#include "VideoDecoder.h"
#include "AudioDecoder.h"
#include "Demuxer.h"
#include "Controller.h"
#include "View.h"
#include "ControlFunction.h"
#include "VideoOutput.h"
#include "AudioOutput.h"
#include "PlayList.h"

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

class Player : public Mediator
             , public ControlFunction
{
public:
    Player();
    ~Player() = default;

    void Config(const std::string& MediaDirectory);
    int Start();
    void PushEvent(PlayerEvent Event) override;
    void MainThreadProcess();

private:
    double GetAudioClock() override;
    AVRational GetTimeBaseAudio() override;
    AVRational GetTimeBaseVideo() override;
    std::string err2str(int errnum) override;
    std::atomic<PlayerState>& GetCurrentState() override;
    bool ConfigVideoOutput() override;
    bool ConfigAudioOutput() override;
    bool InitVideoDecoder(const AVCodecID codecID, AVCodecParameters* codec_par) override;
    bool InitAudioDecoder(const AVCodecID codecID, AVCodecParameters* codec_par) override;
    bool PushVideoFrameToView(UniqueFramePtr frame) override;
    std::string ts2timestr(int64_t ts, AVRational tb);
    int OutputVideoFrame(UniqueFramePtr frame) override;
    int OutputAudioFrame(UniqueFramePtr frame) override;
    int DecodePacket(UniquePacketPtr pkt, const bool IsFlushDecoder = false) override;
    void PushSDLAudioData(const uint8_t* data, size_t Size) override;
    void TheadProcessEvent();
    bool AudioConfig(int sample_rate,
                     int channels,
                     SDL_AudioFormat format,
                     int first_pts,
                     int samples = 1024) override;
    int GetVideoWidth() override;
    int GetVideoHeight() override;
    void VideoConfig(int width, int height) override;
    Rect CheckInWhichButton(const Position postion) override;
    double GetTotalVideoTime() override;

    void EventQuit();
    void EventStop();
    void EventNext();
    void EventPrivious();
    void PlayOtherMedia();
    void EventPause();
    void EventPlay();
    void EventSeek();

    std::unique_ptr<Demuxer>        m_Demuxer      = nullptr;
    std::unique_ptr<VideoOutput>    m_VideoOutput  = nullptr;
    std::unique_ptr<AudioOutput>    m_AudioOutput  = nullptr;
    std::unique_ptr<VideoDecoder>   m_VideoDecoder = nullptr;
    std::unique_ptr<AudioDecoder>   m_AudioDecoder = nullptr;
    std::unique_ptr<Controller>     m_Controller   = nullptr;
    std::unique_ptr<View>           m_View         = nullptr;
    SafeQueue<PlayerEvent>          m_PlayerEvent  = {};
    PlayList                        m_PlayList     = {};
    std::jthread                    m_TheadProcessEvent;

    std::map<PlayerEvent, std::function<void()>>     m_MapProcessing = {};

};

#endif /*_PLAYER_H_*/