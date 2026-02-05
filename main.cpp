#include <thread>
#include "player.h"
#include "audiooutput.h"
#include "videooutput.h"
int main(int argc, char** argv)
// {
//     player lplayer;
//     lplayer.run(argc, argv);
//     return 0;
// }
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

{
    constexpr int SAMPLE_RATE = 48000;
    constexpr int CHANNELS    = 2;
    constexpr int AMPLITUDE   = 3000;
    constexpr double FREQ     = 440.0; // A4
   
    audiooutput audio;
    if (!audio.config(SAMPLE_RATE, CHANNELS, AUDIO_S16SYS))
    {
        return -1;
    }

    audio.start();

    double phase = 0.0;
    double phase_inc = 2.0 * M_PI * FREQ / SAMPLE_RATE;



    while (true)
    {
        // 10 ms audio
        constexpr int samples = SAMPLE_RATE / 100;
        int16_t pcm[samples * CHANNELS];

        for (int i = 0; i < samples; ++i) {
            int16_t s = static_cast<int16_t>(
                std::sin(phase) * AMPLITUDE
            );
            phase += phase_inc;
            if (phase >= 2.0 * M_PI)
                phase -= 2.0 * M_PI;

            // stereo interleaved
            pcm[i * 2 + 0] = s;
            pcm[i * 2 + 1] = s;
        }

        audio.push(
            reinterpret_cast<uint8_t*>(pcm),
            sizeof(pcm)
        );

        std::this_thread::sleep_for(
            std::chrono::milliseconds(5)
        );
    }
    return 0;
}