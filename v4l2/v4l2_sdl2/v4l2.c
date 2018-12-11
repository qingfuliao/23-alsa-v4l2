#include "v4l2.h"
#include<sys/ioctl.h>

/*
����Ƶ�豸�ǳ��򵥣���V4L2�У���Ƶ�豸������һ���ļ���ʹ��open����������豸��

1. �÷�����ģʽ������ͷ�豸
int fd;
fd = open("/dev/video0", O_RDWR | O_NONBLOCK);

2. ���������ģʽ������ͷ�豸�����������Ϊ��
fd = open("/dev/video0", O_RDWR);

��������ģʽ�ͷ�����ģʽ

Ӧ�ó����ܹ�ʹ������ģʽ�������ģʽ����Ƶ�豸�����ʹ�÷�����ģʽ������Ƶ�豸��
��ʹ��δ������Ϣ���������ɻ�ѻ���(DQBUFF)��Ķ������ظ�Ӧ�ó���
*/
int V4l2OpenDeive(V4L2_VIDEO_T *video_obj, const char *const devname, int block)
{
    printf("V4l2OpenDeive devname = %s\n", devname);
    //���豸
    if(strlen(devname) >= MAX_DEV_NAME)
    {
        printf("device name fail:%s\n", devname);
        return -1;
    }
    else
        memset(video_obj, 0, sizeof(video_obj));

    int flag = O_RDWR;
    if(block == 0)
    {
        flag |= O_NONBLOCK;
    }

    video_obj->fd = open(devname, flag);

    if(video_obj->fd < 0)
    {
        printf("open %s fail", devname);
        return -1;
    }
    else
        printf("%s success\n", __func__);

    strncpy(video_obj->devname, devname, MAX_DEV_NAME - 1);
    video_obj->devname[MAX_DEV_NAME - 1] = '\0';
    printf("V4l2OpenDeive devname = %s\n", video_obj->devname);

    return 0;
}

/*
�������VIDIOC_QUERYCAP

���ܣ� ��ѯ��Ƶ�豸�Ĺ��� ;

����˵������������ΪV4L2��������������struct v4l2_capability ;

����ֵ˵���� ִ�гɹ�ʱ����������ֵΪ 0;����ִ�гɹ���
    struct v4l2_capability �ṹ������еķ��ص�ǰ��Ƶ�豸��֧�ֵĹ���;
    ����֧����Ƶ������V4L2_CAP_VIDEO_CAPTURE��V4L2_CAP_STREAMING�ȡ�
*/
int V4l2Querycapture(V4L2_VIDEO_T *video_obj)
{
    //��ѯ�豸����
    if (ioctl(video_obj->fd, VIDIOC_QUERYCAP, &video_obj->cap) == -1)
    {
        printf("Error opening device %s: unable to query device.\n", video_obj->devname);
        return -1;
    }
    else
    {
        printf("driver:\t\t%s\n", video_obj->cap.driver);
        printf("card:\t\t%s\n", video_obj->cap.card);
        printf("bus_info:\t%s\n", video_obj->cap.bus_info);
        printf("version:\t%d\n", video_obj->cap.version);
        printf("capabilities:\t%x\n", video_obj->cap.capabilities);

        if ((video_obj->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE)
        {
            printf("Device %s: supports capture.\n", video_obj->devname);
        }

        if ((video_obj->cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING)
        {
            printf("Device %s: supports streaming.\n", video_obj->devname);
        }
    }

    return 0;
}

/*
�������VIDIOC_S_PARM

���ܣ� ���ò��� ;

����˵������������ΪV4L2��������������struct v4l2_streamparm ;

����ֵ˵���� ִ�гɹ�ʱ����������ֵΪ 0;����ִ�гɹ���
    struct v4l2_streamparm �ṹ������б��������õ��ײ㡣
*/
int V4l2SetParam(V4L2_VIDEO_T *video_obj, uint32_t num, uint32_t deno)
{
    struct v4l2_streamparm param;
    memset(&param, 0, sizeof(struct v4l2_streamparm));
    param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    param.parm.capture.timeperframe.numerator = num;    // ����
    param.parm.capture.timeperframe.denominator = deno; // ��ĸ
    if(ioctl(video_obj->fd, VIDIOC_S_PARM, &param) < 0)
    {
        printf("%s fail\n", __func__);
        return -1;
    }
    else
    {
        printf("%s ok\n", __func__);
        return 0;
    }
    printf("V4l2SetParam numerator:%d, denominator:%d\n",   \
           param.parm.capture.timeperframe.numerator,       \
           param.parm.capture.timeperframe.denominator);
}

/*
�������VIDIOC_G_PARM

���ܣ� ���ò��� ;

����˵������������ΪV4L2��������������struct v4l2_streamparm ;

����ֵ˵���� ִ�гɹ�ʱ����������ֵΪ 0;����ִ�гɹ���
    ��ȡstruct v4l2_streamparm ������
*/
int V4l2GetParam(V4L2_VIDEO_T *video_obj)
{
    struct v4l2_streamparm param;
    memset(&param, 0, sizeof(struct v4l2_streamparm));
    param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(video_obj->fd, VIDIOC_G_PARM, &param) < 0)
    {
        perror("get param failed");
        return -1;
    }
    else
    {
        printf("%s, numerator:%d, denominator:%d\n", __func__,
               param.parm.capture.timeperframe.numerator,
               param.parm.capture.timeperframe.denominator);
        return 0;
    }
}
/*
�������� VIDIOC_ENUM_FMT

���ܣ� ��ȡ��ǰ��Ƶ�豸֧�ֵ���Ƶ��ʽ ��

����˵������������ΪV4L2����Ƶ��ʽ���������� struct v4l2_fmtdesc

����ֵ˵���� ִ�гɹ�ʱ����������ֵΪ 0��
    struct v4l2_fmtdesc �ṹ���е� .pixelformat�� .description ��Ա���ص�ǰ��Ƶ�豸��֧�ֵ���Ƶ��ʽ��
*/
int V4l2EnumFormat(V4L2_VIDEO_T *video_obj)
{

    struct v4l2_format fmt;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ioctl(video_obj->fd, VIDIOC_G_FMT, &fmt);
    printf("Currentdata format information:width:%d,height:%d\n",
           fmt.fmt.pix.width, fmt.fmt.pix.height);
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while(ioctl(video_obj->fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1)
    {
        printf("format:%s, 0x%x\n", fmtdesc.description, fmtdesc.pixelformat);
        fmtdesc.index++;
    }
    return 0;
}
/*
�������VIDIOC_S_FMT

���ܣ� ������Ƶ�豸����Ƶ���ݸ�ʽ������������Ƶͼ�����ݵĳ�����ͼ���ʽ(JPEG��YUYV��ʽ);

����˵������������ΪV4L2����Ƶ���ݸ�ʽ���� struct v4l2_format ;

����ֵ˵����ִ�гɹ�ʱ����������ֵΪ 0;
 */
int V4l2SetFormat(V4L2_VIDEO_T *video_obj, uint32_t format,
                  uint32_t width, uint32_t height)
{
    printf("V4l2SetFormat format:0x%x\n", format);
    //������Ƶ��ʽ
    memset(&video_obj->fmt, 0, sizeof(video_obj->fmt));
    //��Ƶ���������ͣ���Զ����V4L2_BUF_TYPE_VIDEO_CAPTURE
    video_obj->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //��ƵԴ�ĸ�ʽΪJPEG��YUN4:2:2��RGB  V4L2_PIX_FMT_RGB565  V4L2_PIX_FMT_YUV565
    video_obj->fmt.fmt.pix.pixelformat = format;
    //������Ƶ���
    video_obj->fmt.fmt.pix.width = width;
    //������Ƶ�߶�
    video_obj->fmt.fmt.pix.height = height;
    printf("VIDIOC_S_FMT foramt:%x\n", video_obj->fmt.fmt.pix.pixelformat);
    if (ioctl(video_obj->fd, VIDIOC_S_FMT, &video_obj->fmt) < 0)//ʹ������Ч
    {
        perror("set format failed");
        return -1;
    }
    else
    {
        printf("%s V4L2_PIX_FMT_YUYV:%x, V4L2_PIX_FMT_MJPEG:%x\n", __FUNCTION__,
               V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG);
        printf("%s set success[format:%X w:%d h:%d]\n",
               __func__, video_obj->fmt.fmt.pix.pixelformat,
               video_obj->fmt.fmt.pix.width,
               video_obj->fmt.fmt.pix.height);
    }

    struct v4l2_format fmt;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(video_obj->fd, VIDIOC_G_FMT, &fmt) < 0)
    {
        perror("set format failed");
        return -1;
    }
    else
        printf("%s get success[format:%X w:%d h:%d]\n",
               __func__, fmt.fmt.pix.pixelformat, fmt.fmt.pix.width, fmt.fmt.pix.height);
    return 0;
}
/*
�������VIDIOC_REQBUFS

���ܣ� ����V4L2����������Ƶ������(����V4L2��Ƶ���������ڴ�)��V4L2����Ƶ�豸�������㣬λ���ں˿ռ䣬
    ����ͨ��VIDIOC_REQBUFS����������������ڴ�λ���ں˿ռ䣬Ӧ�ó�����ֱ�ӷ��ʣ�
    ��Ҫͨ������mmap�ڴ�ӳ�亯�����ں˿ռ��ڴ�ӳ�䵽�û��ռ��Ӧ�ó���ͨ�������û��ռ��ַ�������ں˿ռ䡣

����˵������������ΪV4L2�����뻺�������ݽṹ������struct v4l2_requestbuffers ;

����ֵ˵���� ִ�гɹ�ʱ����������ֵΪ 0;V4L2��������������Ƶ������;
*/
int V4l2RequestBuffer(V4L2_VIDEO_T *video_obj, uint32_t count)
{
    //����֡����
    video_obj->buffers = (VideoBuffer *)calloc(count, sizeof(VideoBuffer));
    memset(&video_obj->req, 0, sizeof(video_obj->req));

    //�������������ɱ����ͼƬ����
    video_obj->req.count = count;
    //���������ͣ���Զ����V4L2_BUF_TYPE_VIDEO_CAPTURE
    video_obj->req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //�洢���ͣ�V4L2_MEMORY_MMAP��V4L2_MEMORY_USERPTR
    video_obj->req.memory = V4L2_MEMORY_MMAP;
    //ʹ������Ч
    if (ioctl(video_obj->fd, VIDIOC_REQBUFS, &video_obj->req) == -1)
    {
        perror("request buffer error \n");
        return -1;
    }
    else
        printf("%s success[request %d buffers]\n", __func__, count);
    return 0;
}

/*
�������VIDIOC_QUERYBUF

���ܣ� ��ѯ�Ѿ������V4L2����Ƶ�������������Ϣ��������Ƶ��������ʹ��״̬�����ں˿ռ��ƫ�Ƶ�ַ�����������ȵȡ�
    ��Ӧ�ó��������ͨ����VIDIOC_QUERYBUF����ȡ�ں˿ռ����Ƶ��������Ϣ��Ȼ����ú���mmap���ں˿ռ��ַӳ��
    ���û��ռ䣬����Ӧ�ó�����ܹ�����λ���ں˿ռ����Ƶ��������

����˵������������ΪV4L2���������ݽṹ���� struct v4l2_buffer ;

����ֵ˵���� ִ�гɹ�ʱ����������ֵΪ 0;struct v4l2_buffer�ṹ������б�����ָ��Ļ������������Ϣ;

һ������£�Ӧ�ó����е���VIDIOC_QUERYBUFȡ�����ں˻�������Ϣ�󣬽����ŵ���mmap�������ں˿ռ��ַӳ�䵽
    �û��ռ䣬�����û��ռ�Ӧ�ó���ķ��ʡ�
*/
int V4l2MmapBuffer(V4L2_VIDEO_T *video_obj)
{
    //��VIDIOC_REQBUFS��ȡ�ڴ�תΪ����ռ�
    int numBufs;
    for (numBufs = 0; numBufs < video_obj->req.count; numBufs++)
    {
        memset(&video_obj->buf, 0, sizeof(video_obj->buf));
        //���������ͣ���Զ����V4L2_BUF_TYPE_VIDEO_CAPTURE
        video_obj->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        //�洢���ͣ�V4L2_MEMORY_MMAP���ڴ�ӳ�䣩��V4L2_MEMORY_USERPTR���û�ָ�룩
        video_obj->buf.memory = V4L2_MEMORY_MMAP;
        video_obj->buf.index = numBufs;
        //ʹ������Ч
        if (ioctl(video_obj->fd, VIDIOC_QUERYBUF, &video_obj->buf) < 0)
        {
            perror("VIDIOC_QUERYBUF");
            return -1;
        }
        //printf("request buf %d success\n",numBufs);
        // VideoBuffer buffers�������Լ�д�Ľṹ��
        video_obj->buffers[numBufs].length = video_obj->buf.length;
        video_obj->buffers[numBufs].offset = (size_t)video_obj->buf.m.offset;
        //ʹ��mmap����������Ļ����ַת��Ӧ�ó���ľ��Ե�ַ
        video_obj->buffers[numBufs].start = (uint8_t *)mmap(NULL, video_obj->buf.length,
                                            PROT_READ | PROT_WRITE, MAP_SHARED, video_obj->fd, video_obj->buf.m.offset);
        if (video_obj->buffers[numBufs].start == MAP_FAILED)
        {
            perror("buffers error");
            return -1;
        }
        //printf("mmap buf 0x%p lenght:%d success\n",video_obj->buffers[numBufs].start,video_obj->buf.length);
        //���뻺�����
        if (ioctl(video_obj->fd, VIDIOC_QBUF, &video_obj->buf) < 0)
        {
            printf("VIDIOC_QBUF");
            return -1;
        }
    }
    printf("%s success\n", __func__);
    return 0;
}

/**
���ܣ� ������Ƶ�ɼ����Ӧ�ó������VIDIOC_STREAMON������Ƶ�ɼ������
    ��Ƶ�豸��������ʼ�ɼ���Ƶ���ݣ����Ѳɼ�������Ƶ���ݱ��浽��Ƶ��������Ƶ�������С�

����˵������������ΪV4L2����Ƶ���������� enum v4l2_buf_type ��

����ֵ˵���� ִ�гɹ�ʱ����������ֵΪ 0��
 */
int V4l2StartCapture(V4L2_VIDEO_T *video_obj)
{
    //��ʼ��Ƶ��ʾ
    enum v4l2_buf_type type;
    //���������ͣ���Զ����V4L2_BUF_TYPE_VIDEO_CAPTURE
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(video_obj->fd, VIDIOC_STREAMON, &type) < 0)
    {
        perror("VIDIOC_STREAMON");
        return -1;
    }
    printf("%s stream on success\n", __func__);
    return 0;
}

/**
���ܣ� ֹͣ��Ƶ�ɼ����Ӧ�ó������VIDIOC_ STREAMOFFֹͣ��Ƶ�ɼ��������Ƶ�豸���������ڲɼ���Ƶ���ݡ�
����˵������������ΪV4L2����Ƶ���������� enum v4l2_buf_type ��
����ֵ˵���� ִ�гɹ�ʱ����������ֵΪ 0������ִ�гɹ�����Ƶ�豸ֹͣ�ɼ���Ƶ���ݡ�
 */
int V4l2StopCapture(V4L2_VIDEO_T *video_obj)
{
    //��ʼ��Ƶ��ʾ
    enum v4l2_buf_type type;
    //���������ͣ���Զ����V4L2_BUF_TYPE_VIDEO_CAPTURE
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(video_obj->fd, VIDIOC_STREAMOFF, &type) < 0)
    {
        perror("VIDIOC_STREAMON");
        return -1;
    }
    printf("%s stream on success\n", __func__);
    return 0;
}

/**
��   ��:   ����Ƶ�����������������ȡ��һ���Ѿ�������һ֡��Ƶ���ݵ���Ƶ��������
����˵��:��������ΪV4L2���������ݽṹ���� struct v4l2_buffer ��
����ֵ˵���� ִ�гɹ�ʱ����������ֵΪ 0������ִ�гɹ�����Ӧ���ں���Ƶ��������
    �����е�ǰ���㵽����Ƶ���ݣ�Ӧ�ó������ͨ�������û��ռ�����ȡ����Ƶ���ݡ�
    ��ǰ���Ѿ�ͨ�����ú���mmap�����û��ռ���ں˿ռ���ڴ�ӳ�䣩.
 */
int V4l2PullFrameBuffer(V4L2_VIDEO_T *video_obj, uint32_t index,
                        unsigned char **start, uint32_t *len)
{
    video_obj->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;	//ȡ��ԭʼ�ɼ�����
    video_obj->buf.memory = V4L2_MEMORY_MMAP;			//�洢���ͣ�V4L2_MEMORY_MMAP���ڴ�ӳ�䣩��V4L2_MEMORY_USERPTR���û�ָ�룩
    if(video_obj->req.count <= index)
        return -1;
    video_obj->buf.index = index;						//��ȡ�����еĵڼ�֡
//    printf("%s VIDIOC_DQBUF into\n", __func__);
    if (ioctl(video_obj->fd, VIDIOC_DQBUF, &video_obj->buf) < 0)
    {
        perror("VIDIOC_DQBUF");
        return -1;
    }
//    printf("%s VIDIOC_DQBUF leave\n", __func__);
    *start = video_obj->buffers[index].start;
    *len = video_obj->buffers[index].length;
    return 0;
}

/*
�������VIDIOC_QBUF

���ܣ� Ͷ��һ���յ���Ƶ����������Ƶ��������������� ;

����˵������������ΪV4L2���������ݽṹ���� struct v4l2_buffer ;

����ֵ˵���� ִ�гɹ�ʱ����������ֵΪ 0;����ִ�гɹ���ָ��(ָ��)����Ƶ������������Ƶ������У�
    ��������Ƶ�豸����ͼ��ʱ����Ӧ����Ƶ���ݱ����浽��Ƶ���������Ӧ����Ƶ�������С�

*/
int V4l2PushFrameBuffer(V4L2_VIDEO_T *video_obj, uint32_t index)
{
    video_obj->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;	//ȡ��ԭʼ�ɼ�����
    video_obj->buf.memory = V4L2_MEMORY_MMAP;			//�洢���ͣ�V4L2_MEMORY_MMAP���ڴ�ӳ�䣩��V4L2_MEMORY_USERPTR���û�ָ�룩
    if(video_obj->req.count <= index)
        return -1;
    video_obj->buf.index = index;						//�ڼ�֡���뻺��
    //��ȡ��һ֡��Ƶ����
    if (ioctl(video_obj->fd, VIDIOC_QBUF, &video_obj->buf) < 0)
    {
        perror("VIDIOC_QBUF");
        return -1;
    }
    return 0;
}

void V4l2ReleaseBuffer(V4L2_VIDEO_T *video_obj)
{
    int numBufs;
    for (numBufs = 0; numBufs < video_obj->req.count; numBufs++)
        munmap(video_obj->buffers[numBufs].start, video_obj->buf.length);
    free(video_obj->buffers);
    printf("%s\n", __func__);
}

void V4l2CloseDevice(V4L2_VIDEO_T *video_obj)
{
    close(video_obj->fd);
    printf("%s\n", __func__);
}
