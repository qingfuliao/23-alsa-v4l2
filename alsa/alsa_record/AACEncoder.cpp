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

    // �򿪱�����,����ǳ���Ҫ��ʡ�ڴ�����Ե���encModules
    if ((ret = aacEncOpen(&fdk_aac_handle_, 0x0, channels)) != AACENC_OK)
    {
        printf("Unable to open fdkaac encoder, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        free(fdk_aac_handle_);
        return false;
    }

    // ����AAC��׼��ʽ
    if ((ret = aacEncoder_SetParam(fdk_aac_handle_, AACENC_AOT, profile_aac)) != AACENC_OK) /* aac lc */
    {
        printf("Unable to set the AACENC_AOT, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }
    // ���ò�����
    if ((ret = aacEncoder_SetParam(fdk_aac_handle_, AACENC_SAMPLERATE, sample_rate)) != AACENC_OK)
    {
        printf("Unable to set the SAMPLERATE, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }
    // ����ͨ����
    if ((ret = aacEncoder_SetParam(fdk_aac_handle_, AACENC_CHANNELMODE, channels)) != AACENC_OK)
    {
        printf("Unable to set the AACENC_CHANNELMODE, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }
    // ���ñ�����
    if ((ret = aacEncoder_SetParam(fdk_aac_handle_, AACENC_BITRATE, bit_rate)) != AACENC_OK)
    {
        printf("Unable to set the AACENC_BITRATE, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }
    // ���ñ�����������ݴ�aac adtsͷ
    if ((ret = aacEncoder_SetParam(fdk_aac_handle_, AACENC_TRANSMUX, TT_MP4_ADTS)) != AACENC_OK) // 0-raw 2-adts
    {
        printf("Unable to set the ADTS AACENC_TRANSMUX, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }
    // ��ʼ��������
    if ((ret = aacEncEncode(fdk_aac_handle_, NULL, NULL, NULL, NULL)) != AACENC_OK)
    {
        printf("Unable to initialize the aacEncEncode, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }
    // ��ȡ��������Ϣ
    if ((ret = aacEncInfo(fdk_aac_handle_, &fdk_aac_info_)) != AACENC_OK)
    {
        printf("Unable to get the aacEncInfo info, ret = 0x%x, error is %s\n", ret, fdkaac_error(ret));
        goto faild;
    }

    // ����pcm֡��
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
        return AACENC_UNSUPPORTED_PARAMETER;			// ÿ�ζ���֡�������ݽ��б���
    }

    AACENC_BufDesc  out_buf = { 0 };
    AACENC_InArgs	in_args = { 0 };

    // pcm������������
    in_args.numInSamples = input_len / 2;	// ����ͨ���ļ������Ĳ���������ÿ����������2���ֽ�����/2

                                            // pcm������������
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

    // ���������������
    int	out_identifier = OUT_BITSTREAM_DATA;
    int	out_elem_size = 1;
    void *out_ptr = output;
    int out_buffer_size = output_len;
    out_buf.numBufs = 1;
    out_buf.bufs = &out_ptr;
    out_buf.bufferIdentifiers = &out_identifier;
    out_buf.bufSizes = &out_buffer_size;		//һ��Ҫ���Խ��ս���������
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
    // �ȹرձ�����
    if (aacEncClose(&fdk_aac_handle_) != AACENC_OK)
    {
        printf("aacEncClose failed\n");
    }
    // ��handleָ��NULL
    fdk_aac_handle_ = NULL;
}

int AACEncoder::GetPcmFrameLength()
{
    return pcm_frame_len_;
}
