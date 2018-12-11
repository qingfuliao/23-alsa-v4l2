#ifndef V4L2_H
#define V4L2_H
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// �豸�ڵ���������ֽ�
#define MAX_DEV_NAME 32

// ���建��Ľṹ��
typedef struct VideoBuffer
{
    uint8_t *start; // �ڴ�ӳ�����ʼ��ַ
    size_t offset;  // ƫ�Ƶ�ַ
    size_t length;  // ͼ���С
}VideoBuffer;

typedef struct v4l2_param
{
    char devname[MAX_DEV_NAME];//�豸��
    int fd;                     //��������
    VideoBuffer *buffers;
    struct v4l2_requestbuffers req;
    struct v4l2_capability cap;
    struct v4l2_input input;
    struct v4l2_format fmt;
    struct v4l2_buffer buf;
}V4L2_VIDEO_T;

int V4l2OpenDeive(V4L2_VIDEO_T *video_obj, const char *const devname, int block);
int V4l2Querycapture(V4L2_VIDEO_T *video_obj);
int V4l2SetParam(V4L2_VIDEO_T *video_obj, uint32_t num,uint32_t deno);
int V4l2GetParam(V4L2_VIDEO_T *video_obj);
int V4l2EnumFormat(V4L2_VIDEO_T *video_obj);
int V4l2SetFormat(V4L2_VIDEO_T *video_obj, uint32_t format,
                 uint32_t width,uint32_t height);
int V4l2RequestBuffer(V4L2_VIDEO_T *video_obj, uint32_t count);
int V4l2MmapBuffer(V4L2_VIDEO_T *video_obj);
int V4l2StartCapture(V4L2_VIDEO_T *video_obj);
int V4l2StopCapture(V4L2_VIDEO_T *video_obj);
int V4l2PullFrameBuffer(V4L2_VIDEO_T *video_obj, uint32_t index ,
                           unsigned char **start , uint32_t *len);
int V4l2PushFrameBuffer(V4L2_VIDEO_T *video_obj, uint32_t index);
void V4l2ReleaseBuffer(V4L2_VIDEO_T *video_obj);
void V4l2CloseDevice(V4L2_VIDEO_T *video_obj);
#ifdef __cplusplus
}
#endif
#endif // V4L2_H
