#ifndef VIDEO_RENDERER
#define VIDEO_RENDERER

#include "Define.h"

class VideoRenderer
{
public:
    bool Init(const UniqueWindowPtr& Window, const Size VideoSize);
    bool Resize(const UniqueWindowPtr& Window,const Size VideoSize);
    bool UpdateYUVTexture(const yuv& ndata);
private:
    bool CreateRenderer(const UniqueWindowPtr& Window);
    bool CreateTexture(const Size VideoSize);

    UniqueRenderPtr      m_Renderer = nullptr;
    UniqueTexturePtr     m_Texture  = nullptr;
    Size                 m_CurrentVideoSize = {};
};

#endif /* VIDEO_RENDERER */