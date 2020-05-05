package com.devyk.audio_library.manager


/**
 * <pre>
 *     author  : devyk on 2020-05-02 14:36
 *     blog    : https://juejin.im/user/578259398ac2470061f3a3fb/posts
 *     github  : https://github.com/yangkun19921001
 *     mailbox : yang1001yk@gmail.com
 *     desc    : This is com.devyk.audio_library.manager.ChannelConfig
 * </pre>
 */
enum class ChannelConfig private constructor(private val mode: String, private val value: Int) {
    CHANNEL_RIGHT("RIGHT", 0),
    CHANNEL_LEFT("LEFT", 1),
    CHANNEL_CENTER("CENTER", 2);
}

