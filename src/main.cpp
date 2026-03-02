#include <thread>
#include "player.h"
#include "audiooutput.h"
#include "videooutput.h"
int main(int argc, char** argv)
{
    player lplayer;
    try
    {
        lplayer.run(argc, argv);
    }
    catch(const std::exception& e)
    {
        LOGE("{}", e.what());
        return -1;
    }
    
    return 0;
}
