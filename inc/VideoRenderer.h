#ifndef VIDEO_RENDERER
#define VIDEO_RENDERER

#include <SDL_image.h>
#include "Define.h"

class VideoRenderer
{
public:
    bool Init(const UniqueWindowPtr& Window, const Size VideoSize);
    bool Resize(const UniqueWindowPtr& Window,const Size VideoSize);
    bool UpdateYUVTexture(const yuv& ndata);
    bool RenderClear();
    bool RenderCopyVideoTexture(const SDL_Rect& Rect);
    bool RenderCopyButtonPlay(const SDL_Rect& Rect);
    bool RenderCopyButtonNext(const SDL_Rect& Rect);
    bool RenderCopyButtonPrivious(const SDL_Rect& Rect);
    void Present();
private:
    bool CreateRenderer(const UniqueWindowPtr& Window);
    bool CreateVideoTexture(const Size VideoSize);
    void CalculateRect(const Size CurrentWindowSize);
    Size GetMaxWindowSize();
    void ClearYUVBlack();

    UniqueRenderPtr      m_Renderer       = nullptr;
    UniqueTexturePtr     m_VideoTexture   = nullptr;
    UniqueTexturePtr     m_ButtonPlay     = nullptr;
    UniqueTexturePtr     m_ButtonNext     = nullptr;
    UniqueTexturePtr     m_ButtonPrivious = nullptr;
    Size                 m_CurrentVideoSize = {};

    // Retangle
    SDL_Rect             m_VideoRect           = {};
    SDL_Rect             m_ControlAreaRect     = {};
    SDL_Rect             m_ButtonPlayRect      = {};
    SDL_Rect             m_ButtonNextRect      = {};
    SDL_Rect             m_ButtonPriviousRect  = {};
};

#endif /* VIDEO_RENDERER */