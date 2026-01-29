#ifndef _DEFINE_
#define _DEFINE_
#include <vector>
#include <cstdint>

struct yuv
{
    std::vector<uint8_t> plane_y;
    std::vector<uint8_t> plane_u;
    std::vector<uint8_t> plane_v;
};

#endif /*_DEFINE_*/