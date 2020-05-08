//
// Created by 阳坤 on 2020-04-28.
//

#ifndef AUDIOPLAYER_AUDIOCONTROLMANAGER_H
#define AUDIOPLAYER_AUDIOCONTROLMANAGER_H

#include <Native2JavaCallback.h>
#include <AudioDecodec.h>
#include "AudioPlayer.h"
#include "../encode/PCM_2_MP3_Decode.h"

class AudioControlManager {

public:
    AudioControlManager(const char *url, JNIEnv *jniEnv, jobject jobj);

    ~AudioControlManager();


    AudioDecodec *mAudioDecodec = NULL;

    _JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj;

    //播放 PCM 类
    AudioPlayer *mAudioPlayer = NULL;

    char *mUrl = NULL;


    int exit = 0;


    void prepare();

    void start();

    void pause();

    void resume();

    void stop();

    void releaseThread();

    void seek(int toNumber);

    jint getVolumePercent();

    void setVolumePercent(jint i);

    jint getDuration();

    void setChannel(int type);

    int getChannelMode();

    void setSpeed(float speed, bool isPitch);

    pthread_mutex_t release_mutex;

    void cutAudio2Pcm(jint startTime, jint endTime, jboolean isPlayer);

    int encodeMP3init(const char *mp3Path, int sampleRate, int channels, uint64_t bitRate);

    int encode2mp3(uint8_t *string,jbyte * array,jint i);
};


#endif //AUDIOPLAYER_AUDIOCONTROLMANAGER_H
