#include "AudioRender.h"

AudioRender ::AudioRender ():
    samplerate_(44100),
    channels_(2),
    frames_(1024),
    thread_(nullptr),
    running_(false),
    requet_abort_(false)
{
    frame_buffer_.reset(new RingBuffer<PCMFrame>(20));
}

AudioRender ::~AudioRender ()
{
    DeInit();
}

bool AudioRender ::Init(const char* name, uint32_t samplerate,
                        int channels, int frames)
{
    int direct;
    snd_pcm_hw_params_t *param;

    samplerate_ = samplerate;
    channels_ = channels;
    frames_  = frames;
    printf("snd_pcm_open name = %s, default,\n", name);
    int ret = snd_pcm_open(&alsa_handle_, name, SND_PCM_STREAM_CAPTURE, 0);
    if (ret < 0)
    {
        fprintf(stderr,
                "unable to open pcm device: %s\n",
                snd_strerror(ret));
        exit(1);
    }
    else
    {
        printf("open success!\n");
        is_opened_= true;
    }
    if (is_opened_)
    {
        printf("snd_pcm_hw_params_alloca!\n");
        /* Allocate a hardware parameters object. */
        /* 分配一个硬件参数结构体 */
        snd_pcm_hw_params_alloca(&param);
        /* Fill it in with default values. */
            /* 使用默认参数 */
        snd_pcm_hw_params_any(alsa_handle_, param);
        /* Set the desired hardware parameters. */

            /* Interleaved mode */
        snd_pcm_hw_params_set_access(alsa_handle_, param, SND_PCM_ACCESS_RW_INTERLEAVED);
        /* Signed 16-bit little-endian format */
           /* 16位 小端 */

        snd_pcm_hw_params_set_format(alsa_handle_, param, SND_PCM_FORMAT_S16_LE);
        /* Two channels (stereo) */
            /* 双通道 */

        snd_pcm_hw_params_set_channels(alsa_handle_, param, channels);
        snd_pcm_hw_params_set_rate_near(alsa_handle_, param, &samplerate_, &direct);
        printf("snd_pcm_hw_params_set_period_size_near!\n");
        /* Set period size to 1024 frames. */
        /* 一个周期有 1024 帧, 一帧就是每个通道一个采样点 */
        snd_pcm_hw_params_set_period_size_near(alsa_handle_, param, &frames_ , &direct);
        printf("snd_pcm_hw_params!\n");
        /* Write the parameters to the driver */     /* 参数生效 */
        ret= snd_pcm_hw_params(alsa_handle_, param);
        if(ret < 0)
        {
            fprintf(stderr,
                    "unable snd_pcm_hw_params: %s\n",
                    snd_strerror(ret));
            is_opened_ = false;
            snd_pcm_drain(alsa_handle_);
            snd_pcm_close(alsa_handle_);
            return false;
        }
        else
        {
            printf("start\n");
            return start();
        }
    }
    else
    {
        printf("snd_pcm_open failed\n");
        return false;
    }
}
bool AudioRender ::DeInit()
{
    stop();
    if (is_opened_)
    {
        snd_pcm_drain(alsa_handle_);
        snd_pcm_close(alsa_handle_);
        is_opened_ = false;
    }
    if(data_buf_)
    {
        delete [] data_buf_;
        data_buf_ = nullptr;
    }
}
bool AudioRender ::start()
{
    if (!thread_)
    {
        thread_.reset(new thread(&AudioRender ::Run, this));
    }
    return true;
}
bool AudioRender ::stop()
{
    if (thread_)
    {
        requet_abort_ = true;
        thread_->join();			// 等等线程退出
        thread_.reset();
        requet_abort_ = false;
    }
    return true;
}
bool AudioRender ::GetFrame(PCMFrame& frame)
{
    if (frame_buffer_->IsEmpty())
    {
        return false;
    }

    return frame_buffer_->Pop(frame);
}

void AudioRender ::run()
{
    printf("AudioRender ::Run\n");
    running_ = true;
    while (true)
    {
        if (requet_abort_)
        {
            break;
        }
        if (!data_buf_)
        {
            data_buf_ = new uint8_t[frames_ * channels_ * 2];
        }

        int ret;
        for (int i = 0; i != 5; ++i)
        {
            printf("snd_pcm_readi::Run\n");
            ret = snd_pcm_readi(alsa_handle_, data_buf_, frames_);
            if (ret == -EPIPE)
            {
                // overrun, retry!
                snd_pcm_prepare(alsa_handle_);
            }
            else if (ret < 0)
            {
                printf("snd_pcm_readi failed, ret = %s\n", snd_strerror(ret));
                break;
            }
            else
            {
                // maybe read incompletely
                // 插入一帧数据
                // printf("snd_pcm_readi = %d\n", ret);
                if (!frame_buffer_->IsFull())
                {
                    int frame_size =frames_ * channels_ * 2;
                    PCMFrame frame(frame_size);
                    memcpy(frame.data.get(), data_buf_, frame_size);

                    frame_buffer_->Push(std::move(frame));
                }
                else
                {
                    printf("frame_buffer is full");
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    running_ = false;
}
