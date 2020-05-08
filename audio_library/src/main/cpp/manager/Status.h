//
// Created by 阳坤 on 2020-04-27.
//

#ifndef AUDIOPLAYER_STATUS_H
#define AUDIOPLAYER_STATUS_H


class Status {
public:
    int exit = false;

    int seek = false;

    Status();

    ~Status();

    /**
     * ffmpeg init thread is exit ?
     */
    int isFFmpegInitThreadExit = false;

    /**
     * 读取待解码线程是否退出
     */
    int isReadFrameThreadExit = false;

    /**
     * 解码线程是否可以退出
     */
    int isDecodecFrameThreadExit = false;

    /**
     * 播放线程是否可以退出
     */
    int isPlayThreadExit = false;

    /**
     * 是否 cut
     */
    int isCut = false;

    /**
     * 裁剪是否还播放
     */
    int isCutPlayer = false;


};


#endif //AUDIOPLAYER_STATUS_H
