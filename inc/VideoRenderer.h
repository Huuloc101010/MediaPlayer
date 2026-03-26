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
    bool RenderCopySeekBar(const SDL_Rect& Rect);
    bool RenderCopyProgressBar(const SDL_Rect& Rect);
    bool RenderCircle(const SDL_Rect& Rect);
    void Present();
private:
    bool FillColorRect(const SDL_Rect& Rect, const RGBA Color);
    bool CreateRenderer(const UniqueWindowPtr& Window);
    bool CreateVideoTexture(const Size VideoSize);
    Size GetMaxWindowSize();
    void ClearYUVBlack();

    UniqueRenderPtr      m_Renderer       = nullptr;
    UniqueTexturePtr     m_VideoTexture   = nullptr;
    UniqueTexturePtr     m_CircleTexture  = nullptr;
    UniqueTexturePtr     m_ButtonPlay     = nullptr;
    UniqueTexturePtr     m_ButtonNext     = nullptr;
    UniqueTexturePtr     m_ButtonPrivious = nullptr;
    //UniqueTexturePtr     m_SeekBar        = nullptr;
    Size                 m_CurrentVideoSize = {};

};

#endif /* VIDEO_RENDERER */