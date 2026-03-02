#include <thread>
#include "player.h"
#include "audiooutput.h"
#include "videooutput.h"
int main(int argc, char** argv)
{
    player Player;
    try
    {
        Player.Start(argv[1]);
    }
    catch(const std::exception& e)
    {
        LOGE("{}", e.what());
        return -1;
    }
    
    return 0;
}
