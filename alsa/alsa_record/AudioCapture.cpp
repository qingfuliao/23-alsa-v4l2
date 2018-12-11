#include "AudioCapture.h"

using namespace std;
AudioCapture::AudioCapture():
    samplerate_(44100),
    channels_(2),
    actual_frames_(1024),
    need_frames_(1024),
    thread_(nullptr),
    running_(false),
    requet_abort_(false)
{
    frame_buffer_.reset(new RingBuffer<PCMFrame>(20));
}

AudioCapture::~AudioCapture()
{
    DeInit();
}

bool AudioCapture::Init(const char *name, uint32_t samplerate,
                        int channels, int frames)
{
    int direct;
    snd_pcm_hw_params_t *param;

    samplerate_ = samplerate;
    channels_ = channels;
    actual_frames_  = frames;
    need_frames_ = frames;  //我需要的是多少个采样点
    printf("snd_pcm_open name = %s, default, frames:%d, channels_:%d, samplerate:%d\n",
           name, actual_frames_, channels_, samplerate_);
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
        is_opened_ = true;
    }
    if (is_opened_)
    {
        printf("snd_pcm_hw_params_alloca!\n");
        /* Allocate a hardware parameters object. */
        /* 分配一个硬件参数结构体 */
        snd_pcm_hw_params_alloca(&param);
        /* Fill it in with default values. */
        /* 使用默认参数 */
        ret = snd_pcm_hw_params_any(alsa_handle_, param);
        if (ret < 0)
        {
            printf("snd_pcm_hw_params_any failed, ret:%s\n", snd_strerror(ret));
            snd_pcm_close(alsa_handle_);
            return false;
        }

        /* Set the desired hardware parameters. */
        /* Interleaved mode */
        ret = snd_pcm_hw_params_set_access(alsa_handle_, param, SND_PCM_ACCESS_RW_INTERLEAVED);
        if (ret < 0)
        {
            printf("snd_pcm_hw_params_set_access failed, ret:%s\n", snd_strerror(ret));
            snd_pcm_close(alsa_handle_);
            return ret;
        }
        /* Signed 16-bit little-endian format */
        /* 16位 小端 */
        ret = snd_pcm_hw_params_set_format(alsa_handle_, param, SND_PCM_FORMAT_S16_LE);
        if (ret < 0)
        {
            printf("snd_pcm_hw_params_set_format failed, ret:%s\n", snd_strerror(ret));
            snd_pcm_close(alsa_handle_);
            return ret;
        }
        /* Two channels (stereo) */
        /* 双通道 */
        ret = snd_pcm_hw_params_set_channels(alsa_handle_, param, channels);
        if (ret < 0)
        {
            printf("snd_pcm_hw_params_set_channels failed, ret:%s\n", channels,
                   snd_strerror(ret));
            snd_pcm_close(alsa_handle_);
            return false;
        }

        uint32_t rrate = samplerate_;
        ret = snd_pcm_hw_params_set_rate_near(alsa_handle_, param, &rrate, &direct);
        if(rrate != samplerate_ || ret < 0)
        {
            printf("snd_pcm_hw_params_set_rate_near failed, samplerate_:%d, rrate:%d, ret:%s\n",
                   samplerate_, rrate, snd_strerror(ret));
            snd_pcm_close(alsa_handle_);
            return false;
        }
        printf("snd_pcm_hw_params_set_period_size_near!\n");
        /* Set period size to 1024 frames. */
        /* 一个周期有 xx 帧, 一帧就是每个通道一个采样点 */
        ret = snd_pcm_hw_params_set_period_size_near(alsa_handle_, param, &actual_frames_, &direct);
        printf("need_frames_:%d, actual_frames_:%d\n", need_frames_, actual_frames_);
        if(ret <0)
        {
            printf("unable snd_pcm_hw_params_set_period_size_near: %s\n",
                    snd_strerror(ret));
            snd_pcm_close(alsa_handle_);
        }

        printf("snd_pcm_hw_params!\n");
        /* Write the parameters to the driver */     /* 参数生效 */
        ret = snd_pcm_hw_params(alsa_handle_, param);
        if(ret < 0)
        {
            printf("snd_pcm_hw_params failed, ret:%s\n", snd_strerror(ret));
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
bool AudioCapture::DeInit()
{
    stop();
    if (is_opened_)
    {
        /*
         * For playback wait for all pending frames to be played and then stop the PCM.
         *  For capture stop PCM permitting to retrieve residual frames.
        */
        snd_pcm_drain(alsa_handle_);
        snd_pcm_close(alsa_handle_);
        is_opened_ = false;
    }
}
bool AudioCapture::start()
{
    if (!thread_)
    {
        thread_.reset(new thread(&AudioCapture::run, this));
    }
    return true;
}
bool AudioCapture::stop()
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
bool AudioCapture::GetFrame(PCMFrame &frame)
{
    if (frame_buffer_->IsEmpty())
    {
        return false;
    }

    return frame_buffer_->Pop(frame);
}

void AudioCapture::run()
{
    uint8_t *actual_frames_buf_ = nullptr;
    int32_t actual_frames_buf_size_ = actual_frames_ * channels_ * 2; // frames * 4;  2 bytes/sample, 2 channels
    actual_frames_buf_ = new  uint8_t[actual_frames_buf_size_];
    printf("actual_frames_buf_size_= %d\n", actual_frames_buf_size_);

    uint8_t *need_frames_buf_ = nullptr;
    int32_t need_frames_buf_size_ = need_frames_ * channels_ * 2; // frames * 4;  2 bytes/sample, 2 channels
    need_frames_buf_ = new  uint8_t[need_frames_buf_size_];
    printf("need_frames_buf_size_= %d\n", need_frames_buf_size_);

    int need_buf_pos = 0;

    printf("AudioCapture::Run\n");
    running_ = true;
    while (true)
    {
        if (requet_abort_)
        {
            break;
        }

        int ret;
        for (int i = 0; i != 5; ++i)
        {
            //printf("snd_pcm_readi::Run:%d\n",actual_frames_);
            ret = snd_pcm_readi(alsa_handle_, actual_frames_buf_, actual_frames_);
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
                int actual_buf_pos = 0;
                for(; ret > 0; )
                {
                    // maybe read incompletely
                    // 插入一帧数据
                    if(need_buf_pos + ret <= need_frames_)
                    {
                        memcpy(&need_frames_buf_[need_buf_pos * 4], &actual_frames_buf_[actual_buf_pos], ret * 4);    // 拷贝数据
                        need_buf_pos += ret;
                        actual_buf_pos += ret * 4;
                        ret = 0;
                        if(need_buf_pos == need_frames_)
                        {
                            need_buf_pos = 0;
                            pushFrame(need_frames_buf_, need_frames_buf_size_);
                        }
                    }
                    else
                    {
                        memcpy(&need_frames_buf_[need_buf_pos * 4], &actual_frames_buf_[actual_buf_pos], (need_frames_ - need_buf_pos) * 4);  // 拷贝数据
                        ret -= (need_frames_ - need_buf_pos);
                        actual_buf_pos += (need_frames_ - need_buf_pos) * 4;

                        pushFrame(need_frames_buf_, need_frames_buf_size_); // 发送一帧
                        need_buf_pos = 0;
                    }
                }
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    running_ = false;
}

bool AudioCapture::pushFrame(uint8_t *data, int size)
{
    if (!frame_buffer_->IsFull())
    {
        PCMFrame frame(size);
        memcpy(frame.data.get(), data, size);
        frame_buffer_->Push(std::move(frame));
    }
    else
    {
        printf("frame_buffer is full");
    }
}

