#include "Window.h"

bool Window::Init(const Size WindowSize)
{
    m_CurrentWindowSize = WindowSize;
    m_Window.reset(SDL_CreateWindow(NAME_WINDOW, SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, WindowSize.Width,
                                    WindowSize.Height + DEFAULT_WINDOW_CONTROL, SDL_WINDOW_RESIZABLE));
    if(m_Window == nullptr)
    {
        LOGE("Create window SDL fail: {}", SDL_GetError());
        return false;
    }
    return true;
}

bool Window::Resize(const Size WindowSize)
{
    if(m_Window == nullptr)
    {
        LOGE("Window is nullptr");
        return false;
    }
    SDL_SetWindowSize(m_Window.get(), WindowSize.Width, WindowSize.Height + DEFAULT_WINDOW_CONTROL);
    return true;
}

UniqueWindowPtr& Window::Get()
{
    return m_Window;
}

Size Window::GetMaxWindowSize()
{
    Size Retval{};
    SDL_DisplayMode mode;
    if (SDL_GetCurrentDisplayMode(0, &mode) != 0)
    {
        LOGE("SDL_GetCurrentDisplayMode failed: {}", SDL_GetError());
    }
    else
    {
        Retval.Width  = mode.w;
        Retval.Height = mode.h;
        LOGI("Max screen size: {}x{}", Retval.Width, Retval.Height);
    }
    return Retval;
}

Size Window::GetCurrentWindowSize()
{
    Size CurrentWindowSize {};
    SDL_GetWindowSize(m_Window.get(), &CurrentWindowSize.Width, &CurrentWindowSize.Height);
    return CurrentWindowSize;
}