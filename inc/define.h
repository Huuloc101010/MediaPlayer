#ifndef _DEFINE_
#define _DEFINE_
#include <cstdint>
#include <atomic>
#include <mutex>
#include <deque>
#include <memory>
#include <SDL2/SDL.h>
#include "log.h"
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

#define NAME_WINDOW "Video Media Player"
#define LIMIT_QUEUE_VIDEO_PACKET 50
#define LIMIT_QUEUE_AUDIO_PACKET 100
#define LIMIT_QUEUE_VIDEO_FRAME  7
#define LIMIT_QUEUE_AUDIO_FRAME  20

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
};

#endif /*_DEFINE_*/