#include <thread>
#include "player.h"
#include "audiooutput.h"
#include "videooutput.h"
int main(int argc, char** argv)
{
    player Player;
    try
    {
        Player.Config(argv[1]);
        Player.Start();
    }
    catch(const std::exception& e)
    {
        LOGE("{}", e.what());
        return -1;
    }
    catch(...)
    {
        LOGE("Exception has occurred");
        return -1;
    }
    
    return 0;
}
