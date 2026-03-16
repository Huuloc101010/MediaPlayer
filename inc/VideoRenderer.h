#ifndef VIDEO_RENDERER
#define VIDEO_RENDERER

#include "Define.h"

class VideoRenderer
{
public:
    bool Init(const UniqueWindowPtr& Window, const int Width, const int Height);
    bool Resize(const UniqueWindowPtr& Window,const int width, const int height);
    bool UpdateYUVTexture(const yuv& ndata);
private:
    bool CreateRenderer(const UniqueWindowPtr& Window);
    bool CreateTexture(const int Width, const int Height);

    UniqueRenderPtr      m_Renderer = nullptr;
    UniqueTexturePtr     m_Texture  = nullptr;
    int                  m_Width    = 0;
    int                  m_Height   = 0;
};

#endif /* VIDEO_RENDERER */