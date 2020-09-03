//
// Created by 阳坤 on 2020-04-23.
//



#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

#define NATIVE_METHODS_PATH "com/devyk/audio_library/NativeMethods"

#include "AudioControlManager.h"

extern "C" {
#include "lame/libmp3lame/lame.h"
}


AudioControlManager *mAudioControlManager = NULL;
_JavaVM *javaVM = NULL;


static const char *getLameVersion() {
    return get_lame_version();
}

/**
 * ffmpeg version
 * @param jniEnv
 * @param jobject1
 * @return
 */
static jstring Android_JNI_ffmpegVersion(JNIEnv *jniEnv, jobject jobject1) {
    const char *version = AudioDecodec::getFFmpegVersion();
    LOGE("FFmpeg 当前的版本号是:%s", version);
    LOGE("LAME 当前的版本号是:%s", getLameVersion());
    LOGE("FFmpeg 当前的编译信息是:%s",  AudioDecodec::getBuildConfiguration());
    return jniEnv->NewStringUTF(version);
}


/**
 * 准备播放
 * @param jniEnv
 * @param jobject1
 * @param sorce
 */
static void Android_JNI_prepare(JNIEnv *jniEnv, jobject jobject1, jstring sorce) {
    const char *url = jniEnv->GetStringUTFChars(sorce, 0);
    if (!mAudioControlManager) {
        mAudioControlManager = new AudioControlManager(url, jniEnv, jobject1);
        mAudioControlManager->javaVM = javaVM;
        mAudioControlManager->prepare();
    }
    jniEnv->ReleaseStringUTFChars(sorce, url);

}

/**
 * 开始播放
 * @param jniEnv
 * @param jobject1
 */
static void Android_JNI_start(JNIEnv *jniEnv, jobject jobject1) {
    if (mAudioControlManager)
        mAudioControlManager->start();

}


/**
 * 暂停播放
 * @param jniEnv
 * @param jobject1
 */
static void Android_JNI_pause(JNIEnv *jniEnv, jobject jobject1) {
    if (mAudioControlManager)
        mAudioControlManager->pause();
}


/**
 * 恢复播放
 * @param jniEnv
 * @param jobject1
 */
static void Android_JNI_resume(JNIEnv *jniEnv, jobject jobject1) {
    if (mAudioControlManager)
        mAudioControlManager->resume();
}

/**
 * 停止播放
 * @param jniEnv
 * @param jobject1
 */
static void Android_JNI_stop(JNIEnv *jniEnv, jobject jobject1) {
    if (mAudioControlManager) {
        LOGE("开始释放编解码器 Android_JNI_stop");
        mAudioControlManager->releaseThread();
        delete (mAudioControlManager);
        mAudioControlManager = NULL;
        LOGE("开始释放编解码器 通知 JAVA 层");
    }
}

/**
 * 快进
 * @param jniEnv
 * @param jobject1
 * @param seekNumber
 */
static void Android_JNI_seek(JNIEnv *jniEnv, jobject jobject1, jint seekNumber) {
    if (mAudioControlManager) {
        mAudioControlManager->seek(seekNumber);
    }

}

/**
 * 获取当前播放的音量
 * @param jniEnv
 * @param jobject1
 * @return
 */
static jint Android_JNI_getVolumePercent(JNIEnv *jniEnv, jobject jobject1) {
    if (mAudioControlManager)
        return mAudioControlManager->getVolumePercent();
    return 100;

}

/**
 * 设置当前播放的音量
 * @param jniEnv
 * @param jobject1
 */
static void Android_JNI_setVolumePercent(JNIEnv *jniEnv, jobject jobject1, jint percent) {
    if (mAudioControlManager)
        mAudioControlManager->setVolumePercent(percent);

}

/**
 * 获取流播放时间总长
 * @param jniEnv
 * @param jobject1
 * @return
 */
static jint Android_JNI_getDuration(JNIEnv *jniEnv, jobject jobject1) {
    if (mAudioControlManager)
        return mAudioControlManager->getDuration();
    return 0;
}

/**
 * 设置声音通道
 * @param jniEnv
 * @param jobject1
 * @return
 */
static void Android_JNI_setChannel(JNIEnv *jniEnv, jobject jobject1, jint channelMode) {
    if (mAudioControlManager)
        mAudioControlManager->setChannel(channelMode);
}

/**
 * 获取声音通道模式
 * @param jniEnv
 * @param jobject1
 * @return
 */
static jint Android_JNI_getChannelMode(JNIEnv *jniEnv, jobject jobject1) {
    if (mAudioControlManager)
        return mAudioControlManager->getChannelMode();
    return 2;
}

/**
 * 设置播放倍速
 * @param jniEnv
 * @param jobject1
 * @param speed
 * @param sipitch
 */
static void Android_JNI_setSpeed(JNIEnv *jniEnv, jobject jobject1, jfloat speed, jboolean isPitch) {
    if (mAudioControlManager) {
        mAudioControlManager->setSpeed(speed, isPitch);
    }
}

/**
 * 设置播放倍速
 * @param startTime
 * @param endTime
 * @param speed
 * @param isPlayer
 */
static void Android_JNI_cutAudioPcm(JNIEnv *jniEnv, jobject jobject1, jfloat speed, jint startTime, jint endTime,
                                    jboolean isPlayer) {
    if (mAudioControlManager) {
        mAudioControlManager->cutAudio2Pcm(startTime, endTime, isPlayer);
    }
}

/**
 * 编码 MP3 初始化
 * @param jniEnv
 * @param jobject1
 * @param outMp3FilePath
 * @param sampleRate
 * @param channels
 * @param bitRate
 * @return
 */
//char *mp3SavePath, int sampleRate, int channels, int bitRate
static jint Android_JNI_encode2mp3Init(JNIEnv *jniEnv, jobject jobject1, jstring outMp3FilePath, jint sampleRate,
                                       jint channels,
                                       jlong bitRate) {



    int ret = 0;
    if (mAudioControlManager) {
        const char *saveMp3Path = jniEnv->GetStringUTFChars(outMp3FilePath, NULL);
        ret = mAudioControlManager->encodeMP3init(saveMp3Path, sampleRate, channels, bitRate);
        jniEnv->ReleaseStringUTFChars(outMp3FilePath, saveMp3Path);
    }

    return ret;

}

/**
 * 开始编码为 MP3
 */
//char *mp3SavePath, int sampleRate, int channels, int bitRate
static int
Android_JNI_encode2mp3(JNIEnv *jniEnv, jobject jobject1, jbyteArray pcmArray, jbyteArray mp3_buffer, jint size) {
    int ret = 0;
    jbyte *pcmData = jniEnv->GetByteArrayElements(pcmArray, NULL);
    jbyte *out_mp3 = jniEnv->GetByteArrayElements(mp3_buffer, NULL);
    if (mAudioControlManager) {
        return ret = mAudioControlManager->encode2mp3((uint8_t*)pcmData, out_mp3, size);
    }
    jniEnv->ReleaseByteArrayElements(pcmArray, pcmData, NULL);
    jniEnv->ReleaseByteArrayElements(mp3_buffer, out_mp3, NULL);
    return ret;
}

static JNINativeMethod mNativeMethods[] = {
        {"nativeFFmpegVersions", "()Ljava/lang/String;",     (void **) Android_JNI_ffmpegVersion},
        {"prepare",              "(Ljava/lang/String;)V",    (void **) Android_JNI_prepare},
        {"start",                "()V",                      (void **) Android_JNI_start},
        {"pause",                "()V",                      (void **) Android_JNI_pause},
        {"stop",                 "()V",                      (void **) Android_JNI_stop},
        {"seek",                 "(I)V",                     (void **) Android_JNI_seek},
        {"getVolumePercent",     "()I",                      (void **) Android_JNI_getVolumePercent},
        {"setVolumePercent",     "(I)V",                     (void **) Android_JNI_setVolumePercent},
        {"getDuration",          "()I",                      (void **) Android_JNI_getDuration},
        {"setChannel",           "(I)V",                     (void **) Android_JNI_setChannel},
        {"getChannelMode",       "()I",                      (void **) Android_JNI_getChannelMode},
        {"setSpeed",             "(FZ)V",                    (void **) Android_JNI_setSpeed},
        {"cutAudio2Pcm",         "(IIZ)V",                   (void **) Android_JNI_cutAudioPcm},
        {"encode2mp3_init",      "(Ljava/lang/String;IIJ)I", (void **) Android_JNI_encode2mp3Init},
        {"encode2mp3",           "([B[BI)I",                 (void **) Android_JNI_encode2mp3},
        {"resume",               "()V",                      (void **) Android_JNI_resume}
};


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *pVoid) {
    JNIEnv *jniEnv;
    javaVM = vm;
    if (vm->GetEnv(reinterpret_cast<void **>(&jniEnv), JNI_VERSION_1_6) != JNI_OK)
        return JNI_ERR;
    jclass nativeMethodClass = jniEnv->FindClass(NATIVE_METHODS_PATH);
    jniEnv->RegisterNatives(nativeMethodClass, mNativeMethods, NELEM(mNativeMethods));
    jniEnv->DeleteLocalRef(nativeMethodClass);

    return JNI_VERSION_1_6;
}