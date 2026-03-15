#include <thread>
#include "Player.h"
int main(int argc, char** argv)
{
    (void)argc;
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
    // Sleep this thread forever
    std::atomic<bool> SleepForever = true;
    SleepForever.wait(true);
    return 0;
}
