#ifndef _DEFINE_
#define _DEFINE_
#include <cstdint>
#include <atomic>
#include <mutex>
#include <deque>
#include <memory>
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

template<typename T, void(*FreeFunction)(T**)>
struct UniquePtrDeleter
{
    void operator()(T* resource) const
    {
        if(resource)
        {
            FreeFunction(&resource);
        }
    }
};
using UniqueFramePtr      = std::unique_ptr<AVFrame, UniquePtrDeleter<AVFrame, av_frame_free>>;
using UniquePacketPtr     = std::unique_ptr<AVPacket, UniquePtrDeleter<AVPacket, av_packet_free>>;
using UniqueFormatContext = std::unique_ptr<AVFormatContext, UniquePtrDeleter<AVFormatContext, avformat_close_input>>;


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
    // int64_t total_sample;
    // int sample_rate;
    // int channels;
    // int byte_per_sample;
    // bool started;
};

enum class PlayerState : uint8_t
{
    IDLE,
    STOPPED,
    PLAYING,
    PAUSED,
};

#endif /*_DEFINE_*/