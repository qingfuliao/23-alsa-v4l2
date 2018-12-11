#include "AACEncoder.h"

#include <stdlib.h>
#include <stdio.h>

static const char *fdkaac_error(AACENC_ERROR erraac)
{
    switch (erraac)
    {
        case AACENC_OK: return "No error";
        case AACENC_INVALID_HANDLE: return "Invalid handle";
        case AACENC_MEMORY_ERROR: return "Memory allocation error";
        case AACENC_UNSUPPORTED_PARAMETER: return "Unsupported parameter";
        case AACENC_INVALID_CONFIG: return "Invalid config";
        case AACENC_INIT_ERROR: return "Initialization error";
        case AACENC_INIT_AAC_ERROR: return "AAC library initialization error";
        case AACENC_INIT_SBR_ERROR: return "SBR library initialization error";
        case AACENC_INIT_TP_ERROR: return "Transport library initialization error";
        case AACENC_INIT_META_ERROR: return "Metadata library initialization error";
        case AACENC_ENCODE_ERROR: return "Encoding error";
        case AACENC_ENCODE_EOF: return "End of file";
        default: return "Unknown error";
    }
}

AACEncoder::AACEncoder():
    fdk_aac_handle_(NULL), pcm_frame_len_(0)
{
}

bool AACEncoder::Init(const int sample_rate, const int channels, const int bit_rate, const int profile_aac)
{
    AACENC_ERROR ret;

    // 打开编码器,如果非常需要节省内存则可以调整encModules
    if ((ret = aacEncOpen(&fdk_aac_handle_, 0x0, channels)) != AACENC_OK)
    {
        printf("Unable to open fdkaac encoder, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        free(fdk_aac_handle_);
        return false;
    }

    // 设置AAC标准格式
    if ((ret = aacEncoder_SetParam(fdk_aac_handle_, AACENC_AOT, profile_aac)) != AACENC_OK) /* aac lc */
    {
        printf("Unable to set the AACENC_AOT, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }
    // 设置采样率
    if ((ret = aacEncoder_SetParam(fdk_aac_handle_, AACENC_SAMPLERATE, sample_rate)) != AACENC_OK)
    {
        printf("Unable to set the SAMPLERATE, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }
    // 设置通道数
    if ((ret = aacEncoder_SetParam(fdk_aac_handle_, AACENC_CHANNELMODE, channels)) != AACENC_OK)
    {
        printf("Unable to set the AACENC_CHANNELMODE, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }
    // 设置比特率
    if ((ret = aacEncoder_SetParam(fdk_aac_handle_, AACENC_BITRATE, bit_rate)) != AACENC_OK)
    {
        printf("Unable to set the AACENC_BITRATE, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }
    // 设置编码出来的数据带aac adts头
    if ((ret = aacEncoder_SetParam(fdk_aac_handle_, AACENC_TRANSMUX, TT_MP4_ADTS)) != AACENC_OK) // 0-raw 2-adts
    {
        printf("Unable to set the ADTS AACENC_TRANSMUX, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }
    // 初始化编码器
    if ((ret = aacEncEncode(fdk_aac_handle_, NULL, NULL, NULL, NULL)) != AACENC_OK)
    {
        printf("Unable to initialize the aacEncEncode, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }
    // 获取编码器信息
    if ((ret = aacEncInfo(fdk_aac_handle_, &fdk_aac_info_)) != AACENC_OK)
    {
        printf("Unable to get the aacEncInfo info, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }

    // 计算pcm帧长
    pcm_frame_len_ = fdk_aac_info_.inputChannels * fdk_aac_info_.frameLength * 2;

    printf("AACEncoderInit   channels = %d, pcm_frame_len = %d\n",
        fdk_aac_info_.inputChannels, pcm_frame_len_);

    return  true;
faild:
    if (fdk_aac_handle_ && fdk_aac_handle_)
    {
        if (aacEncClose(&fdk_aac_handle_) != AACENC_OK)
        {
            printf("aacEncClose failed\n");
        }
    }
    if (fdk_aac_handle_)
        free(fdk_aac_handle_);

    return false;
}

int AACEncoder::Encode(const uint8_t * input, const int input_len, uint8_t * output, int &output_len)
{
    AACENC_ERROR ret;

    if (!fdk_aac_handle_)
    {
        return AACENC_INVALID_HANDLE;
    }

    if (input_len != pcm_frame_len_)
    {
        printf("input_len = %d no equal to need length = %d\n", input_len, pcm_frame_len_);
        return AACENC_UNSUPPORTED_PARAMETER;			// 每次都按帧长的数据进行编码
    }

    AACENC_BufDesc  out_buf = { 0 };
    AACENC_InArgs	in_args = { 0 };

    // pcm数据输入配置
    in_args.numInSamples = input_len / 2;	// 所有通道的加起来的采样点数，每个采样点是2个字节所以/2

                                            // pcm数据输入配置
    int		in_identifier = IN_AUDIO_DATA;
    int		in_elem_size = 2;
    void	*in_ptr = (void	*)input;
    int		in_buffer_size = input_len;
    AACENC_BufDesc	in_buf = { 0 };
    in_buf.numBufs = 1;
    in_buf.bufs = &in_ptr;
    in_buf.bufferIdentifiers = &in_identifier;
    in_buf.bufSizes = &in_buffer_size;
    in_buf.bufElSizes = &in_elem_size;

    // 编码数据输出配置
    int	out_identifier = OUT_BITSTREAM_DATA;
    int	out_elem_size = 1;
    void *out_ptr = output;
    int out_buffer_size = output_len;
    out_buf.numBufs = 1;
    out_buf.bufs = &out_ptr;
    out_buf.bufferIdentifiers = &out_identifier;
    out_buf.bufSizes = &out_buffer_size;		//一定要可以接收解码后的数据
    out_buf.bufElSizes = &out_elem_size;

    AACENC_OutArgs	out_args = { 0 };

    if ((ret = aacEncEncode(fdk_aac_handle_, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK)
    {
        printf("aacEncEncode ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));

        return ret;
    }
    output_len = out_args.numOutBytes;

    return AACENC_OK;
}

void AACEncoder::DeInit()
{
    // 先关闭编码器
    if (aacEncClose(&fdk_aac_handle_) != AACENC_OK)
    {
        printf("aacEncClose failed\n");
    }
    // 将handle指向NULL
    fdk_aac_handle_ = NULL;
}

int AACEncoder::GetPcmFrameLength()
{
    return pcm_frame_len_;
}
