//
// Created by 阳坤 on 2020-04-27.
//

#ifndef AUDIOPLAYER_BASECHANNEL_H
#define AUDIOPLAYER_BASECHANNEL_H

extern "C" {
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <libavutil/frame.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
};

#include <android_xlog.h>
#include <NativeQueue.h>
#include <Native2JavaCallback.h>
#include "../soundtouch/include/SoundTouch.h"
#include "../manager/Status.h"


using namespace soundtouch;


/**
 * 用于播放需要的结构体
 */
typedef struct AV_PCM_DATA {
    int pcmSize;
    uint8_t *pcm = NULL;
    int curProgress;
    int totalProgress;
    int oldSize;

} AV_PCM_DATA;

class BaseAudioChannel {
public:
    AV_PCM_DATA *getAVPCMPack(int pcmSize, int curProgress, int totalProgress, int oldSize) {
        AV_PCM_DATA *av_pcm_data = (AV_PCM_DATA *) (av_mallocz(sizeof(AV_PCM_DATA)));
        av_pcm_data->pcm = (uint8_t *) (av_malloc(pcmSize));
        av_pcm_data->pcmSize = pcmSize;
        av_pcm_data->curProgress = curProgress;
        av_pcm_data->totalProgress = totalProgress;
        av_pcm_data->oldSize = oldSize;
        return av_pcm_data;
    };


    BaseAudioChannel(Status **sta, Native2JavaCallback *pCallback);

    ~BaseAudioChannel();

    /**
    * 当前流数据的时长
*/
    int duration = 0;


    /**
     * 初始化 OpenSLES
     */
    void initOpenSLES(int sample_rate);

    /**
     * 获取当前采样率
     * @param sample_rate
     * @return
     */
    SLint32 getCurSampleRate(int sample_rate);

    /**
     * 写入音频 PCM 数据
     * @param pcm
     * @param length
     */
    void writeAudioData(uint8_t *pcm, int length, int i, int i1);

    /**
     * 暂停播放
     */
    void pause();

    /**
     * 恢复播放
     */
    void resume();

    /**
     * 停止播放/释放
     */
    void release();


    void initThreadLock();

    void destroyThreadLock();

    void waitThreadLock();

    void notifyThreadLock();

    int getPCMData();

    int getCurVolume();

    void setChannelMode(int channelMode);

    int getChannelMode() {
        return mChannelMode;
    };

    void setSpeedAndPitch(float speed, bool isPitch);

    int setSoundTouchData();


public:
    NativeQueue<AV_PCM_DATA *> pcmQueue;

    /**
     * 采样率
     */
    int sampleRate;
    /**
     * 总的播放时长
     */
    double total_time = 0.0;
    /**
     * 当前 frame 时间戳
     */
    double curFrame_time;
    /**
     * 上一次调用的时间
     */
    double pre_tiem = 0.0;


    /**
     * Java 层回调
     */
    Native2JavaCallback *pCallback = NULL;

    /**
     * pcm 缓冲数据
     */
    uint8_t *out_pcm_buffer = NULL;

    /**
     * 循环处理事件管理
     */
    Status *status = NULL;


    uint8_t *mBuffers[2];
    int mBufSize;
    int mIndex;
    pthread_mutex_t mMutex;
    pthread_cond_t cont_out;
    int cont_val;


    // 引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;

    //音频通道
    SLMuteSoloItf pcmChannelModePlay = NULL;

    //音量
    SLVolumeItf pcmVolumePlay = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

    void setVolume(int volume);


    /**
     * 速率初始化
     */
    void speedInit() {
//        this->sampleBuffer = static_cast<SAMPLETYPE *>(malloc(this->sampleRate * 2 * 2));
        soundTouch = new SoundTouch();
        soundTouch->setSampleRate(this->sampleRate);
        soundTouch->setChannels(2);
        soundTouch->setPitch(this->speed);
        soundTouch->setTempo(this->speed);
    }


    /**
     * 获取当前的速率
     * @return
     */
    int getSpeed() {
        return this->speed;
    }

    /**
     * 拿到当前语音分贝值
     */
    int getVoiceDecibel(const unsigned char *pcmData, size_t pcmSize);


private:
    /**
    * 当前播放的音量
    */
    int curVolume = 100;

    /**
     * 当前播放声音通道,立体声
     */
    int mChannelMode = 2;
    /**
     * 播放速率
     */
    float speed = 1.0f;
    /**
     * 是否变调
     */
    bool isPitch = true;

    //SoundTouch
    SoundTouch *soundTouch = NULL;

    //原始转换出来的 size
    int oldSize;


    pthread_mutex_t mutexSpeed;
};


#endif //AUDIOPLAYER_BASECHANNEL_H
