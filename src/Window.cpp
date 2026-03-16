#include "Window.h"

bool Window::Init(const int width, const int height)
{
    m_Width = width;
    m_Height = height;
    m_Window.reset(SDL_CreateWindow(NAME_WINDOW, SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, width,
                                    height, SDL_WINDOW_RESIZABLE));
    if(m_Window == nullptr)
    {
        LOGE("Create window SDL fail: {}", SDL_GetError());
        return false;
    }
    return true;
}

bool Window::Resize(const int width, const int height)
{
    if(m_Window == nullptr)
    {
        LOGE("Window is nullptr");
        return false;
    }
    SDL_SetWindowSize(m_Window.get(), width, height);
    return true;
}

UniqueWindowPtr& Window::Get()
{
    return m_Window;
}