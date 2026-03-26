#ifndef _VIEW_
#define _VIEW_

#include <SDL2/SDL.h>
#include "Define.h"
#include "ControlFunction.h"
#include "AudioDevice.h"
#include "Window.h"
#include "VideoRenderer.h"
#include "SafeQueue.h"

class Mediator;

class View : public ControlFunction
{
public:
    View(Mediator* mediator);
    ~View();
    bool Init();
    void Config(const Size WindowSize);
    void Play() override;
    void Pause()override;
    void Stop() override;
    void Exit() override;

    // Video
    void ShowVideo();
    void PushVideoFrame(UniqueFramePtr frame);

    // Audio
    void Push(const uint8_t* data, size_t Size);
    bool ConfigAudioOutput(UniqueFramePtr& frame);
    bool Config(int sample_rate,
        int channels,
        SDL_AudioFormat format, int first_pts,
        int samples = 1024);
    std::atomic<double>& GetClock();
    void CheckResizeWindow();
    Rect CheckInWhichButton(const Position position);
    std::optional<double> CheckSeekPercent(const Position position);
private:
    
    Size GetMaxWindowSize();
    void CalculateRect(const Size CurrentWindowSize);
    bool CheckInRect(const SDL_Rect& rect, const Position position);

    SafeQueue<UniqueFramePtr>   m_QueueSafe{};
    Mediator*            m_Mediator             = {};
    std::mutex           m_ResizeWindow;
    VideoRenderer        m_VideoRenderer;
    Window               m_Window;
    AudioDevice          m_AudioDevice;
    Size                 m_CurrentVideoSize     = {};
    Size                 m_ConfigVideoSize      = {};
    Size                 m_CurrentWindowSize    = {};
    Size                 m_MaxWindowSize        = {};

    // Retangle
    SDL_Rect             m_VideoRect           = {};
    SDL_Rect             m_ControlAreaRect     = {};
    SDL_Rect             m_ButtonPlayRect      = {};
    SDL_Rect             m_ButtonNextRect      = {};
    SDL_Rect             m_ButtonPriviousRect  = {};
    SDL_Rect             m_SeekBar             = {};
    SDL_Rect             m_ProgressBar         = {};
    SDL_Rect             m_Circle              = {};
};

#endif /* _VIEW */