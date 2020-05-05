package com.devyk.audio_library.manager

import com.devyk.audio_library.NativeMethods
import com.devyk.audio_library.callback.IPlayerCallback
import com.tencent.mars.xlog.Log
import java.util.concurrent.ArrayBlockingQueue
import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.ThreadPoolExecutor
import java.util.concurrent.TimeUnit

/**
 * <pre>
 *     author  : devyk on 2020-04-25 16:12
 *     blog    : https://juejin.im/user/578259398ac2470061f3a3fb/posts
 *     github  : https://github.com/yangkun19921001
 *     mailbox : yang1001yk@gmail.com
 *     desc    : This is NativeManager
 * </pre>
 */

object NativeManager {


    private var mNativeMethods = NativeMethods();

    private var mCurVolume = 100;

    /**
     * 线程池管理
     */
    private var mThreadPoolService: ThreadPoolExecutor? = createThreadPool(20,100);


    /**
     * 查看 ffmpeg 版本
     */
    public fun getFFmpegVersion(): String = mNativeMethods.nativeFFmpegVersions()

    /**
     * 准备播放
     */
    public fun prepare(source: String) {
        mThreadPoolService?.execute { mNativeMethods.prepare(source) }
    }

    /**
     * 开始播放
     */
    public fun play() {
        mThreadPoolService?.execute { mNativeMethods.start() }
    }

    /**
     * 暂停
     */
    fun pause() = mNativeMethods.pause()

    /**
     * 恢复
     */
    fun resume() = mNativeMethods.resume()

    /**
     * 停止
     */
    fun stop() {
        mThreadPoolService?.execute {
            mNativeMethods.stop()
            Log.e("开始释放编解码器", "JAVA 层收到");
            mNativeMethods.onRelease()
        }

    }

    /**
     * 快进播放
     */
    fun seek(num: Int) = mThreadPoolService?.execute { mNativeMethods.seek(num) }

    /**
     * 获取当前播放的音量
     */
    fun getVolumePercent(): Int {
//        val volumePercent = mNativeMethods.getVolumePercent();
        return mCurVolume;
    }

    /**
     * 获取播放总长
     */
    fun getStreanPlayDuration(): Int = mNativeMethods.getDuration();

    /**
     * 获取当前播放的音量
     */
    fun setVolume(volume: Int) {
        this.mCurVolume = volume;
        mThreadPoolService?.execute {
            mNativeMethods.setVolumePercent(mCurVolume)
        }
    }

    /**
     * 获取当前播放的声音通道，默认立体声
     */
    fun getChannelMode(): Int = mNativeMethods.getChannelMode();

    /**
     * 设置音频通道
     */
    fun setChannel(channelMode: ChannelConfig) =
        mThreadPoolService?.execute { mNativeMethods.setChannel(channelMode.ordinal) }

    /**
     * 设置回调监听
     */
    public fun addPlayCallback(callback: IPlayerCallback) = mNativeMethods.addPlayCallback(callback)

    /**
     * 设置 play 速度
     */
    fun setSpeed(speed: Float, b: Boolean) = mThreadPoolService?.execute { mNativeMethods.setSpeed(speed, b) }


    fun killThreadPool() {
//        mThreadPoolService?.shutdown();
    }


    fun createThreadPool(corePoolSize: Int = 10, maxPoolSize: Int = 100): ThreadPoolExecutor = ThreadPoolExecutor(
        corePoolSize, maxPoolSize,
        1L, TimeUnit.MILLISECONDS,
        LinkedBlockingQueue<Runnable>(maxPoolSize)
    )
}

