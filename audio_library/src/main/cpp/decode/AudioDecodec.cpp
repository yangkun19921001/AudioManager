//
// Created by 阳坤 on 2020-04-26.
//

#include "AudioDecodec.h"

/**
 * 释放包
 */
void dropAVPacket(AVPacket **avPacket) {
    if (*avPacket) {
        BaseDecodec::releasePacket(&*avPacket);
    }
}

AudioDecodec::AudioDecodec(Native2JavaCallback *native2JavaCallback, const char *url, Status **sta) :
        BaseDecodec(native2JavaCallback, url, sta) {
    this->callback = native2JavaCallback;
    this->url = url;
    pthread_mutex_init(&seek_mutex, NULL);

    avPacketQueue.setReleaseCallback(dropAVPacket);

}

AudioDecodec::~AudioDecodec() {
    pthread_mutex_destroy(&seek_mutex);
}


/**
 * 子线程初始化 FFmpeg
 * @param pVoid
 * @return
 */
void *prepareThread(void *pVoid) {
    AudioDecodec *audioDecodec = static_cast<AudioDecodec *>(pVoid);
    audioDecodec->decodeInit();
    LOGE("线程退出---》prepareThread");
    pthread_exit(&audioDecodec->pThread);

}

/**
 * 子线程读取待解码数据
 * @param pVoid
 * @return
 */
void *readThread(void *pVoid) {
    AudioDecodec *audioDecodec = static_cast<AudioDecodec *>(pVoid);
    audioDecodec->readDecodeData();
    LOGE("线程退出---》readThread");
    pthread_exit(&audioDecodec->rThread);
}

/**
 * 子线程解码
 * @param pVoid
 * @return
 */
void *decodeThread(void *pVoid) {
    AudioDecodec *audioDecodec = static_cast<AudioDecodec *>(pVoid);
    audioDecodec->startDecode();
    LOGE("线程退出---》startDecode");
    pthread_exit(&audioDecodec->dThread);
}

/**
 * 初始化编解码器参数线程
 */
void AudioDecodec::prepareDecodeThread() {
    pthread_create(&this->pThread, 0, prepareThread, this);
}

/**
 * 开启解码器线程
 */
void AudioDecodec::startDecodeThread() {
    avPacketQueue.setFlag(1);
    //解码线程
    pthread_create(&this->dThread, 0, decodeThread, this);
    //读取待解码数据的线程
    pthread_create(&this->rThread, 0, readThread, this);

}


/**
 * 对解码器和 ffmpeg 做一些初始化工作
 */
void AudioDecodec::decodeInit() {
    int ret = init();
    //保证 init 现在可以退出
    status->isFFmpegInitThreadExit = true;
    this->initResult = ret;
    switch (ret) {
        case 1://初始化成功
            LOGE("AudioDecodec decodeInit is ok %d", ret);
            break;
        case -1://异常终止程序
            LOGE("AudioDecodec  decodeInit 强制终止 %d", ret);
            break;
        default:
            LOGE("AudioDecodec  decodeInit error %d", ret);
            break;
    }
}


int AudioDecodec::checkIsExit() {
    //主要用来检测读取源数据是否完成，如果完成释放循环
    int looper = 1;
    while (looper) {
        if (avPacketQueue.queueSize() > 0) {
            av_usleep(1000 * 100);
            continue;
        } else {
            looper = false;
        }
    }
    return 1;
}


/**
 * 开始解码
 */
void AudioDecodec::readDecodeData() {
    int ret = -1;
    while (status && !this->status->exit) {//全局控制是否退出
        if (status->seek) {
            av_usleep(1000 * 100);
            continue;
        }

        if (this->avPacketQueue.queueSize() > 100) {
            av_usleep(1000 * 100);
            continue;
        }

        if (!avFormatContext) {
            pthread_mutex_unlock(&seek_mutex);
            //读取线程可以安全退出
            this->status->isReadFrameThreadExit = true;
            break;
        }

        AVPacket *avPacket = getAVPacket();
        pthread_mutex_lock(&seek_mutex);
        ret = readFrame(&this->avFormatContext, &avPacket);
        pthread_mutex_unlock(&seek_mutex);

        if (status->isCut) {
            //计算显示的时间 当前时间
            int readCurAudioTime = avPacket->pts * av_q2d(time_base);
            if (readCurAudioTime == this->cut_startTime) {
                //LOGE("cut read 读取开始帧：%d", readCurAudioTime);
            }

            if (readCurAudioTime > this->cut_endTime) {//将结束的音频也包含进去
                LOGE("cut read 读取结束帧：%d", readCurAudioTime);
//                break;这里不能这样结束，会导致下游一直处理等待阻塞
            }
        }


        if (ret == 0) {
            if (avPacket->stream_index == this->audioStreamIndex) {
                this->avPacketQueue.push(avPacket);
            } else {
                LOGE("readDecodeData 不是语音流");
                releasePacket(&avPacket);
            }
        } else {//代表读取完了,或者读取失败了，这里统一就不在多加判断条件了，直接一个 else 解决
            LOGE("readDecodeData 代表读取完了");
            releasePacket(&avPacket);
            if (checkIsExit() && status && !this->status->exit) {
                LOGE("readDecodeData 可以执行退出");
                //读取线程可以安全退出
                this->status->isReadFrameThreadExit = true;
                break;
            }
        }
    }
    //读取线程可以安全退出
    this->status->isReadFrameThreadExit = true;
    //再次释放锁资源
    pthread_mutex_unlock(&seek_mutex);
    LOGE("readDecodeData 数据已读取完毕或者强制退出");
}


/**
 * 开始真正解码的地方
 */
void AudioDecodec::startDecode() {
    //分配一块用于装 PCM 数据的 内存空间
    mallocPCMBuffer();
    int pcm_out_size = 0;
    while (status && !this->status->exit) {//全局控制是否退出
        if (status->seek) {
            av_usleep(1000 * 100);
            continue;
        }
//        if (avPacketQueue.queueSize() == 0) {
//            av_usleep(1000 * 100);
//            continue;
//        }
        //1. 拿到一个 AVPacket
        AVPacket *avPacket = getAVPacket();
        int ret = avPacketQueue.pop(avPacket);
        if (!ret) {
            releasePacket(&avPacket);
            continue;
        }
        //2. 说明拿到了待解码数据,送入解码器
        ret = sendPacket2Codec(&this->avCodecContext, avPacket);
        if (ret != 0) {
            LOGE("startDecode---> avcodec_send_packet ---> error :%d", ret);
            releasePacket(&avPacket);
            continue;
        }
        //3. 接收解码数据
        AVFrame *avFrame = getAVFrame();
        ret = receiveDecodec2AVFrame(&this->avCodecContext, &avFrame);
        if (ret != 0) {
            LOGE("startDecode---> avcodec_receive_frame ---> error :%d", ret);
            releasePacket(&avPacket);
            releaseFrame(&avFrame);
            continue;
        }

        //4.
        // 音频播放器的数据格式是我们在下面定义的（16位 双声道 ....）
        // 而原始数据（是待播放的音频 PCM 数据）
        // 所以，上面的两句话，无法统一，一个是(自己定义的16位 双声道 ..) 一个是原始数据，为了解决上面的问题，就需要重采样。
        // 开始重采样

        // 4.1 获取通道布局
        if (avFrame->channels && avFrame->channel_layout == 0) {//代表默认
            avFrame->channel_layout = getDefaultChannelLayout(avFrame);
        } else if (!avFrame->channels && avFrame->channel_layout > 0) {//代表已有通道存在
            avFrame->channels = getChannelLayoutNumChannels(avFrame->channel_layout);
        }
        //4.2 初始化一个转换上下文
        SwrContext *swrContext = NULL;
        swrContext = swrAllocSetOpts(&swrContext,
                                     AV_CH_LAYOUT_STEREO, //立体声
                                     AV_SAMPLE_FMT_S16, //采样格式大小：2 byte 大小
                                     this->out_sample_rate,//输出采样率 44100 。。。
                                     avFrame->channel_layout,//采样布局
                                     (AVSampleFormat) avFrame->format,//采样格式
                                     avFrame->sample_rate,//输入采样率 44100 。。。
                                     0,
                                     0);


        //4.3 初始化 转换上下文
        ret = swrInit(&swrContext);
        if (!swrContext || ret < 0) {//代表转换失败
            LOGE("startDecode---> swrAllocSetOpts or  swrInit ---> error :%d", ret);
            releasePacket(&avPacket);
            releaseFrame(&avFrame);
            continue;
        }

        int64_t dst_nb_samples = avRescaleRnd(swr_get_delay(swrContext, avFrame->sample_rate) +
                                              avFrame->nb_samples, out_sample_rate,
                                              avFrame->sample_rate, AV_ROUND_UP);


        //4.4 开始转换
        ret = swrConvert(
                swrContext,
                &out_buffers,
                dst_nb_samples,
                (const uint8_t **) avFrame->data,
                avFrame->nb_samples);

        if (ret < 0) {//说明转换失败了，需要释放
            releasePacket(&avPacket);
            releaseFrame(&avFrame);
            releaseSwrContext(&swrContext);
            LOGE("startDecode---> swrConvert ---> error :%d", ret);
            continue;
        }

        pcm_out_size = ret * out_sample_size * out_channels;

        //计算显示的时间
        int now_time = 0;
        now_time = avFrame->pts * av_q2d(time_base);

        /**
        * 开始进行裁剪
        */
        if (status->isCut) {
            //计算显示的时间 当前时间
            if (now_time == this->cut_startTime && !this->startCutTag) {
                LOGE("cut decode 解码开始帧：%d", now_time);
                this->startCutTag = 1;
                this->endCutTag = 0;
                this->callback->onCutStart(CHILD_THREAD);
            }

            if (now_time > this->cut_endTime && !this->endCutTag) {
                LOGE("cut decode 解码结束帧：%d", now_time);
                this->status->isCut = false;
                this->status->isCutPlayer = true;
                this->endCutTag = 1;
                this->startCutTag = 0;
                this->callback->onCutComplete(CHILD_THREAD);
                //释放
                releasePacket(&avPacket);
                releaseFrame(&avFrame);
                releaseSwrContext(&swrContext);
                avPacketQueue.clearQueue();
                break;//截取完成，可以退出
            }
        }
        //4.5 将解码完成后的 PCM 数据回调给需要处
        if (this->decodecFrameCallback) {
            decodecFrameCallback(now_time, pcm_out_size, out_buffers, ret, out_sample_rate, out_channels, 16);
        }
        //释放
        releasePacket(&avPacket);
        releaseFrame(&avFrame);
        releaseSwrContext(&swrContext);
        av_usleep(1000 * 10);

    }

    //解码线程可以安全退出
    this->status->isDecodecFrameThreadExit = true;
    LOGE("startDecode---> 解码退出");
}

/**
 * 释放解码器所有内存
 */
void AudioDecodec::release() {
    LOGE("AudioDecodec release start");
    //1. 先释放父类,在释放子类
    onRelease();
    //释放解码格式的上下文
    if (avFormatContext)
        releaseFormatContext(&avFormatContext);

    if (avCodecContext)
        releaseAVCodecContext(&avCodecContext);

    //如果pcm buffer 存在就释放
    if (this->out_buffers) {
        free(out_buffers);
        out_buffers = NULL;
    }
    if (url) {
        delete (url);
        url = NULL;
    }
    //释放队列
    if (avPacketQueue.queueSize() > 0) {
        avPacketQueue.clearQueue();
        avPacketQueue.delReleaseCallback();
        avPacketQueue.setFlag(0);
    }
    LOGE("AudioDecodec release is ok");
}

void AudioDecodec::resume() {

}

void AudioDecodec::_seek(int number) {
    LOGE(">>>>>>>>>seek");
    if (status) {
        pthread_mutex_lock(&seek_mutex);
        status->seek = true;
        if (avPacketQueue.queueSize() > 0)
            avPacketQueue.clearQueue();
        seek(number);
        status->seek = false;
        pthread_mutex_unlock(&seek_mutex);
    }
    LOGE("seek<<<<<<<<<");
}


/**
 * 裁剪
 * @param startTime
 * @param endTime
 * @param isPlayer
 */
void AudioDecodec::cutAudio2Pcm(int startTime, int endTime, int isPlayer) {
    if (startTime >= 0 && endTime <= this->duration && duration != 0) {
        if (status) {
            this->cut_startTime = startTime;
            this->cut_endTime = endTime;

        }
    }
}









