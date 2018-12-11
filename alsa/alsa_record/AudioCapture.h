#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include <cstdint>
#include <memory>
#include <thread>

#include <alsa/asoundlib.h>
#include "RingBuffer.h"

struct PCMFrame
{
    PCMFrame(uint32_t size = 100)
        : data(new uint8_t[size + 1024])
    {
        this->size = size;
    }
    uint32_t size = 0;
    uint32_t channels = 2;
    uint32_t samplerate = 44100;
    std::shared_ptr<uint8_t> data;
    int64_t timpestamp;
};

class AudioCapture
{
public:
    AudioCapture();
    ~AudioCapture();
    bool Init(const char* name = "default",
              uint32_t samplerate = 44100,
              int channels = 2,
              int frames = 1024);

    bool DeInit();
    bool GetFrame(PCMFrame& frame);
private:
    bool start();       // 启动线程
    bool stop();        // 停止线程
    void run();         // loop代码，负责读取声卡数据
    bool pushFrame(uint8_t *data, int size);
    std::shared_ptr<RingBuffer<PCMFrame>> frame_buffer_;
    uint32_t samplerate_ = 44100;
    int channels_ = 2;
    snd_pcm_t *alsa_handle_ = nullptr;
    snd_pcm_uframes_t actual_frames_ = 1024;
    snd_pcm_uframes_t need_frames_ = 1024;


    bool is_opened_ = false;

    std::shared_ptr<std::thread> thread_;
    bool running_ = false;	// 线程是否处于运行态
    bool requet_abort_ = false;
};

#endif // AUDIOCAPTURE_H
