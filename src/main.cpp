#include <thread>
#include "Player.h"
int main(int argc, char** argv)
{
    (void)argc;
    Player Player;
    try
    {
        Player.Config(argv[1]);
        Player.Start();
        Player.MainThreadProcess();
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
