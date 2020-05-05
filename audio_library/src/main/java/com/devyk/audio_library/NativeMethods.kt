package com.devyk.audio_library

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
     * C++ 层回调，说明停止播放
     */
    public fun onStop() {
        mPlayerCallback?.onStop()
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
    public fun onVoiceDBInfo(db:Int){
        mPlayerCallback?.onVoiceDBInfo(db)
    }

    /**
     * 设置 Java 层 回调
     */
    public fun addPlayCallback(playerCallback: IPlayerCallback) {
        this.mPlayerCallback = playerCallback;
    }

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


}