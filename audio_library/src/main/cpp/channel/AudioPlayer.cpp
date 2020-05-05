//
// Created by 阳坤 on 2020-04-27.
//


#include "AudioPlayer.h"



AudioPlayer::AudioPlayer(Status **pStatus, Native2JavaCallback *pCallback) : BaseAudioChannel(pStatus, pCallback) {



}

AudioPlayer::~AudioPlayer() {
    LOGE("线程退出---》_play");
    pthread_exit(&this->playId);
}


void *_play(void *pVoid) {
    AudioPlayer *audioPlayer = static_cast<AudioPlayer *>(pVoid);
    audioPlayer->play();
    pthread_exit(&audioPlayer->playId);
}

void AudioPlayer::play() {
    initOpenSLES(this->sampleRate);
}


void AudioPlayer::pushPCM(int pcmSize, uint8_t *pcm, int curProgress, int totalProgress, int oldSize) {
    //TODO ----写入文件测试 PCM
    //FILE *file = fopen("sdcard/ceshi.pcm", "w");
    //fwrite(pcm, pcmSize, 1, file);
//    AV_PCM_DATA *av_pcm_data = new AV_PCM_DATA();
    AV_PCM_DATA *av_pcm_data = getAVPCMPack(pcmSize, curProgress, totalProgress, oldSize);
    memcpy(av_pcm_data->pcm, pcm, pcmSize);
    pcmQueue.push(av_pcm_data);



}

void AudioPlayer::playThread(int sampleRate) {
    this->sampleRate = sampleRate;
    pthread_create(&playId, 0, _play, this);

}

void AudioPlayer::_seek() {
    if (pcmQueue.queueSize() > 0)
        pcmQueue.clearQueue();

    //将当前进度恢复默认
    this->total_time = 0;
    this->pre_tiem = 0;
}

int AudioPlayer::getVolumePercent() {
    return getCurVolume();
}

void AudioPlayer::setVolumePercent(int volumePercent) {
    setVolume(volumePercent);

}

int AudioPlayer::getStreamPlayDuration() {
    return this->duration;
}

/**
 * 设置音频通道
 * @param type
 */
void AudioPlayer::setChannel(int type) {
    setChannelMode(type);
}

void AudioPlayer::setSpeed(float speed, bool isPitch) {
    setSpeedAndPitch(speed,isPitch);
}











