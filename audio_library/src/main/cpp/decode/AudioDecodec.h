//
// Created by 阳坤 on 2020-04-26.
//

#ifndef AUDIOPLAYER_AUDIODECODEC_H
#define AUDIOPLAYER_AUDIODECODEC_H


#include "BaseDecodec.h"
#include <unistd.h>

/**
 * 负责 Audio 解码
 */


class AudioDecodec : public BaseDecodec {
public:

    /**
     * 初始化线程
     */
    pthread_t pThread;

    /**
     * 读取源数据线程
     */
    pthread_t rThread;
    /**
     * 解码线程
     */
    pthread_t dThread;

    /**
     * pcm 数据
     */
    uint8_t *out_buffers = NULL;

    /**
     * 输出的通道
     */
    int out_channels;

    /**获取采样size*/
    int out_sample_size;

    /**计算缓冲大小*/
    int out_buffers_size;

    /**计算缓冲大小*/
    int out_sample_rate;

    pthread_mutex_t seek_mutex;


    AudioDecodec(Native2JavaCallback *native2JavaCallback, const char *url, Status **sta);

    ~AudioDecodec();

    /**
     * 开启解码线程
     */
    void startDecodeThread();

    /**
     * 准备解码线程
     */
    void prepareDecodeThread();

    /**
     * 解码的初始化
     */
    void decodeInit();

    /**
     * 开始解码为 PCM
     */
    void startDecode();

    /**
     * 读取解码数据
     */
    void readDecodeData();

    /**
     * 检查是否读取完毕
     * @return
     */
    int checkIsExit();

    void release();

    /**
     * 给 uint8_t 分一块内存空间
     */
    void mallocPCMBuffer() {
        //如果多次调用该函数，有可能会存在内存泄漏，所以先检查然后释放
        if (out_buffers) {
            free(out_buffers);
            out_buffers = 0;
        }
        //获取双声通道
        this->out_channels = getChannelLayoutNumChannels(AV_CH_LAYOUT_STEREO);
        //获取采样size
        this->out_sample_size = getBytesPerSample(AV_SAMPLE_FMT_S16);
        //获取采样率
        this->out_sample_rate = audioSampleRate;
        //计算缓冲大小
        this->out_buffers_size = out_sample_rate * out_sample_size * out_channels;
        //分配缓冲内存
        this-> out_buffers = (uint8_t *) (malloc(out_buffers_size));
    }


    void resume();

    void _seek(int number);

    /**
     * 根据这个来判断是否初始化成功
     */
    int initResult = 0;




    int *pLopper;
};


#endif //AUDIOPLAYER_AUDIODECODEC_H
