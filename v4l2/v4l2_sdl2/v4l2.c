#include "v4l2.h"
#include<sys/ioctl.h>

/*
打开视频设备非常简单，在V4L2中，视频设备被看做一个文件。使用open函数打开这个设备：

1. 用非阻塞模式打开摄像头设备
int fd;
fd = open("/dev/video0", O_RDWR | O_NONBLOCK);

2. 如果用阻塞模式打开摄像头设备，上述代码变为：
fd = open("/dev/video0", O_RDWR);

关于阻塞模式和非阻塞模式

应用程序能够使用阻塞模式或非阻塞模式打开视频设备，如果使用非阻塞模式调用视频设备，
即使尚未捕获到信息，驱动依旧会把缓存(DQBUFF)里的东西返回给应用程序。
*/
int V4l2OpenDeive(V4L2_VIDEO_T *video_obj, const char *const devname, int block)
{
    printf("V4l2OpenDeive devname = %s\n", devname);
    //打开设备
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
控制命令：VIDIOC_QUERYCAP

功能： 查询视频设备的功能 ;

参数说明：参数类型为V4L2的能力描述类型struct v4l2_capability ;

返回值说明： 执行成功时，函数返回值为 0;函数执行成功后，
    struct v4l2_capability 结构体变量中的返回当前视频设备所支持的功能;
    例如支持视频捕获功能V4L2_CAP_VIDEO_CAPTURE、V4L2_CAP_STREAMING等。
*/
int V4l2Querycapture(V4L2_VIDEO_T *video_obj)
{
    //查询设备属性
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
控制命令：VIDIOC_S_PARM

功能： 设置参数 ;

参数说明：参数类型为V4L2的能力描述类型struct v4l2_streamparm ;

返回值说明： 执行成功时，函数返回值为 0;函数执行成功后，
    struct v4l2_streamparm 结构体变量中变量被设置到底层。
*/
int V4l2SetParam(V4L2_VIDEO_T *video_obj, uint32_t num, uint32_t deno)
{
    struct v4l2_streamparm param;
    memset(&param, 0, sizeof(struct v4l2_streamparm));
    param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    param.parm.capture.timeperframe.numerator = num;    // 分子
    param.parm.capture.timeperframe.denominator = deno; // 分母
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
控制命令：VIDIOC_G_PARM

功能： 设置参数 ;

参数说明：参数类型为V4L2的能力描述类型struct v4l2_streamparm ;

返回值说明： 执行成功时，函数返回值为 0;函数执行成功后，
    获取struct v4l2_streamparm 参数。
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
控制命令 VIDIOC_ENUM_FMT

功能： 获取当前视频设备支持的视频格式 。

参数说明：参数类型为V4L2的视频格式描述符类型 struct v4l2_fmtdesc

返回值说明： 执行成功时，函数返回值为 0；
    struct v4l2_fmtdesc 结构体中的 .pixelformat和 .description 成员返回当前视频设备所支持的视频格式；
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
控制命令：VIDIOC_S_FMT

功能： 设置视频设备的视频数据格式，例如设置视频图像数据的长、宽，图像格式(JPEG、YUYV格式);

参数说明：参数类型为V4L2的视频数据格式类型 struct v4l2_format ;

返回值说明：执行成功时，函数返回值为 0;
 */
int V4l2SetFormat(V4L2_VIDEO_T *video_obj, uint32_t format,
                  uint32_t width, uint32_t height)
{
    printf("V4l2SetFormat format:0x%x\n", format);
    //设置视频格式
    memset(&video_obj->fmt, 0, sizeof(video_obj->fmt));
    //视频数据流类型，永远都是V4L2_BUF_TYPE_VIDEO_CAPTURE
    video_obj->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //视频源的格式为JPEG或YUN4:2:2或RGB  V4L2_PIX_FMT_RGB565  V4L2_PIX_FMT_YUV565
    video_obj->fmt.fmt.pix.pixelformat = format;
    //设置视频宽度
    video_obj->fmt.fmt.pix.width = width;
    //设置视频高度
    video_obj->fmt.fmt.pix.height = height;
    printf("VIDIOC_S_FMT foramt:%x\n", video_obj->fmt.fmt.pix.pixelformat);
    if (ioctl(video_obj->fd, VIDIOC_S_FMT, &video_obj->fmt) < 0)//使配置生效
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
控制命令：VIDIOC_REQBUFS

功能： 请求V4L2驱动分配视频缓冲区(申请V4L2视频驱动分配内存)，V4L2是视频设备的驱动层，位于内核空间，
    所以通过VIDIOC_REQBUFS控制命令字申请的内存位于内核空间，应用程序不能直接访问，
    需要通过调用mmap内存映射函数把内核空间内存映射到用户空间后，应用程序通过访问用户空间地址来访问内核空间。

参数说明：参数类型为V4L2的申请缓冲区数据结构体类型struct v4l2_requestbuffers ;

返回值说明： 执行成功时，函数返回值为 0;V4L2驱动层分配好了视频缓冲区;
*/
int V4l2RequestBuffer(V4L2_VIDEO_T *video_obj, uint32_t count)
{
    //申请帧缓冲
    video_obj->buffers = (VideoBuffer *)calloc(count, sizeof(VideoBuffer));
    memset(&video_obj->req, 0, sizeof(video_obj->req));

    //缓存数量，即可保存的图片数量
    video_obj->req.count = count;
    //数据流类型，永远都是V4L2_BUF_TYPE_VIDEO_CAPTURE
    video_obj->req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //存储类型：V4L2_MEMORY_MMAP或V4L2_MEMORY_USERPTR
    video_obj->req.memory = V4L2_MEMORY_MMAP;
    //使配置生效
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
控制命令：VIDIOC_QUERYBUF

功能： 查询已经分配的V4L2的视频缓冲区的相关信息，包括视频缓冲区的使用状态、在内核空间的偏移地址、缓冲区长度等。
    在应用程序设计中通过调VIDIOC_QUERYBUF来获取内核空间的视频缓冲区信息，然后调用函数mmap把内核空间地址映射
    到用户空间，这样应用程序才能够访问位于内核空间的视频缓冲区。

参数说明：参数类型为V4L2缓冲区数据结构类型 struct v4l2_buffer ;

返回值说明： 执行成功时，函数返回值为 0;struct v4l2_buffer结构体变量中保存了指令的缓冲区的相关信息;

一般情况下，应用程序中调用VIDIOC_QUERYBUF取得了内核缓冲区信息后，紧接着调用mmap函数把内核空间地址映射到
    用户空间，方便用户空间应用程序的访问。
*/
int V4l2MmapBuffer(V4L2_VIDEO_T *video_obj)
{
    //将VIDIOC_REQBUFS获取内存转为物理空间
    int numBufs;
    for (numBufs = 0; numBufs < video_obj->req.count; numBufs++)
    {
        memset(&video_obj->buf, 0, sizeof(video_obj->buf));
        //数据流类型，永远都是V4L2_BUF_TYPE_VIDEO_CAPTURE
        video_obj->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        //存储类型：V4L2_MEMORY_MMAP（内存映射）或V4L2_MEMORY_USERPTR（用户指针）
        video_obj->buf.memory = V4L2_MEMORY_MMAP;
        video_obj->buf.index = numBufs;
        //使配置生效
        if (ioctl(video_obj->fd, VIDIOC_QUERYBUF, &video_obj->buf) < 0)
        {
            perror("VIDIOC_QUERYBUF");
            return -1;
        }
        //printf("request buf %d success\n",numBufs);
        // VideoBuffer buffers是我们自己写的结构体
        video_obj->buffers[numBufs].length = video_obj->buf.length;
        video_obj->buffers[numBufs].offset = (size_t)video_obj->buf.m.offset;
        //使用mmap函数将申请的缓存地址转换应用程序的绝对地址
        video_obj->buffers[numBufs].start = (uint8_t *)mmap(NULL, video_obj->buf.length,
                                            PROT_READ | PROT_WRITE, MAP_SHARED, video_obj->fd, video_obj->buf.m.offset);
        if (video_obj->buffers[numBufs].start == MAP_FAILED)
        {
            perror("buffers error");
            return -1;
        }
        //printf("mmap buf 0x%p lenght:%d success\n",video_obj->buffers[numBufs].start,video_obj->buf.length);
        //放入缓存队列
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
功能： 启动视频采集命令，应用程序调用VIDIOC_STREAMON启动视频采集命令后，
    视频设备驱动程序开始采集视频数据，并把采集到的视频数据保存到视频驱动的视频缓冲区中。

参数说明：参数类型为V4L2的视频缓冲区类型 enum v4l2_buf_type ；

返回值说明： 执行成功时，函数返回值为 0；
 */
int V4l2StartCapture(V4L2_VIDEO_T *video_obj)
{
    //开始视频显示
    enum v4l2_buf_type type;
    //数据流类型，永远都是V4L2_BUF_TYPE_VIDEO_CAPTURE
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
功能： 停止视频采集命令，应用程序调用VIDIOC_ STREAMOFF停止视频采集命令后，视频设备驱动程序不在采集视频数据。
参数说明：参数类型为V4L2的视频缓冲区类型 enum v4l2_buf_type ；
返回值说明： 执行成功时，函数返回值为 0；函数执行成功后，视频设备停止采集视频数据。
 */
int V4l2StopCapture(V4L2_VIDEO_T *video_obj)
{
    //开始视频显示
    enum v4l2_buf_type type;
    //数据流类型，永远都是V4L2_BUF_TYPE_VIDEO_CAPTURE
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
功   能:   从视频缓冲区的输出队列中取得一个已经保存有一帧视频数据的视频缓冲区；
参数说明:参数类型为V4L2缓冲区数据结构类型 struct v4l2_buffer ；
返回值说明： 执行成功时，函数返回值为 0；函数执行成功后，相应的内核视频缓冲区中
    保存有当前拍摄到的视频数据，应用程序可以通过访问用户空间来读取该视频数据。
    （前面已经通过调用函数mmap做了用户空间和内核空间的内存映射）.
 */
int V4l2PullFrameBuffer(V4L2_VIDEO_T *video_obj, uint32_t index,
                        unsigned char **start, uint32_t *len)
{
    video_obj->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;	//取得原始采集数据
    video_obj->buf.memory = V4L2_MEMORY_MMAP;			//存储类型：V4L2_MEMORY_MMAP（内存映射）或V4L2_MEMORY_USERPTR（用户指针）
    if(video_obj->req.count <= index)
        return -1;
    video_obj->buf.index = index;						//读取缓存中的第几帧
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
控制命令：VIDIOC_QBUF

功能： 投放一个空的视频缓冲区到视频缓冲区输入队列中 ;

参数说明：参数类型为V4L2缓冲区数据结构类型 struct v4l2_buffer ;

返回值说明： 执行成功时，函数返回值为 0;函数执行成功后，指令(指定)的视频缓冲区进入视频输入队列，
    在启动视频设备拍摄图像时，相应的视频数据被保存到视频输入队列相应的视频缓冲区中。

*/
int V4l2PushFrameBuffer(V4L2_VIDEO_T *video_obj, uint32_t index)
{
    video_obj->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;	//取得原始采集数据
    video_obj->buf.memory = V4L2_MEMORY_MMAP;			//存储类型：V4L2_MEMORY_MMAP（内存映射）或V4L2_MEMORY_USERPTR（用户指针）
    if(video_obj->req.count <= index)
        return -1;
    video_obj->buf.index = index;						//第几帧放入缓存
    //获取下一帧视频数据
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
