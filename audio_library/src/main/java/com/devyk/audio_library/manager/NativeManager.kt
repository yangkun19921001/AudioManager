package com.devyk.audio_library.manager

import com.devyk.audio_library.NativeMethods
import com.devyk.audio_library.callback.ICutCallback
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
    private var mThreadPoolService: ThreadPoolExecutor? = createThreadPool(10, 100);


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
        mThreadPoolService?.execute {
            mNativeMethods.start()
        }
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

    /**
     * 裁剪解码后的 PCM
     */
    fun cutAudio2Pcm(startTime: Int, endTime: Int, isPlayer: Boolean) {
        mThreadPoolService?.execute { mNativeMethods.cutAudio2Pcm(startTime, endTime, isPlayer) }
    }

    /**
     * 截取 PCM 回调
     */
    fun addCutPcmCallback(iCutPcmCallback: ICutCallback) = mNativeMethods.addCutPcmCallback(iCutPcmCallback)

    /**
     * 编码 mp3 初始化
     */
    fun encodeMP3init(mp3Path: String, sampleRate: Int, channel: Int, bitRate: Long): Int =
        mNativeMethods.encode2mp3_init(mp3Path, sampleRate, channel, bitRate)

    /**
     * 将 PCM 送入编码器
     */
    fun encodeMp3(byteArray: ByteArray, out_mp3_: ByteArray, size: Int): Int =
        mNativeMethods.encode2mp3(byteArray, out_mp3_, size)
}

