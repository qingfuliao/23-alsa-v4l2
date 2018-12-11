#include <iostream>
#include "v4l2.h"
#include "SDL2/SDL.h"

// ±‡“Î
// g++  main.cpp v4l2.c -L/usr/local/SDL/lib/ -I/usr/include/SDL2 -lSDL2 -lpthread -o v4l2_sdl2

#define MAX_BUF 5
#define VIDEO_REC_YUV
#define PICTURE_WIDTH	320
#define PICTURE_HEIGHT	240

static pthread_t pid;
static unsigned char quit = 0;
//--------SDL2----------
static unsigned char inited = 0;
static SDL_Window *win = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static SDL_CommonEvent comm;
static SDL_Event event;

static int SDL2Init(int w, int h)
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1)
    {
        printf("SDL_Init fail!");
        return -1;
    }
    else
        printf("SDL_Init success\n");

    win = SDL_CreateWindow("V4L2 display", 0, 0, w, h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if(win == NULL)
    {
        printf("SDL_CreateWindow fail\n");
        return -1;
    }
    else
    {
        printf("SDL_CreateWindow success\n");
    }

    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    if(renderer == NULL)
    {
        printf("SDL_CreateRenderer fail\n");
        return -1;
    }
    else
    {
        printf("SDL_CreateRenderer success\n");
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YUY2, SDL_TEXTUREACCESS_STREAMING, w, h);
    if(texture == NULL)
    {
        printf("SDL_CreateTexture fail\n");
        return -1;
    }
    else
    {
        printf("SDL_CreateTexture success\n");
    }
    return 0;
}

static void *EventLoop(void *param)
{
    printf("%s begin time:%d\n", __func__, SDL_GetTicks());
    while(1)
    {
        if(SDL_PollEvent(&event) > 0 &&
                (comm.type != event.common.type || comm.timestamp != event.common.timestamp))
        {
            comm.type = event.common.type;
            comm.timestamp = event.common.timestamp;
            switch(event.type)
            {
            case SDL_QUIT:
                printf("SDL_WINDOWEVENT\n");
                quit = 1;
                return NULL;
            default:
                printf("%X\n", event.type);
                break;
            }
        }
    }
    printf("%s end time:%d\n", __func__, SDL_GetTicks());
    return NULL;
}

static int SDL2Refresh(void *pixels, int pitch)
{
    if(inited == 0)
    {
        if(SDL2Init(PICTURE_WIDTH, PICTURE_HEIGHT))
            return -1;
        inited = 1;
        if(pthread_create(&pid, NULL, EventLoop, NULL) != 0)
        {
            printf("pthread_create fail\n");
            return -1;
        }
    }

    if(SDL_UpdateTexture(texture, NULL, pixels, pitch) != 0)
    {
        printf("SDL_UpdateTexture fail\n");
        return -1;
    }
    //«Âø’‰÷»æ
    SDL_RenderClear(renderer);
    if(SDL_RenderCopy(renderer, texture, NULL, NULL) != 0)
    {
        printf("SDL_RenderCopy fail\n");
        return -1;
    }
    SDL_RenderPresent(renderer);
    return 0;
}

static void SDL2Deinit(void)
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    printf("SDL uninit\n");
}

int main(int argc, char *argv[])
{
    int index = 0;
    int count = 0;
    unsigned char *photo = NULL;
    unsigned int len = 0;
    char deive_name[64];
    char yuv_name[] = "record.yuv";
    V4L2_VIDEO_T video_capture;

    if(argc == 2)
    {
        strcpy(deive_name, argv[1]);
    }
    else
    {
        strcpy(deive_name, "/dev/video0");
    }

    printf("\n%s(%d)\n", __FUNCTION__, __LINE__);
    if(V4l2OpenDeive(&video_capture, deive_name, 1) != 0)
    {
        printf("V4l2OpenDeive failed\n");
        return -1;
    }
    printf("\n%s(%d) V4l2Querycapture\n", __FUNCTION__, __LINE__);
    if(V4l2Querycapture(&video_capture) < 0)
    {
        printf("V4l2Querycapture failed\n");
        return -1;
    }
    printf("\n%s(%d) fopen\n", __FUNCTION__, __LINE__);
#ifdef VIDEO_REC_YUV
    FILE *yuv_fd = fopen(yuv_name, "wb+");
    if(yuv_fd == NULL)
    {
        printf("open fail:%s\n", yuv_name);
        return -1;
    }
#endif
    printf("\n%s(%d) V4l2EnumFormat\n", __FUNCTION__, __LINE__);
    if(V4l2EnumFormat(&video_capture) < 0)
    {
        printf("V4l2EnumFormat failed\n");
        return -1;
    }
    printf("\n%s(%d) V4l2SetFormat\n", __FUNCTION__, __LINE__);
    if(V4l2SetFormat(&video_capture, V4L2_PIX_FMT_YUYV, PICTURE_WIDTH, PICTURE_HEIGHT) < 0)
    {
        printf("V4l2SetFormat failed\n");
        return -1;
    }
    printf("\n%s(%d) V4l2SetParam\n", __FUNCTION__, __LINE__);
    if(V4l2SetParam(&video_capture, 1, 25) < 0)
    {
        printf("V4l2SetParam failed\n");
        return -1;
    }
    printf("\n%s(%d) V4l2GetParam\n", __FUNCTION__, __LINE__);
    if(V4l2GetParam(&video_capture))
    {
        printf("V4l2GetParam failed\n");
        return -1;
    }
    if(V4l2RequestBuffer(&video_capture, MAX_BUF))
    {
        printf("V4l2RequestBuffer failed\n");
        return -1;
    }
    if(V4l2MmapBuffer(&video_capture))
    {
        printf("V4l2MmapBuffer failed\n");
        return -1;
    }
    if(V4l2StartCapture(&video_capture))
    {
        printf("start_v4l2_capture failed\n");
        return -1;
    }
    while(1)
    {
//        printf("count:%d\n", count++);
        // index ¥”0ø™ º ∑∂Œß[0, MAX_BUF)
        if(V4l2PullFrameBuffer(&video_capture, index, &photo, &len))
        {
            printf("V4l2PullFrameBuffer break, count:%d\n", count++);
            break;
        }
#ifdef VIDEO_REC_YUV
        {
//            printf("fwrite len = %d\n", len);
            int l = fwrite(photo, 1, len, yuv_fd);
            if(l != (PICTURE_HEIGHT * PICTURE_WIDTH * 2))
            {
                printf("write %s failed, l = %d\n", yuv_name, l);
                fclose(yuv_fd);
                break;
            }
        }
#endif
        if(SDL2Refresh(photo, PICTURE_WIDTH * 2))
            break;
        V4l2PushFrameBuffer(&video_capture, index);
        index++;
        if(index == MAX_BUF)
            index = 0;
        if(quit == 1)
            break;
        usleep(1000 * 10);
    }
#ifdef VIDEO_REC_YUV
    fclose(yuv_fd);
#endif
    printf("exit count:%d\n", count++);
    SDL2Deinit();
    V4l2StopCapture(&video_capture);
    V4l2ReleaseBuffer(&video_capture);
    V4l2CloseDevice(&video_capture);
}
