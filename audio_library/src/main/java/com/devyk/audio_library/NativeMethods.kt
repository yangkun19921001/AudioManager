package com.devyk.audio_library

import com.devyk.audio_library.callback.ICutCallback
import com.devyk.audio_library.callback.IPlayerCallback
import com.devyk.audio_library.manager.NativeManager
import com.tencent.mars.xlog.Log

/**
 * <pre>
 *     author  : devyk on 2020-04-25 16:02
 *     blog    : https://juejin.im/user/578259398ac2470061f3a3fb/posts
 *     github  : https://github.com/yangkun19921001
 *     mailbox : yang1001yk@gmail.com
 *     desc    : This is NativeManager
 * </pre>
 */

class NativeMethods {

    private val TAG = javaClass.simpleName

    private var mPlayerCallback: IPlayerCallback? = null
    private var mCutCallback: ICutCallback? = null

    companion object {
        init {
            System.loadLibrary("audio")
        }
    }


    /**
     * 获取 ffmpeg version
     */
    public external fun nativeFFmpegVersions(): String

    /**
     * 播放准备
     */
    public external fun prepare(source: String)

    /**
     * 开始播放
     */
    public external fun start();

    /**
     * 暂停
     */
    public external fun pause()

    /**
     * 停止
     */
    public external fun stop()

    /**
     * 恢复播放
     */
    public external fun resume();

    /**
     * Seek
     */
    public external fun seek(num: Int);

    /**
     * c++ 层获取当前流的总长
     */
    public external fun getDuration(): Int;


    /**
     * native 层获取当前播放的音量
     */
    public external fun getVolumePercent(): Int;

    /**
     * native 层获取当前播放的音量
     */
    public external fun setVolumePercent(percent: Int)

    /**
     * 设置音频通道
     */
    public external fun setChannel(ordinal: Int);

    /**
     * 获取音频声音通道模式
     */
    public external fun getChannelMode(): Int;

    /**
     * 设置变速，变调
     * @param speed :播放速度
     * @param isPitch: 是否变调
     */
    public external fun setSpeed(speed: Float, isPitch: Boolean);

    /**
     * @param startTime 裁剪的开始时间
     * @param endTime   裁剪的结束时间
     * @param isPlayer  是否边裁剪边播放
     */
    public external fun cutAudio2Pcm(startTime: Int, endTime: Int, isPlayer: Boolean);


    /**
     * Native 层回调,说明准备好了
     */
    public fun onCallParpared() {
        mPlayerCallback?.onCallParpared()

    }


    /**
     * Native 层回调,播放进度更新
     */
    public fun onCallTimeInfo(cur: Int, total: Int) {
        mPlayerCallback?.onPlayProgress(cur, total)

    }

    /**
     * C++ 层回调，说明开始播放
     */
    public fun onPlay() {
        mPlayerCallback?.onPlay()
    }

    /**
     * C++ 层回调，说明暂停播放
     */
    public fun onPause() {
        mPlayerCallback?.onPause()
    }

    /**
     * C++ 层回调，说明播放完成
     */
    public fun onComplete() {
        mPlayerCallback?.onComplete()
    }

    /**
     * C++ 层回调，说明释放成功
     */
    public fun onRelease() {
        mPlayerCallback?.onRelease()
        NativeManager.killThreadPool()
    }

    /**
     * C++ 回调，通知错误原因
     */
    public fun onError(errorCode: Int) {
        mPlayerCallback?.onError("")
    }

    /**
     * C++ 层回调分贝值
     */
    public fun onVoiceDBInfo(db: Int) {
        mPlayerCallback?.onVoiceDBInfo(db)
    }

    /**
     * 设置 Java 层 播放回调
     */
    public fun addPlayCallback(playerCallback: IPlayerCallback) {
        this.mPlayerCallback = playerCallback;
    }

    /**
     * 设置 Java 层截取回调
     */
    public fun addCutPcmCallback(cutCallback: ICutCallback) {
        this.mCutCallback = cutCallback;
    }

    /**
     * 截取开始
     */
    public fun onCutStart() {
        mCutCallback?.onStart()
    }

    /**
     * 截取结束
     */
    public fun onCutComplete() {
        mCutCallback?.onComplete()
    }

    /**
     * C++ 层回调截取的 PCM 数据
     */
    public fun onCutPcmData(byte: ByteArray, sampleRate: Int, channel: Int, bit: Int) {
        mCutCallback?.onCutPcmData(byte, sampleRate, channel, bit)

    }
}