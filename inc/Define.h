#ifndef _DEFINE_
#define _DEFINE_
#include <cstdint>
#include <atomic>
#include <mutex>
#include <deque>
#include <memory>
#include <SDL2/SDL.h>
#include "Log.h"
extern "C"
{
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
 
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#define NAME_WINDOW                  "Video Media Player"
#define ICON_BUTTON_PLAY_PATH        "icon/play.png"
#define ICON_BUTTON_NEXT_PATH        "icon/next.png"
#define ICON_BUTTON_PRIVIOUS_PATH    "icon/previous.png"
#define ICON_CIRCLE                  "icon/circle.png"
#define DEFAULT_WINDOW_HEIGHT    780 
#define DEFAULT_WINDOW_CONTROL   150 
#define DEFAULT_WINDOW_WIDTH     1280 
#define DEFAULT_BUTTON_HEIGHT    75
#define DEFAULT_BUTTON_WIDTH     75
#define DEFAULT_SEEK_BAR_HEIGHT  7
#define DEFAULT_SEEK_BAR_WIDTH   700
#define DEFAULT_CIRCLE_HEIGHT    20
#define DEFAULT_CIRCLE_WIDTH     20
#define LIMIT_QUEUE_VIDEO_PACKET 50
#define LIMIT_QUEUE_AUDIO_PACKET 100
#define LIMIT_QUEUE_VIDEO_FRAME  7
#define LIMIT_QUEUE_AUDIO_FRAME  20
#define COLOR_BLACK              RGBA{0, 0, 0, 255}
#define COLOR_WHITE              RGBA{255, 255, 255, 255}
#define COLOR_GREEN              RGBA{33, 150, 243, 255}

template<typename T, void(*FreeFunction)(T*)>
struct UniquePtrDeleterLevel1
{
    void operator()(T* resource) const
    {
        if(resource)
        {
            FreeFunction(resource);
        }
    }
};

template<typename T, void(*FreeFunction)(T**)>
struct UniquePtrDeleterLevel2
{
    void operator()(T* resource) const
    {
        if(resource)
        {
            FreeFunction(&resource);
        }
    }
};

// SDL2 resource
using UniqueWindowPtr     = std::unique_ptr<SDL_Window, UniquePtrDeleterLevel1<SDL_Window, SDL_DestroyWindow>>;
using UniqueRenderPtr     = std::unique_ptr<SDL_Renderer, UniquePtrDeleterLevel1<SDL_Renderer, SDL_DestroyRenderer>>;
using UniqueTexturePtr    = std::unique_ptr<SDL_Texture, UniquePtrDeleterLevel1<SDL_Texture, SDL_DestroyTexture>>;


// ffmpeg resource
using UniqueFramePtr      = std::unique_ptr<AVFrame, UniquePtrDeleterLevel2<AVFrame, av_frame_free>>;
using UniquePacketPtr     = std::unique_ptr<AVPacket, UniquePtrDeleterLevel2<AVPacket, av_packet_free>>;
using UniqueFormatContext = std::unique_ptr<AVFormatContext, UniquePtrDeleterLevel2<AVFormatContext, avformat_close_input>>;


struct yuv
{
    uint8_t* plane_y;
    uint8_t* plane_u;
    uint8_t* plane_v;
    int linesize_y;
    int linesize_u;
    int linesize_v;
};

struct Size
{
    int Height;
    int Width;
    bool operator==(const Size& other) const = default;
    bool operator!=(const Size& other) const = default;
    bool operator<=(const Size& other) const = default;
    bool operator>=(const Size& other) const = default;
    Size& operator=(const Size& other)       = default;
    explicit operator bool() const
    {
        return (Height > 0) || (Width > 0);
    }
};

struct Position
{
    int x, y;
};

struct RGBA
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
};

struct AudioS16Buffer
{
    uint8_t* data = nullptr;
    int      size = 0;   // bytes
};

struct Clock
{
    std::atomic<double> pts;
    std::atomic<double> last_frame_pts;
};

enum class PlayerState : uint8_t
{
    IDLE,
    STOPPED,
    PLAYING,
    PAUSED,
    EXITING,
};

enum class PlayerEvent : uint8_t
{
    QUIT,
    STOP,
    NEXT,
    PRIVIOUS,
    PAUSE,
    PLAY,
    SEEK,
};

enum class Rect : uint8_t
{
    NONE,
    PLAY,
    NEXT,
    PRIVIOUS,
    SEEK,
};
#endif /*_DEFINE_*/