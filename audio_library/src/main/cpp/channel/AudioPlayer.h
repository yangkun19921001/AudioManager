//
// Created by 阳坤 on 2020-04-27.
//

#ifndef AUDIOPLAYER_AUDIOCHANNEL_H
#define AUDIOPLAYER_AUDIOCHANNEL_H

#include <Native2JavaCallback.h>
#include "BaseAudioChannel.h"

class AudioPlayer : public BaseAudioChannel {


public:
    AudioPlayer(Status **pStatus, Native2JavaCallback *pCallback);

    ~AudioPlayer();

    void play();

    void pushPCM(int pcmSize, uint8_t *pcm, int curProgress, int totalProgress, int i);

    void start();

    void playThread(int sampleRate);

    pthread_t playId;

    void _seek();

    int getVolumePercent();

    void setVolumePercent(int setVolumePercent);

    int getStreamPlayDuration();

    void setChannel(int type);

    void setSpeed(float speed, bool isPitch);
};


#endif //AUDIOPLAYER_AUDIOCHANNEL_H
