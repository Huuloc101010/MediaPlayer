#ifndef _VIEW_
#define _VIEW_

#include <SDL2/SDL.h>
#include "define.h"

class view
{
public:
    view();
    ~view();
    bool init();
    void destroy();
    void Config(const int Width, const int Height);
    bool UpdateYUVTexture(const yuv& ndata);

private:
    int                  m_Width = 0;
    int                  m_Height = 0;
    SDL_Window*          m_Window = nullptr;
    SDL_Renderer*        m_Renderer = nullptr;
    SDL_Texture*         m_Texture = nullptr;
};

#endif /* _VIEW */