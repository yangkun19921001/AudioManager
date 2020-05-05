//
// Created by 阳坤 on 2020-04-28.
//

#include "AudioControlManager.h"


AudioPlayer *audioPlayer = NULL;

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
void DecodecFrameCallback(int curTime, int outPCMSize, uint8_t *out_buffers, int oldSize) {
    if (audioPlayer)
        audioPlayer->pushPCM(outPCMSize, out_buffers, curTime, audioTotalTime, oldSize);
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
void  release(void *pVoid) {
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
    if (mAudioPlayer)
        return mAudioPlayer->getStreamPlayDuration();
    return 0;
}

void AudioControlManager::setChannel(int type) {
    if (mAudioPlayer)
        return mAudioPlayer->setChannelMode(type);
}

int AudioControlManager::getChannelMode() {
    if (mAudioPlayer)
        return mAudioPlayer->getChannelMode();
}


void AudioControlManager::setSpeed(float speed, bool isPitch) {
    if (mAudioPlayer)
        return mAudioPlayer->setSpeed(speed, isPitch);
}


/**
 * 停止播放
 */
void AudioControlManager::stop() {
    LOGE("AUDIO-DECODEC AudioControlManager stop ----1》");
    LOGE("开始释放编解码器 2");
    if (mStatus == NULL || mStatus->exit) {
        return;
    }
    //将当前解码和读取 frame 线程释放
    mStatus->exit = true;
    pthread_mutex_lock(&release_mutex);
    if (!mAudioDecodec)
        return;
    int reCount = 0;
    LOGE("开始释放编解码器 3");
    //保证所有循环都已退出
    while (!exit) {
        //释放播放模块
        if (mStatus && mStatus->isPlayThreadExit
                ) {
            if (mAudioPlayer) {
                mAudioPlayer->release();
                free(mAudioPlayer);
                mAudioPlayer = NULL;
                LOGE("AudioControlManager exit AudioPlayer");
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
                LOGE("AudioControlManager exit AudioDecodec");
            }
        }

        //全部退出
        if (mStatus && mStatus->isFFmpegInitThreadExit && mStatus->isPlayThreadExit &&
            mStatus->isDecodecFrameThreadExit &&
            mStatus->isReadFrameThreadExit) {
            exit = true;
            LOGE("AudioControlManager exit all");
        }

        //这一步相当于超时主动强制退出
        if (reCount++ > 1000) {
            exit = true;
            LOGE("AudioControlManager qz exit all");
            if (mAudioPlayer) {
                mAudioPlayer->release();
                free(mAudioPlayer);
                mAudioPlayer = NULL;
                LOGE("AudioControlManager exit AudioPlayer");
            }

            if (mAudioDecodec && mAudioDecodec->initResult != 0) {
                mAudioDecodec->release();
                free(mAudioDecodec);
                mAudioDecodec = NULL;
                LOGE("AudioControlManager exit AudioDecodec");
            }
        }
        av_usleep(1000 * 10);//暂停10毫秒
    }


    LOGE("开始释放编解码器 26");
    //释放 Native2JavaCallback
    if (mCallback) {
        delete (mCallback);
        mCallback = NULL;
    }

    LOGE("开始释放编解码器 27");
    //释放状态机制
    if (mStatus) {
        delete mStatus;
        mStatus = NULL;
    }
    pthread_mutex_unlock(&release_mutex);
    LOGE("开始释放编解码器 28");
}




