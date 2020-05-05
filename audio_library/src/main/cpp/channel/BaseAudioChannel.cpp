//
// Created by 阳坤 on 2020-04-27.
//


#include "BaseAudioChannel.h"

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *pVoid);


void updatePlayTimeInfo(BaseAudioChannel *audioPlayer);

void updatePlayVoiceDBInfo(Native2JavaCallback *pCallback, int size);


void onReleasePCMCallback(AV_PCM_DATA **pcmData) {
    if (*pcmData && (*pcmData)->pcm) {
        free((*pcmData)->pcm);
        (*pcmData)->pcm = NULL;
        (*pcmData)->pcm = 0;
    }
}

BaseAudioChannel::BaseAudioChannel(Status **sta, Native2JavaCallback *pCallback) : mIndex(0) {
    this->status = *sta;
    this->pCallback = pCallback;
    initThreadLock();
    pcmQueue.setFlag(1);

    //必须初始化
    mBuffers[0] = NULL;
    mBuffers[1] = NULL;


    pthread_mutex_init(&mutexSpeed, NULL);

    /**
     * 注册释放内存
     */
    pcmQueue.setReleaseCallback(onReleasePCMCallback);

}


BaseAudioChannel::~BaseAudioChannel() {
}


void BaseAudioChannel::initOpenSLES(int sample_rate) {
    this->sampleRate = sample_rate;
    this->out_pcm_buffer = (uint8_t *) (av_malloc(sample_rate * 2 * 2));
    speedInit();
    SLresult result;
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("initOpenSLES ->1 error %d", result);
        return;
    }

    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_BOOLEAN_FALSE != result) {
        LOGE("initOpenSLES ->2 error %d", result);
        return;
    }
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("initOpenSLES ->3 error %d", result);
        return;
    }

    //第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("initOpenSLES ->4 error %d", result);
        return;
    }
    (void) result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_BOOLEAN_FALSE != result) {
        LOGE("initOpenSLES ->5 error %d", result);
        return;
    }

    (void) result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);

    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void) result;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, 0};


    // 第三步，配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            static_cast<SLuint32>(getCurSampleRate(sample_rate)),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };

    SLDataSource slDataSource = {&android_queue, &pcm};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_MUTESOLO};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk,
                                                sizeof(ids) / sizeof(ids[0]), ids, req);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("initOpenSLES ->6 error %d", result);
        return;
    }
    //初始化播放器
    result = (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("initOpenSLES ->7 error %d", result);
        return;
    }
    //得到接口后调用  获取Player接口
    result = (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("initOpenSLES ->8 error %d", result);
        return;
    }

    //设置音量接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmVolumePlay);
    setVolume(this->curVolume);

    //设置音频通道
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_MUTESOLO, &pcmChannelModePlay);

    //注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
    //获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    LOGE("initOpenSLES -> init is ok");
    this->status->isPlayThreadExit = true;
    pcmBufferCallBack(pcmBufferQueue, this);
    LOGE("initOpenSLES -> init is ok");
}


SLint32 BaseAudioChannel::getCurSampleRate(int sample_rate) {
    SLint32 rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}


void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *pVoid) {
    auto audioPlayer = static_cast<BaseAudioChannel *>(pVoid);
    if (!audioPlayer)
        return;
    if (audioPlayer->status && audioPlayer->status->exit)
        LOGE("looper  pcmBufferCallBack start");
    //拿到 PCM 原始数据
    int size = audioPlayer->getData();

    //对 PCM 做变速变调操作。
    size = audioPlayer->setSoundTouchData();

    if (size > 0 && audioPlayer->out_pcm_buffer && audioPlayer->status && !audioPlayer->status->exit) {
        //更新播放时间
        updatePlayTimeInfo(audioPlayer);
        //更新播放语音分贝值
        updatePlayVoiceDBInfo(audioPlayer->pCallback, audioPlayer->getVoiceDecibel(
               audioPlayer->out_pcm_buffer, size));
        if (audioPlayer->pcmBufferQueue && audioPlayer->pcmBufferQueue)
            //放入缓存，开始播放声音
            (*audioPlayer->pcmBufferQueue)->Enqueue(audioPlayer->pcmBufferQueue, audioPlayer->out_pcm_buffer, size);
    }
    if (audioPlayer->status && audioPlayer->status->exit)
        LOGE("looper  pcmBufferCallBack exit");
}

void updatePlayVoiceDBInfo(Native2JavaCallback *pCallback, int db) {
    pCallback->onVoiceDBInfo(CHILD_THREAD, db);
}

void updatePlayTimeInfo(BaseAudioChannel *audioPlayer) {
    audioPlayer->total_time += audioPlayer->mBufSize / (double) audioPlayer->sampleRate * 2 * 2;
    if (0.1 <= audioPlayer->total_time - audioPlayer->pre_tiem) {
        audioPlayer->pre_tiem = audioPlayer->total_time;
        if (audioPlayer->duration)
            //把时长回调给应用层
            audioPlayer->pCallback->onCallTimeInfo(CHILD_THREAD, static_cast<int>(audioPlayer->total_time),
                                                   audioPlayer->duration);
    }
}


/**
 * 提供二种方式来进行传流播放
 * @param pcm
 * @param length
 * @param curPro
 * @param totalPro
 */
void BaseAudioChannel::writeAudioData(uint8_t *pcm, int length, int curPro, int totalPro) {
    // 必须等待一帧音频播放完毕后才可以 Enqueue 第二帧音频


//    if (!this->status->exit)//只有播放的时候阻塞
    if (!this->status->exit)
        waitThreadLock();

    if (this->status->exit) {
        LOGE("----》退出阻塞");
        return;
    }

    this->total_time = curPro;
    this->duration = totalPro;
    if (mBufSize < length) {
        mBufSize = length;
        if (mBuffers[0]) {
            delete[] mBuffers[0];
            mBuffers[0] = NULL;
        }
        if (mBuffers[1]) {
            delete[] mBuffers[1];
            mBuffers[1] = NULL;
        }
        mBuffers[0] = new uint8_t[mBufSize];
        mBuffers[1] = new uint8_t[mBufSize];
        LOGE("mBufSize :%d pcmSize:%d", mBufSize, length);
    }
    //这里 mBuffers ，mIndex 必须初始化
    memcpy(mBuffers[mIndex], pcm, length);
    //手动激活回调函数
    (*this->pcmBufferQueue)->Enqueue(this->pcmBufferQueue, mBuffers[mIndex], length);
    mIndex = 1 - mIndex;
}

void BaseAudioChannel::pause() {
    if (pcmPlayerPlay)
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PAUSED);
}

void BaseAudioChannel::resume() {
    if (pcmPlayerPlay)
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);

}


void BaseAudioChannel::initThreadLock() {
    pthread_mutex_init(&mMutex, NULL);
    pthread_cond_init(&cont_out, NULL);
}

void BaseAudioChannel::destroyThreadLock() {
    pthread_mutex_destroy(&mMutex);
    pthread_cond_destroy(&cont_out);
}

void BaseAudioChannel::waitThreadLock() {
    pthread_mutex_lock(&mMutex);
    if (cont_val == 0) {
        pthread_cond_wait(&cont_out, &mMutex);
    }
    cont_val = 0;
    pthread_mutex_unlock(&mMutex);
}

void BaseAudioChannel::notifyThreadLock() {
    pthread_mutex_lock(&mMutex);
    cont_val = 1;
    pthread_cond_signal(&cont_out);
    pthread_mutex_unlock(&mMutex);
}


/**
 * 倍速 变调
 * @param out_buffer
 * @param pcmSize
 */
int finished = true;


//FILE *file = fopen("sdcard/ceshi1.pcm", "w");
//FILE *file2 = fopen("sdcard/ceshi2.pcm", "w");


int BaseAudioChannel::setSoundTouchData() {
    int num = 0;
    while (status && !status->exit) {
        if (finished) {
            finished = false;
            if (this->mBufSize > 0 && this->out_pcm_buffer) {
                pthread_mutex_lock(&mutexSpeed);
                soundTouch->putSamples(reinterpret_cast<const SAMPLETYPE *>(this->out_pcm_buffer), this->oldSize);
                num = soundTouch->receiveSamples(reinterpret_cast<SAMPLETYPE *>(this->out_pcm_buffer),
                                                 this->mBufSize / 4);
                pthread_mutex_unlock(&mutexSpeed);
            } else {
                soundTouch->flush();
            }
        }
        if (num == 0) {
            finished = true;
            continue;
        }

        return num * 2 * 2;
    }

    return 0;
}

//FILE *file = fopen("sdcard/ceshi2.pcm", "w");
//fwrite(this->buffer, size, 1, file);
int BaseAudioChannel::getData() {
    while (this->status && !this->status->exit) {
        if (this->status->seek) {
            av_usleep(5 * 1000);
            continue;
        }
        AV_PCM_DATA *av_pcm_data;
        int ret = pcmQueue.pop(av_pcm_data);
        if (ret) {
            this->out_pcm_buffer = (uint8_t *) memcpy(this->out_pcm_buffer, av_pcm_data->pcm, av_pcm_data->pcmSize);
            this->total_time = av_pcm_data->curProgress;
            this->duration = av_pcm_data->totalProgress;
            this->mBufSize = av_pcm_data->pcmSize;
            this->oldSize = av_pcm_data->oldSize;
            break;
        }

        //释放原有的数据避免内存泄漏
        if (av_pcm_data && av_pcm_data->pcm) {
            free(av_pcm_data->pcm);
            av_pcm_data->pcm = NULL;
            free(av_pcm_data);
            av_pcm_data = NULL;

        }
        continue;
    }
    return this->mBufSize;
}

/**
 * 设置当前音量
 * @param volume
 */
void BaseAudioChannel::setVolume(int percent) {
    this->curVolume = percent;
    if (pcmVolumePlay != NULL) {
        if (percent > 30) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -20);
        } else if (percent > 25) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -22);
        } else if (percent > 20) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -25);
        } else if (percent > 15) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -28);
        } else if (percent > 10) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -30);
        } else if (percent > 5) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -34);
        } else if (percent > 3) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -37);
        } else if (percent > 0) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -40);
        } else {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -100);
        }
    }
}

/**
 * 获取当前音量接口
 * @return
 */
int BaseAudioChannel::getCurVolume() {
    return this->curVolume;
}

/**
 * 设置音频通道
 * @param channelMode
 */
void BaseAudioChannel::setChannelMode(int channelMode) {
    this->mChannelMode = channelMode;
    if (pcmChannelModePlay != NULL) {
        if (channelMode == 0)//right
        {
            (*pcmChannelModePlay)->SetChannelMute(pcmChannelModePlay, 1, false);
            (*pcmChannelModePlay)->SetChannelMute(pcmChannelModePlay, 0, true);
        } else if (channelMode == 1)//left
        {
            (*pcmChannelModePlay)->SetChannelMute(pcmChannelModePlay, 1, true);
            (*pcmChannelModePlay)->SetChannelMute(pcmChannelModePlay, 0, false);
        } else if (channelMode == 2)//center
        {
            (*pcmChannelModePlay)->SetChannelMute(pcmChannelModePlay, 1, false);
            (*pcmChannelModePlay)->SetChannelMute(pcmChannelModePlay, 0, false);
        }


    }
}

void BaseAudioChannel::setSpeedAndPitch(float speed, bool isPitch) {
    this->speed = speed;
    this->isPitch = isPitch;
    if (!soundTouch) {
        LOGE("setSpeedAndPitch error soundTouch is NULL");
        return;
    }
    pthread_mutex_lock(&mutexSpeed);
    //给定一个区间
    if (speed >= 0.0 && speed <= 3.0) {
        if (isPitch)
            soundTouch->setPitch(speed);
        else
            soundTouch->setPitch(1.0);//恢复默认
        soundTouch->setTempo(speed);
    }
    pthread_mutex_unlock(&mutexSpeed);

}


/**
* 获取所有振幅之平均值 计算db (振幅最大值 2^16-1 = 65535 最大值是 96.32db)
* 16 bit == 2字节 == short int
* 无符号16bit：96.32=20*lg(65535);
*
* @param pcmdata 转换成char类型，才可以按字节操作
* @param size pcmdata的大小
* @return
*/
int BaseAudioChannel::getVoiceDecibel(const unsigned char *pcmdata, size_t size) {
    int db = 0;
    short int value = 0;
    double sum = 0;
    for (int i = 0; i < size; i += 2) {
        memcpy(&value, pcmdata + i, 2); //获取2个字节的大小（值）
        sum += abs(value); //绝对值求和
    }
    sum = sum / (size / 2); //求平均值（2个字节表示一个振幅，所以振幅个数为：size/2个）
    if (sum > 0) {
        db = (int) (20.0 * log10(sum));
    }
    return db;
}


void BaseAudioChannel::release() {

    LOGE("开始释放编解码器 15");
    //1. 设置停止状态
    if (pcmPlayerPlay) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_STOPPED);
        delete pcmPlayerPlay;
        pcmPlayerPlay = 0;
    }

    LOGE("开始释放编解码器 16");
    //2. 销毁播放器
    if (pcmPlayerObject) {
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        delete pcmPlayerObject;
        delete pcmBufferQueue;
        pcmPlayerObject = NULL;
        pcmBufferQueue = NULL;
        pcmChannelModePlay = NULL;
        pcmVolumePlay = NULL;
    }
    LOGE("开始释放编解码器 17");
    //3. 销毁混音器
    if (outputMixObject) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    LOGE("开始释放编解码器 18");
    //4. 销毁引擎
    if (engineObject) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    LOGE("开始释放编解码器 19");
    if (this->mBuffers && mBuffers[0]) {
        delete[] mBuffers[0];
        mBuffers[0] = NULL;
    }

    LOGE("开始释放编解码器 20");
    if (soundTouch != NULL) {
        delete soundTouch;
        soundTouch = NULL;
    }
    LOGE("开始释放编解码器 21");
    if (this->mBuffers && mBuffers[1]) {
        delete[] mBuffers[1];
        mBuffers[1] = NULL;
    }
    LOGE("开始释放编解码器 22");
    if (pcmQueue.queueSize() > 0) {
        pcmQueue.clearQueue();
        pcmQueue.setFlag(0);
        pcmQueue.delReleaseCallback();
    }

    if (out_pcm_buffer) {
        free(out_pcm_buffer);
        out_pcm_buffer = NULL;
    }

    LOGE("开始释放编解码器 23");

}















