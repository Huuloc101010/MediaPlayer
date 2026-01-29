#ifndef _DEFINE_
#define _DEFINE_
#include <cstdint>

struct yuv
{
    uint8_t* plane_y;
    uint8_t* plane_u;
    uint8_t* plane_v;
};

#endif /*_DEFINE_*/