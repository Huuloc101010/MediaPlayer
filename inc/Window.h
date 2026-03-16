#ifndef WINDOW
#define WINDOW
#include "Define.h"

class Window
{
public:
    bool Init(const int width = DEFAULT_WINDOW_WIDTH, const int height = DEFAULT_WINDOW_HEIGHT);
    bool Resize(const int width, const int height);
    UniqueWindowPtr& Get();

private:
    UniqueWindowPtr      m_Window   = nullptr;
    int                  m_Width    = {};
    int                  m_Height   = {};
};

#endif /* WINDOW */