#ifndef _VIEW_
#define _VIEW_

#include <SDL2/SDL.h>
#include "define.h"
#include "controlfunction.h"

class view : public controlfunction
{
public:
    view();
    ~view();
    bool init();
    void Config(const int Width, const int Height);
    bool UpdateYUVTexture(const yuv& ndata);
    void Exit() override;
private:
    int                  m_Width    = 0;
    int                  m_Height   = 0;
    UniqueWindowPtr      m_Window   = nullptr;
    UniqueRenderPtr      m_Renderer = nullptr;
    UniqueTexturePtr     m_Texture  = nullptr;
};

#endif /* _VIEW */