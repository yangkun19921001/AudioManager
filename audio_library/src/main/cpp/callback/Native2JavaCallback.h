//
// Created by 阳坤 on 2020-04-26.
//

#ifndef AUDIOPLAYER_NATIVE2JAVACALLBACK_H
#define AUDIOPLAYER_NATIVE2JAVACALLBACK_H

#include <android_xlog.h>
#include <jni.h>


#define MAIN_THREAD 0
#define CHILD_THREAD 1

class Native2JavaCallback {

public:
    Native2JavaCallback(JavaVM *vm, JNIEnv *env, jobject *jobj);

    ~Native2JavaCallback();

    void onCallParapred(int type, int i, int i1, int64_t i2, int64_t i3);

    void onCallTimeInfo(int threadType, int curTime, int totalTime);

    void onPlay(int threadTy);

    void onComplete(int threadTy);

    void onPause(int threadTy);

    void onRelease(int threadTy);

    void onError(int threadTy, int errorCode);

    JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj;

    /**
     * 准备工作的回调
     */
    jmethodID jmid_parpared;

    /**
     * 时间进度的回调方法
     */
    jmethodID jmid_timeInfo;
    /**
     * 相当于Java 层 播放完毕
     */
    jmethodID jmid_onComplete;

    /**
     * 相当于Java 层 停止播放
     */
    jmethodID jmid_onPause;

    void toJavaMethod(int threadTy, jmethodID jmetId, int count, ...);


    /**
 * 相当于Java 层 停止
 */
    jmethodID jmid_release;
    /**
     * 开始播放
     */
    jmethodID jmid_play;
    /**
     * 出现错误
     */
    jmethodID jmid_error;

    void onVoiceDBInfo(int threadMode, int db);

    /**
     * 分贝
     */
    jmethodID jmid_voiceDBInfo;

    void onCutAudioPCMData(uint8_t *buffer, int pcmSize, int sampleRate, int channel, int bit);

    /**
     * 截取
     */
    jmethodID jmid_cut_pcm;
    jmethodID jmid_cut_start;
    jmethodID jmid_cut_onComplete;

    void onCutStart(int i);

    void onCutComplete(int i);
};


#endif //AUDIOPLAYER_NATIVE2JAVACALLBACK_H
