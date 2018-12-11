#ifndef AACENCODER_H
#define AACENCODER_H

#include <stdint.h>
#include <fdk-aac/aacenc_lib.h>
#include <fdk-aac/FDK_audio.h>

// 对应
#define PROFILE_AAC_LC		2				// AOT_AAC_LC
#define PROFILE_AAC_HE		5				// AOT_SBR
#define PROFILE_AAC_HE_v2	29				// AOT_PS PS, Parametric Stereo (includes SBR)
#define PROFILE_AAC_LD		23				// AOT_ER_AAC_LD Error Resilient(ER) AAC LowDelay object
#define PROFILE_AAC_ELD		39				// AOT_ER_AAC_ELD AAC Enhanced Low Delay

class AACEncoder
{
public:
    AACEncoder();
    bool Init(const int sample_rate, const int channels, const int bit_rate, const int profile_aac);
    int Encode(const uint8_t * input, const int input_len, uint8_t * output, int &output_len);
    int GetPcmFrameLength();
    void DeInit();

private:
    HANDLE_AACENCODER fdk_aac_handle_;		// fdk-aac
    AACENC_InfoStruct fdk_aac_info_;		// fdk-aac
    int pcm_frame_len_;                     // 每次送pcm的字节数
};

#endif // AACENCODE_H
