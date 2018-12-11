#ifndef AUDIORENDER_H
#define AUDIORENDER_H


class AudioRender
{
ppublic:
    AudioRender();
    ~AudioRender();
    bool Init(const char* name = "default",
              uint32_t samplerate = 44100,
              int channels = 2,
              int frames = 1024);

    bool DeInit();
    bool PushFrame(PCMFrame& frame);
private:
    bool start();       // 启动线程
    bool stop();        // 停止线程
    void run();         // loop代码，负责读取声卡数据
    std::shared_ptr<RingBuffer<PCMFrame>> frame_buffer_;
    uint32_t samplerate_ = 44100;
    int channels_ = 2;
    snd_pcm_t *alsa_handle_ = nullptr;
    snd_pcm_uframes_t frames_ = 1024;
    uint8_t *frames_buf_ = nullptr;
    uint8_t frames_size = 4096; // frames * 4;  2 bytes/sample, 2 channels

    bool is_opened_ = false;

    std::shared_ptr<std::thread> thread_;
    bool running_ = false;	// 线程是否处于运行态
    bool requet_abort_ = false;
};

#endif // AUDIORENDER_H
