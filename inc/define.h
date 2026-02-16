#ifndef _DEFINE_
#define _DEFINE_
#include <cstdint>
#include <atomic>

#define NAME_WINDOW "Video Media Player"

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

struct AudioClock
{
    std::atomic<double> pts;
    std::atomic<double> last_frame_pts;
    // int64_t total_sample;
    // int sample_rate;
    // int channels;
    // int byte_per_sample;
    // bool started;
};

#endif /*_DEFINE_*/