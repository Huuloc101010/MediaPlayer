#ifndef _DEFINE_
#define _DEFINE_
#include <cstdint>

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

#endif /*_DEFINE_*/