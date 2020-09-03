package com.devyk.audio_library.callback

/**
 * <pre>
 *     author  : devyk on 2020-05-05 18:24
 *     blog    : https://juejin.im/user/578259398ac2470061f3a3fb/posts
 *     github  : https://github.com/yangkun19921001
 *     mailbox : yang1001yk@gmail.com
 *     desc    : This is ICutCallback
 * </pre>
 */
public interface ICutCallback {


    fun onStart();
    fun onComplete();

    fun onCutPcmData(byte: ByteArray, sampleRate: Int, channel: Int, bit: Int);
}