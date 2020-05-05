package com.devyk.audio_library.callback

/**
 * <pre>
 *     author  : devyk on 2020-04-26 23:25
 *     blog    : https://juejin.im/user/578259398ac2470061f3a3fb/posts
 *     github  : https://github.com/yangkun19921001
 *     mailbox : yang1001yk@gmail.com
 *     desc    : This is IPlayerCallback
 * </pre>
 */

interface IPlayerCallback {

    fun onCallParpared() {}
    fun onPlay() {}
    fun onComplete() {}
    fun onPause() {}
    fun onError(error: String) {}
    fun onRelease()
    fun onPlayProgress(cur: Int, total: Int)
    fun onVoiceDBInfo(db: Int);

}