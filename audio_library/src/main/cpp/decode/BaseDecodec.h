//
// Created by 阳坤 on 2020-04-27.
//

#ifndef AUDIOPLAYER_BASEDECODEC_H
#define AUDIOPLAYER_BASEDECODEC_H

#include <Status.h>
#include "Native2JavaCallback.h"
#include "NativeQueue.h"

extern "C" {
#include "libavutil/avutil.h"
#include "libavutil/time.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include <libswresample/swresample.h>
};

//template<typename T>

class BaseDecodec {
    //解码完成帧回调
    /**
     * @param type
     */
    typedef void (*DecodecFrameCallback)(int curTime, int outPCMSize, uint8_t *, int size, int sampleRate, int channel,
                                         int bit);

    /**
     * @param 初始化成功，需要将音频源数据信息分发下去
     */
    typedef void (*AudioStreamsInfoCallBack)(int sampleRate, int totalTime);


public:
    int audioStreamIndex = -1;
    AVCodecParameters *avAudioCodecParameters = NULL;
    Native2JavaCallback *callback = NULL;
    AVFormatContext *avFormatContext = NULL;
    AVCodecContext *avCodecContext = NULL;
    NativeQueue<AVPacket *> avPacketQueue;//未解码数据
    DecodecFrameCallback decodecFrameCallback;//解码回调
    AudioStreamsInfoCallBack audioStreamsInfoCallBack;//音频流数据信息回调
    pthread_mutex_t init_mutex;
    int duration = 0;
    AVRational time_base;
    const char *url = 0;
    /**
      * 执行的任务是否退出
      */
    Status *status;

    /**
     * 采样率
     */
    int audioSampleRate;


    BaseDecodec(Native2JavaCallback *native2JavaCallback, const char *url, Status **sta);

    ~BaseDecodec();

    int init();

    void onRelease();

    //设置删除帧的回调
    void addDecodecFrameCallback(DecodecFrameCallback decodecFrameCallback) {
        this->decodecFrameCallback = decodecFrameCallback;
    }

    //设置音频数据信息的回调，分发给需要的地方，需要的话，只需要注册回调即可
    void addAudioStreamsInfoCallBack(AudioStreamsInfoCallBack decodecFrameCallback) {
        this->audioStreamsInfoCallBack = decodecFrameCallback;
    }


    void seek(int number);


    int getStreamPlayDuration() { return this->duration; }


/**
 * 拿到 ffmpeg 版本
 * @return
 */
    static const char *getFFmpegVersion() {
        return av_version_info();
    };

    static const char *getBuildConfiguration() {
        return avutil_configuration();
    };

    void avNetworkInit() {
        avformat_network_init();

    }

/**
 * 分配一个 AVFormatContext
 * 可以用于释放上下文等内容
 * @return
 */
    AVFormatContext *getAVFormatContext() {
        return avformat_alloc_context();
    }


/**
 * 根据 source 读取数据源
 *
 * @return 0 on success, a negative AVERROR on failure.
 */
    int readSource(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options) {
        if (!(*ps))return -1;
        return avformat_open_input(&*ps, url, 0, *&options);
    }

/**
 * 找到原始数据流
 * @param options 这个参数一般可以传入 NULL
 * @return >=0 OK
 *
 */
    int findStreamInfo(AVFormatContext **ic, AVDictionary **options) {
        if (!(*ic))return -1;
        return avformat_find_stream_info(*ic, options);
    }

/**
 * 查看是否有音频流
 * @see AVMEDIA_TYPE_AUDIO
 */

    int findAudioStream(AVFormatContext **pFormatCtx) {
        if (!(*pFormatCtx))return -1;
        for (int i = 0; i < (*pFormatCtx)->nb_streams; i++) {
            if (this->status->exit)return -1;
            if ((*pFormatCtx)->streams && (*pFormatCtx)->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)//得到视频流
            {
                LOGE("findVideoStream index--1 :%d", i);
                continue;
            }
            if ((*pFormatCtx)->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)//得到音频流
            {
                this->audioStreamIndex = i;
                this->avAudioCodecParameters = (*pFormatCtx)->streams[i]->codecpar;
                this->audioSampleRate = (*pFormatCtx)->streams[i]->codecpar->sample_rate;
                this->channels = (*pFormatCtx)->streams[i]->codecpar->channels;
                this->duration = (*pFormatCtx)->streams[i]->duration / AV_TIME_BASE;
                this->time_base = (*pFormatCtx)->streams[i]->time_base;
                LOGE("findAudioStream index--1 :%d", i);
                return i + 1;
            }
        }
        return 0;
    }

/**
 * 拿到解码器
 */
    AVCodec *getAVCodec(AVCodecID avCodecID) {
        return avcodec_find_decoder(avCodecID);
    }

/**
 * 分配一个解码上下文
 */
    AVCodecContext *getAVCodecContext(AVCodec *avCodec) {
        return avcodec_alloc_context3(avCodec);
    }

/**
 * 给解码器设置参数
 * @return >= 0 on success, a negative AVERROR code on failure.
 */
    int
    avCodecParameters2Context(AVCodecContext *codecContext, const AVCodecParameters *avCodecParameters) {
        if (!codecContext)return -1;
        return avcodec_parameters_to_context(codecContext, avCodecParameters);
    }

/**
 * 打开解码器
 * @return zero on success, a negative value on error
 */
    int openAVCodec(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options) {
        if (!avctx)return -1;
        return avcodec_open2(avctx, codec, options);
    }

/**
 * 拿到解码包
 */
    AVPacket *getAVPacket() {
        return av_packet_alloc();
    }

/**
 * 读取包里面待解码帧数据
 */
    int readFrame(AVFormatContext **s, AVPacket **pkt) {
        if (!*s)return -1;
        return av_read_frame(*s, *pkt);
    }

/**
 * 释放 packet
 */
    static int releasePacket(AVPacket **avPacket) {
        av_packet_free(*&avPacket);
        av_free(*avPacket);
        *avPacket = NULL;
        return 1;
    }

    /**
 * 释放 packet
 */
    int releaseFrame(AVFrame **avFrame) {
        av_frame_free(*&avFrame);
        av_free(*avFrame);
        *avFrame = NULL;
        return 1;
    }

    /**
     * 释放 AVFormatContext
     * @param formatContext
     */
    void releaseFormatContext(AVFormatContext **formatContext) {
        avformat_close_input(*&formatContext);
        avformat_free_context(*formatContext);
        formatContext = NULL;
    }

    /**
     * 释放编解码器
     * @param avCodecContext
     */
    void releaseAVCodecContext(AVCodecContext **avCodecContext) {
        avcodec_close(*avCodecContext);
        avcodec_free_context(*&avCodecContext);
        *avCodecContext = NULL;
    }

    /**
     * 将待解码数据发送到解码器中
     * @param avctx
     * @param avpkt
     * @return  AVERROR 为失败
     */
    int sendPacket2Codec(AVCodecContext **avctx, const AVPacket *avpkt) {
        return avcodec_send_packet(*avctx, avpkt);
    }

    /**
     * 初始化一个解码帧数据，也就是解码完成原始的 PCM 数据包装类
     */
    AVFrame *getAVFrame() {
        return av_frame_alloc();
    }

    /**
     * 从编码器中拿到解码之后的数据
     * @return 0 is OK
     */
    int receiveDecodec2AVFrame(AVCodecContext **avctx, AVFrame **frame) {
        return avcodec_receive_frame(*avctx, *frame);
    }

    /**
     * 获取默认的通道数量
     * @param nb_channels
     * @return
     */
    int64_t getDefaultChannelLayout(AVFrame *avFrame) {
        return av_get_default_channel_layout(avFrame->channels);
    }

    /**
     * 获取当前通道数据量
     */
    int getChannelLayoutNumChannels(uint64_t channel_layout) {
        return av_get_channel_layout_nb_channels(channel_layout);
    }

    /**
     * 返回当前采样格式大小
     */
    int getBytesPerSample(enum AVSampleFormat sample_fmt) {
        return av_get_bytes_per_sample(sample_fmt);
    }

    /**
     * 初始化一个转换上下文
     * 根据通道布局、音频数据格式、采样频率，返回分配的转换上下文 SwrContext 指针
     */
    SwrContext *swrAllocSetOpts(struct SwrContext **s,
                                int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate,
                                int64_t in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate,
                                int log_offset, void *log_ctx) {
        return swr_alloc_set_opts(*s, out_ch_layout,
                                  out_sample_fmt,
                                  out_sample_rate,
                                  in_ch_layout,
                                  in_sample_fmt,
                                  in_sample_rate,
                                  log_offset,
                                  log_ctx);
    }

    /**
     * 初始化 转换上下文
     * @return AVERROR error code in case of failure.
     */
    int swrInit(SwrContext **swrContext) {
        return swr_init(*swrContext);
    }

    /**
      * 重采样
      *
      * @param out_buffers            输出缓冲区，当PCM数据为Packed包装格式时，只有out[0]会填充有数据。
      * @param dst_nb_samples         每个通道可存储输出PCM数据的sample数量。
      * @param in                     输入缓冲区，当PCM数据为Packed包装格式时，只有in[0]需要填充有数据。
      * @param in_count               输入PCM数据中每个通道可用的sample数量。
      *
      * @return  返回每个通道输出的sample数量，发生错误的时候返回负数。
      */
    int swrConvert(struct SwrContext *s, uint8_t **out_buffers, int dst_nb_samples,
                   const uint8_t **in, int in_count) {
        return swr_convert(s,
                           out_buffers,
                           dst_nb_samples,
                           in,
                           in_count);//返回每个通道输出的sample数量，发生错误的时候返回负数。
    }

    /**
     * 转换时间基
     * @param a
     * @param b
     * @param c
     * @param rnd
     * @return
     */
    int64_t avRescaleRnd(int64_t a, int64_t b, int64_t c, enum AVRounding rnd) {
        return av_rescale_rnd(a, b, c, rnd);
    };

    /**
     * 释放转换 swrContext
     */
    void releaseSwrContext(SwrContext **swrContext) {
        swr_free(*&swrContext);
        *swrContext = NULL;
    }

    /**
     * 释放 万能指针
     */
    void releaseFree(void **p) {
        av_free(*p);
        *p = NULL;
    }

    int *pLopper = NULL;
    /**
     * 该音频流的通道数量
     */
    int channels = 0;
};


#endif //AUDIOPLAYER_BASEDECODEC_H
