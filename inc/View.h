#ifndef _VIEW_
#define _VIEW_

#include <SDL2/SDL.h>
#include "Define.h"
#include "ControlFunction.h"
#include "AudioDevice.h"
#include "Window.h"

class View : public ControlFunction
{
public:
    View();
    ~View();
    bool Init();
    void Config(const int Width, const int Height);
    bool UpdateYUVTexture(const yuv& ndata);
    void Play() override;
    void Pause()override;
    void Stop() override;
    void Exit() override;

    // Audio
    void Push(const uint8_t* data, size_t Size);
    bool ConfigAudioOutput(UniqueFramePtr& frame);
    bool Config(int sample_rate,
        int channels,
        SDL_AudioFormat format, int first_pts,
        int samples = 1024);
    std::atomic<double>& GetClock();
private:

    Window               m_Window;
    AudioDevice          m_AudioDevice;
    int                  m_Width    = 0;
    int                  m_Height   = 0;
    
    UniqueRenderPtr      m_Renderer = nullptr;
    UniqueTexturePtr     m_Texture  = nullptr;

};

#endif /* _VIEW */