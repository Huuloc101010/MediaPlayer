#include <thread>
#include "player.h"
#include "videooutput.h"
int main(int argc, char** argv)
{
    player lplayer;
    lplayer.run(argc, argv);
    return 0;
}
// {
//     std::vector<uint8_t> y(500*500, 200); 
//     std::vector<uint8_t> u(500*500/2, 200); 
//     std::vector<uint8_t> v(500*500/2, 200); 
//     videooutput lvideoouput(500,500);
//     //lvideoouput.init();
//     while(true)
//     {
//         lvideoouput.show({y.data(), u.data(), v.data()});
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
//     return 0;
// }
