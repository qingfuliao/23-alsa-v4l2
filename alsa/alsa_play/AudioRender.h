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
    bool start();       // �����߳�
    bool stop();        // ֹͣ�߳�
    void run();         // loop���룬�����ȡ��������
    std::shared_ptr<RingBuffer<PCMFrame>> frame_buffer_;
    uint32_t samplerate_ = 44100;
    int channels_ = 2;
    snd_pcm_t *alsa_handle_ = nullptr;
    snd_pcm_uframes_t frames_ = 1024;
    uint8_t *frames_buf_ = nullptr;
    uint8_t frames_size = 4096; // frames * 4;  2 bytes/sample, 2 channels

    bool is_opened_ = false;

    std::shared_ptr<std::thread> thread_;
    bool running_ = false;	// �߳��Ƿ�������̬
    bool requet_abort_ = false;
};

#endif // AUDIORENDER_H
