#ifndef _VIEW_
#define _VIEW_

#include <SDL2/SDL.h>
#include "Define.h"
#include "ControlFunction.h"
#include "AudioDevice.h"
#include "Window.h"
#include "VideoRenderer.h"
#include "SafeQueue.h"

class View : public ControlFunction
{
public:
    View();
    ~View();
    bool Init();
    void Config(const int Width, const int Height);
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
private:

    SafeQueue<UniqueFramePtr>   m_QueueSafe{};
    
    std::mutex           m_ResizeWindow;
    VideoRenderer        m_VideoRenderer;
    Window               m_Window;
    AudioDevice          m_AudioDevice;
    int                  m_CurrentVideoWidth    = 0;
    int                  m_CurrentVideoHeight   = 0;
    int                  m_ConfigVideoWidth     = 0;
    int                  m_ConfigVideoHeight    = 0;
};

#endif /* _VIEW */