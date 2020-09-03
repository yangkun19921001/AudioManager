//
// Created by 阳坤 on 2020-04-26.
//

#include "Native2JavaCallback.h"


Native2JavaCallback::Native2JavaCallback(JavaVM *vm, JNIEnv *env, jobject *jobj) {
    this->javaVM = vm;
    this->jniEnv = env;
//    this->jobj = *jobj;
    this->jobj = env->NewGlobalRef(*jobj);
    jclass jcls = jniEnv->GetObjectClass(this->jobj);
    //准备播放
    this->jmid_parpared = this->jniEnv->GetMethodID(jcls, "onCallParpared", "(IIJJ)V");
    //开始播放
    this->jmid_play = this->jniEnv->GetMethodID(jcls, "onPlay", "()V");
    //暂停播放
    this->jmid_onPause = this->jniEnv->GetMethodID(jcls, "onPause", "()V");
    //播放完成
    this->jmid_onComplete = this->jniEnv->GetMethodID(jcls, "onComplete", "()V");
    //释放成功
    this->jmid_release = this->jniEnv->GetMethodID(jcls, "onRelease", "()V");
    //播放出错
    this->jmid_error = this->jniEnv->GetMethodID(jcls, "onError", "(I)V");

    //截取回调
    this->jmid_cut_pcm = this->jniEnv->GetMethodID(jcls, "onCutPcmData", "([BIII)V");
    this->jmid_cut_start = this->jniEnv->GetMethodID(jcls, "onCutStart", "()V");
    this->jmid_cut_onComplete = this->jniEnv->GetMethodID(jcls, "onCutComplete", "()V");

    //播放时间回调
    this->jmid_timeInfo = this->jniEnv->GetMethodID(jcls, "onCallTimeInfo", "(II)V");
    //播放分贝回调
    this->jmid_voiceDBInfo = this->jniEnv->GetMethodID(jcls, "onVoiceDBInfo", "(I)V");
}

Native2JavaCallback::~Native2JavaCallback() {
    if (this->jobj) {
        this->jniEnv->DeleteGlobalRef(this->jobj);
    }
    //释放全局
    this->javaVM = NULL;
    this->jobj = NULL;
    this->jniEnv = NULL;
}


void
Native2JavaCallback::onCallParapred(int type, int audioSampleRate, int channels, int64_t bit_rate, int64_t duration) {
    toJavaMethod(type, jmid_parpared, 4, audioSampleRate, channels, bit_rate, duration);

}

/**
 * 开始播放的回调
 */
void Native2JavaCallback::onPlay(int threadType) {
    toJavaMethod(threadType, this->jmid_play, 0);
}

/**
 * 停止播放的回调
 */
void Native2JavaCallback::onComplete(int threadType) {
    toJavaMethod(threadType, this->jmid_onComplete, 0);
}

/**
 * 释放成功的回调
 */
void Native2JavaCallback::onRelease(int threadType) {
    LOGE("开始释放 Native2JavaCallback");
    toJavaMethod(threadType, this->jmid_release, 0);
    LOGE("释放完成 Native2JavaCallback");
//    this->jniEnv->DeleteGlobalRef(this->jobj);
}

/**
 * 出现 error 的回调
 */
void Native2JavaCallback::onError(int threadType, int errorCode) {
    toJavaMethod(threadType, this->jmid_error, 1, errorCode);

}

/**
 *
 * 进度显示
 *
 * @param threadType
 * @param curTime
 * @param totalTime
 *
 */
void Native2JavaCallback::onCallTimeInfo(int threadType, int curTime, int totalTime) {
    toJavaMethod(threadType, this->jmid_timeInfo, 2, curTime, totalTime);
}


/**
 * 通知Java 更新分贝
 * @param threadMode
 * @param db
 */
void Native2JavaCallback::onVoiceDBInfo(int threadMode, int db) {
    toJavaMethod(threadMode, this->jmid_voiceDBInfo, 1, db);
}

/**
 * 执行 Java 方法
 */
void Native2JavaCallback::toJavaMethod(int threadType, jmethodID jmetId, int count, ...) {
    if (threadType == MAIN_THREAD) {//直接在主线程调用 Java 函数
        if (this->jniEnv && this->jobj && jmetId)
            this->jniEnv->CallVoidMethod(this->jobj, jmetId);
    } else if (threadType == CHILD_THREAD) {//如果是子线程那么 JavaVM 需要先依附到当前线程上
        JNIEnv *env;
        if (this->javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("toJavaMethod", "AttachCurrentThread error");
            return;
        }
        if (!count) {
            env->CallVoidMethod(this->jobj, jmetId);
            if (this->javaVM->DetachCurrentThread() != JNI_OK) {
                LOGE("DetachCurrentThread error");
            }
            return;
        }
        va_list ap;//声明一个va_list变量
        va_start(ap, count);//初始化，第二个参数为最后一个确定的形参
//        int *p = new int[count];
//        for (int i = 0; i < count; i++) {
//            p[i] = va_arg(ap, int); //读取可变参数，的二个参数为可变参数的类型
//        }

        switch (count) {
            case 1:
                env->CallVoidMethod(this->jobj, jmetId, va_arg(ap, int));
                break;
            case 2:
                env->CallVoidMethod(this->jobj, jmetId, va_arg(ap, int), va_arg(ap, int));
                break;
            case 4:
                env->CallVoidMethod(this->jobj, jmetId,va_arg(ap, int), va_arg(ap, int), va_arg(ap, uint64_t), va_arg(ap, uint64_t));
                break;
        }
        if (this->javaVM->DetachCurrentThread() != JNI_OK) {
            LOGE("DetachCurrentThread error");
        }

        va_end(ap);
    }
}

/**
 * 将 截取到的 PCM 返回给 Java 层
 * @param buffer
 * @param pcmSize
 * @param sampleRate
 * @param channel
 * @param bit
 */
void Native2JavaCallback::onCutAudioPCMData(uint8_t *buffer, int pcmSize, int sampleRate, int channel, int bit) {
    if (pcmSize <= 0) {
        return;
    }

    JNIEnv *env;
    if (this->javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
        LOGE("toJavaMethod", "AttachCurrentThread error");
        return;
    }
    jbyteArray pcmArray = env->NewByteArray(pcmSize);
    env->SetByteArrayRegion(pcmArray, 0, pcmSize, reinterpret_cast<const jbyte *>(buffer));
    env->CallVoidMethod(this->jobj, this->jmid_cut_pcm, pcmArray, sampleRate, channel, bit);
    env->DeleteLocalRef(pcmArray);
    if (this->javaVM->DetachCurrentThread() != JNI_OK) {
        LOGE("DetachCurrentThread error");
    }
}


/**
 * 开始截取
 * @param threadMode
 */
void Native2JavaCallback::onCutStart(int threadMode) {
    toJavaMethod(threadMode, this->jmid_cut_start, 0);
}

/**
 * 截取完成
 * @param threadMode
 */
void Native2JavaCallback::onCutComplete(int threadMode) {
    toJavaMethod(threadMode, this->jmid_cut_onComplete, 0);
}




