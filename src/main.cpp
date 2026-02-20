#include <thread>
#include "player.h"
#include "audiooutput.h"
#include "videooutput.h"
int main(int argc, char** argv)
{
    player lplayer;
    lplayer.run(argc, argv);
    return 0;
}
