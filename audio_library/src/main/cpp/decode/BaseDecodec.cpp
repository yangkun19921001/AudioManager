//
// Created by 阳坤 on 2020-04-27.
//

#include "BaseDecodec.h"


BaseDecodec::BaseDecodec(Native2JavaCallback *native2JavaCallback, const char *url, Status **sta) {
    this->callback = native2JavaCallback;
    this->url = url;
    this->status = *sta;
    pthread_mutex_init(&init_mutex, NULL);
}


int avformat_callback(void *ctx) {
    BaseDecodec *decodec = (BaseDecodec *) (ctx);
    if (decodec->status->exit) {
        return AVERROR_EOF;
    }
    return 0;
}


int BaseDecodec::init() {
    pthread_mutex_lock(&init_mutex);
    avNetworkInit();
    if (status && this->status->isFFmpegInitThreadExit) {//这里如果快速点击，有可能为导致空指针
        LOGE("init code :%d", status);
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    //1. 拿到 AVFormatContext 上下文
    this->avFormatContext = getAVFormatContext();
    //1.1 定义出错回调
    this->avFormatContext->interrupt_callback.callback = avformat_callback;
    this->avFormatContext->interrupt_callback.opaque = this;
    //2.根据传入的链接读取数据源
    //字典: 键值对
    AVDictionary *dictionary = 0;
    av_dict_set(&dictionary, "timeout", "5000000", 0);//单位是微妙
    int ret = readSource(&avFormatContext, this->url, NULL, &dictionary);
    if (ret != 0) {//-5 没有连接网络
        LOGE("can not open url error code :%d", ret);
        pthread_mutex_unlock(&init_mutex);
        return -2;
    }
    if (status && this->status->isFFmpegInitThreadExit) {
        LOGE("init :%d", status);
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    //3.找到语音流
    ret = findStreamInfo(&this->avFormatContext, &dictionary);
    av_dict_free(&dictionary);
    if (ret < 0) {
        LOGE("can not find streams info from %s");
        pthread_mutex_unlock(&init_mutex);
        return -3;
    }
    if (status && this->status->isFFmpegInitThreadExit) {
        LOGE("init :%d", status);
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    //4.找到对应音频流的索引，还有解码参数
    ret = findAudioStream(&this->avFormatContext);
    if (ret <= 0) {
        LOGE("can not find audio streams error code :%d ", ret);
        pthread_mutex_unlock(&init_mutex);
        return -6;
    }
    if (status && this->status->isFFmpegInitThreadExit) {
        LOGE("init :%d", status);
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    //这里捕获到了一个无效内存引用，固在判断下
    if (this->avAudioCodecParameters == NULL)
        return -1;
    //5.得到解码器
    AVCodec *codec = getAVCodec(this->avAudioCodecParameters->codec_id);
    if (!codec) {
        LOGE("can not find AVCodec from");
        pthread_mutex_unlock(&init_mutex);
        return -4;
    }
    if (status && this->status->isFFmpegInitThreadExit) {
        LOGE("init :%d", status);
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    //6. 得到解码器上下文
    this->avCodecContext = getAVCodecContext(codec);
    if (!avCodecContext) {
        LOGE("can not find AVCodecContext from");
        pthread_mutex_unlock(&init_mutex);
        return -5;
    }
    if (status && this->status->isFFmpegInitThreadExit) {
        LOGE("can not open url error code :%d", status);
        return -1;
    }
    //7. 给解码器上下文设置解码参数
    ret = avCodecParameters2Context(avCodecContext, this->avAudioCodecParameters);
    if (ret < 0) {
        LOGE("can not find CodecParameters2Context error code :%d ", ret);
        pthread_mutex_unlock(&init_mutex);
        return -6;
    }
    //8. 打开解码器
    ret = openAVCodec(avCodecContext, codec, NULL);
    if (ret != 0) {
        LOGE("cant not open Codec  error code :%d  ", ret);
        pthread_mutex_unlock(&init_mutex);
        return -7;
    }
    //将音频流信息回调出去
    if (audioStreamsInfoCallBack && avFormatContext && avFormatContext->streams[audioStreamIndex])
        this->audioStreamsInfoCallBack(this->audioSampleRate, avFormatContext->duration / AV_TIME_BASE);
    //走到这里说明一切正常，编解码器初始化成功
    if (callback)
        this->callback->onCallParapred(CHILD_THREAD);
    pthread_mutex_unlock(&init_mutex);
    return 1;
}

BaseDecodec::~BaseDecodec() {
    pthread_mutex_destroy(&init_mutex);
}

/**
 * 释放
 */
void BaseDecodec::onRelease() {
    if (this->avAudioCodecParameters) {
        delete (this->avAudioCodecParameters);
        this->avAudioCodecParameters = NULL;
    }

}

void BaseDecodec::seek(int number) {
    if (duration <= 0) {
        return;
    }
    if (number >= 0 && number <= number) {
        int64_t rel = number * AV_TIME_BASE;
        avformat_seek_file(this->avFormatContext, -1, INT64_MIN, rel, INT64_MAX, 0);
    }

}







