//
// Created by 阳坤 on 2020-05-06.
//

#ifndef AUDIOPLAYER_PCM_2_MP3_DECODE_H
#define AUDIOPLAYER_PCM_2_MP3_DECODE_H

#include "../callback/Native2JavaCallback.h"
#include <cstring>
#include <stdio.h>

extern "C" {

#include "../lame/libmp3lame/lame.h"
}

class PCM_2_MP3_Decode {
public:
    PCM_2_MP3_Decode(Native2JavaCallback *);

    ~PCM_2_MP3_Decode();


public:

    int init(const char *mp3SavePath, int sampleRate, int channels, uint64_t bitRate);

    int encode(uint8_t *pcm, jbyte * bufferSize,int size);

    void destory();

public:
    Native2JavaCallback *mNative2JavaCallback = NULL;
    FILE *mp3File = NULL;
    lame_t lameClient;
    char *mp3SavePath = NULL;
    int sampleRate;
    int channel;
    int bit;

};


#endif //AUDIOPLAYER_PCM_2_MP3_DECODE_H
