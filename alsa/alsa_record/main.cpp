#include <iostream>
#include <sys/time.h>

#include "AudioCapture.h"
#include "AACEncoder.h"


using namespace std;

uint64_t GetCurrentMillisecs()
{
    timeval tv;
    ::gettimeofday(&tv, 0);
    unsigned long long ret = tv.tv_sec;
    return ret * 1000 + tv.tv_usec / 1000;
}

// 输入 ./alsa_reord 1000
// 即是录制1000秒
int main(int argc, char *argv[])
{
    uint64_t start_time = GetCurrentMillisecs();
    uint64_t record_time = 1000*60;    // 默认1分钟
    if(argc == 2)
    {
        record_time = atoi(argv[1]);
        record_time *= 1000;        // 换算成毫秒
    }

    FILE *aac_file = fopen("record.aac", "wb+");

    printf("Init aac encoder\n");
    AACEncoder aac_encoder;
    if(!aac_encoder.Init(44100, 2, 128000, 2))
    {
        printf("AACEncoder init failed");
        return -1;
    }

     printf("Init audio capture\n");
    AudioCapture audio_capture;
    if(!audio_capture.Init("default", 44100, 2, 1024))
    {
        printf("AudioCapture init failed");
        return -1;
    }

    uint8_t enc_data[8192]; // 2的13次方 AAC帧最大长度
    int enc_len = 0;
    int aac_total_len = 0;
    while (true)
    {
        uint64_t cur_time = GetCurrentMillisecs();
        if(cur_time - start_time >= record_time)
        {
            printf("record time reach to %fs, so finish it\n", (float)(record_time/1000.0));
            break;
        }
        if((cur_time - start_time) % 5000 == 0)
        {
            printf("current reord time = %fs\n", (float)((cur_time - start_time)/1000.0));
        }
        PCMFrame frame(0);
        if (audio_capture.GetFrame(frame))
        {
            enc_len = 8192; // 2的13次方
            aac_encoder.Encode((const uint8_t *)frame.data.get(),
                               frame.size, enc_data,
                               enc_len);
            if (enc_len > 0)
            {
                aac_total_len += enc_len;
                fwrite(enc_data, enc_len, 1, aac_file);
            }
            else
            {
                printf("aac encoder encode failed\n");
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    audio_capture.DeInit();
    aac_encoder.DeInit();
    fclose(aac_file);
    printf("reord exit\n");
    return 0;
}
