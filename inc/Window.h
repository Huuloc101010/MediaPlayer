#ifndef WINDOW
#define WINDOW
#include "Define.h"

class Window
{
public:
    bool Init(const Size WindowSize = {DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_WIDTH});
    bool Resize(const Size WindowSize);
    UniqueWindowPtr& Get();

private:
    UniqueWindowPtr      m_Window            = nullptr;
    Size                 m_CurrentWindowSize = {};
};

#endif /* WINDOW */