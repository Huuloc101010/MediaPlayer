#ifndef _VIDEO_OUTPUT_
#define _VIDEO_OUTPUT_

#include <mutex>
#include <thread>
#include <shared_mutex>
#include <SDL2/SDL.h>
#include <iostream>
#include <memory>
#include "define.h"
#include "output.h"

class mediator;

class videooutput : public output
{
public:
    videooutput(const int width,const int height, mediator* mediator);
    ~videooutput();
    bool UpdateYUVTexture(const yuv& ndata);
    bool ConvertFramePtrToRawData(UniqueFramePtr frame);
    void ThreadProcessFramePtr() override;

private:
    void checkevent();
    bool init();
    void destroy();
    mediator*            m_Mediator{};
    std::jthread         m_ThreadCheckEvent;
    int                  m_Width = 0;
    int                  m_Height = 0;
    SDL_Window*          m_Window = nullptr;
    SDL_Renderer*        m_Renderer = nullptr;
    SDL_Texture*         m_Texture = nullptr;
    SDL_Event            m_Event;
};

#endif /*_VIDEO_OUTPUT_*/