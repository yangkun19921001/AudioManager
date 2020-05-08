//
// Created by 阳坤 on 2020-04-28.
//

#include "AudioControlManager.h"


AudioPlayer *audioPlayer = NULL;
Native2JavaCallback *mCallback = NULL;
//全局执行的循环任务退出
Status *mStatus = NULL;

PCM_2_MP3_Decode *pcm2Mp3Decode = NULL;

int audioSampleRate = 0;
int audioTotalTime;

AudioControlManager::AudioControlManager(const char *url, JNIEnv *jniEnv, jobject jobject1) {
    this->jniEnv = jniEnv;
    this->jobj = jobject1;
    this->mUrl = new char[strlen(url) + 1];
    strcpy(this->mUrl, url);
    pthread_mutex_init(&release_mutex, NULL);

}

AudioControlManager::~AudioControlManager() {
    pthread_mutex_destroy(&release_mutex);

}

/**
 * 解码完成之后的 PCM 数据需要回调回来
 * @param type
 * @param outPCMSize
 * @param out_buffers
 */
void DecodecFrameCallback(int curTime, int outPCMSize, uint8_t *out_buffers, int oldSize, int sampleRate, int channel,
                          int bit) {
    if (audioPlayer)
        audioPlayer->pushPCM(outPCMSize, out_buffers, curTime, audioTotalTime, oldSize);

    //将裁剪的音频 PCM 原始流回调给 Java 层
    if (mCallback && mStatus && mStatus->isCut)
        mCallback->onCutAudioPCMData(out_buffers, outPCMSize, sampleRate, channel, bit);

};

/**
 * 拿到音频流的回调
 * @param type
 * @param outPCMSize
 * @param out_buffers
 */
void AudioStreamsInfoCallBack(int samRate, int totalTime) {
    audioSampleRate = samRate;
    audioTotalTime = totalTime;

};


/**
 * 运行在子线程的释放工作
 * @param pVoid
 * @return
 */
void release(void *pVoid) {
    auto *audioManager = static_cast<AudioControlManager *>(pVoid);
    audioManager->stop();
}

/**
 * 做一些准备工作
 */
void AudioControlManager::prepare() {
    if (!mCallback) {
        mCallback = new Native2JavaCallback(this->javaVM, jniEnv, &this->jobj);
    }

    if (!mStatus) {
        mStatus = new Status();
        mStatus->isFFmpegInitThreadExit = false;
        mStatus->isDecodecFrameThreadExit = false;
        mStatus->isPlayThreadExit = false;
        mStatus->isReadFrameThreadExit = false;
        mStatus->exit = false;
        mStatus->seek = false;
    }

    if (mStatus && !mAudioPlayer) {
        mAudioPlayer = new AudioPlayer(&mStatus, mCallback);
        audioPlayer = mAudioPlayer;
    }


    if (!mAudioDecodec) {
        mAudioDecodec = new AudioDecodec(mCallback, this->mUrl, &mStatus);
        mAudioDecodec->prepareDecodeThread();
        mAudioDecodec->addDecodecFrameCallback(DecodecFrameCallback);
        mAudioDecodec->addAudioStreamsInfoCallBack(AudioStreamsInfoCallBack);
    }

}


/**
 * 开始播放
 */
void AudioControlManager::start() {
    if (mAudioPlayer) {
        mAudioPlayer->playThread(audioSampleRate);
    }
    if (mAudioDecodec) {
        mAudioDecodec->startDecodeThread();
    }

}


/**
 * 暂停播放
 */
void AudioControlManager::pause() {
    if (mAudioPlayer)
        mAudioPlayer->pause();

}

/**
 * 恢复播放
 */
void AudioControlManager::resume() {
    if (mAudioPlayer)
        mAudioPlayer->resume();

    if (mAudioDecodec) {
        mAudioDecodec->resume();
    }

}


void AudioControlManager::releaseThread() {
    if (!mStatus) {
        LOGE("还未开始播放");
        return;
    }
    stop();
}

/**
 * 指定滑动的位置
 * @param toNumber
 */
void AudioControlManager::seek(int toNumber) {

    if (toNumber <= 0 || toNumber >= getDuration()) {
        LOGE("seek error :%d", toNumber);
        return;
    }
    if (mAudioDecodec) {
        mAudioDecodec->_seek(toNumber);
    }
    if (mAudioPlayer) {
        mAudioPlayer->_seek();
    }
}

/**
 * 获取播放的音量
 * @return
 */
int AudioControlManager::getVolumePercent() {
    if (mAudioPlayer)
        return mAudioPlayer->getVolumePercent();
    return 100;
}


/**
 * 设置音量
 * @param perent
 */
void AudioControlManager::setVolumePercent(int perent) {
    if (mAudioPlayer)
        return mAudioPlayer->setVolumePercent(perent);

}

/**
 * 获取流的播放时长
 * @return
 */
int AudioControlManager::getDuration() {
    int duration = 0;
    if (mAudioPlayer)
        duration = mAudioPlayer->getStreamPlayDuration();
    if (!duration && mAudioDecodec)
        duration = mAudioDecodec->getStreamPlayDuration();
    return duration;
}

void AudioControlManager::setChannel(int type) {
    if (mAudioPlayer)
        return mAudioPlayer->setChannelMode(type);
}

int AudioControlManager::getChannelMode() {
    if (mAudioPlayer)
        return mAudioPlayer->getChannelMode();

    return 2;
}


void AudioControlManager::setSpeed(float speed, bool isPitch) {
    if (mAudioPlayer)
        return mAudioPlayer->setSpeed(speed, isPitch);
}


/**
 * 停止播放
 */
void AudioControlManager::stop() {
    LOGE("开始释放编解码器");
    if (mStatus == NULL || mStatus->exit) {
        return;
    }
    //将当前解码和读取 frame 线程释放
    mStatus->exit = true;
    pthread_mutex_lock(&release_mutex);
    if (!mAudioDecodec)
        return;
    int reCount = 0;
    //保证所有循环都已退出
    while (!exit) {
        //释放播放模块
        if (mStatus && mStatus->isPlayThreadExit
                ) {
            if (mAudioPlayer) {
                mAudioPlayer->release();
                free(mAudioPlayer);
                mAudioPlayer = NULL;
                LOGE("开始释放编解码器 exit AudioPlayer");
            }
        }

        //释放解码模块
        if (mStatus &&
            mStatus->isDecodecFrameThreadExit &&
            mStatus->isReadFrameThreadExit) {
            if (mAudioDecodec && mAudioDecodec->initResult != 0) {
                mAudioDecodec->release();
                free(mAudioDecodec);
                mAudioDecodec = NULL;
                LOGE("开始释放编解码器 exit AudioDecodec");
            }
        }

        //全部退出
        if (mStatus && mStatus->isFFmpegInitThreadExit && mStatus->isPlayThreadExit &&
            mStatus->isDecodecFrameThreadExit &&
            mStatus->isReadFrameThreadExit) {
            exit = true;
            LOGE("开始释放编解码器 exit all");
        }

        //这一步相当于超时主动强制退出
        if (reCount++ > 1000) {
            exit = true;
            LOGE("开始释放编解码器 qz exit all");
            if (mAudioPlayer) {
                mAudioPlayer->release();
                free(mAudioPlayer);
                mAudioPlayer = NULL;
                LOGE("开始释放编解码器 qz exit AudioPlayer");
            }

            if (mAudioDecodec && mAudioDecodec->initResult != 0) {
                mAudioDecodec->release();
                free(mAudioDecodec);
                mAudioDecodec = NULL;
                LOGE("开始释放编解码器 qz exit AudioDecodec");
            }
        }
        av_usleep(1000 * 10);//暂停10毫秒
    }


    //释放 Native2JavaCallback
    if (mCallback) {
        delete (mCallback);
        mCallback = NULL;
    }

    //释放状态机制
    if (mStatus) {
        delete mStatus;
        mStatus = NULL;
    }

    //停止编码 MP3
    if (pcm2Mp3Decode) {
        pcm2Mp3Decode->destory();
        delete (pcm2Mp3Decode);
        pcm2Mp3Decode = NULL;
    }

    pthread_mutex_unlock(&release_mutex);
    LOGE("开始释放编解码器 is ok");
}

/**
 * 将读取出来的音频根据规定的时间裁剪为 PCM
 * @param startTime
 * @param endTime
 * @param isPlayer
 */
void AudioControlManager::cutAudio2Pcm(jint startTime, jint endTime, jboolean isPlayer) {
    if (mAudioDecodec) {
        if (startTime >= 0 && endTime <= this->getDuration()) {
            //开启裁剪
            mStatus->isCut = true;
            mStatus->isCutPlayer = isPlayer;
            mAudioDecodec->cutAudio2Pcm(startTime, endTime, isPlayer);
            //直接指定到解码的时间处
            seek(startTime);
        }
    }
}

/**
 * 初始化
 * @param mp3Path
 * @param sampleRate
 * @param channels
 * @param bitRate
 */
int AudioControlManager::encodeMP3init(const char *mp3Path, int sampleRate, int channels, uint64_t bitRate) {
    int ret = 0;
    if (!pcm2Mp3Decode) {
        pcm2Mp3Decode = new PCM_2_MP3_Decode(mCallback);
        ret = pcm2Mp3Decode->init(mp3Path, sampleRate, channels, bitRate);
    }
    return ret;
}

/**
 * 开始编码为 MP3
 * @param string
 * @param size
 */
int AudioControlManager::encode2mp3(uint8_t *pcm, jbyte * out_mp3, jint size) {
    if (pcm2Mp3Decode)
        return   pcm2Mp3Decode->encode(pcm, out_mp3, size);
    return 0;
}




