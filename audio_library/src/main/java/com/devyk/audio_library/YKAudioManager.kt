package com.devyk.audio_library

import android.os.Environment
import com.tencent.mars.xlog.Log
import com.tencent.mars.xlog.Xlog
import java.io.File

/**
 * <pre>
 *     author  : devyk on 2020-04-25 17:17
 *     blog    : https://juejin.im/user/578259398ac2470061f3a3fb/posts
 *     github  : https://github.com/yangkun19921001
 *     mailbox : yang1001yk@gmail.com
 *     desc    : This is AudioUtils
 * </pre>
 */
object YKAudioManager {

    private val TAG = this.javaClass.simpleName

    /**
     * init 框架
     */
    public fun init() {

    }

    /**
     * init log
     */
    public fun initLog(
        isLogOpen: Boolean = true,
        logPath: String = Environment.getExternalStorageDirectory().absolutePath + File.separator + "DevYK/AudioManager",
        cachePath: String = Environment.getExternalStorageDirectory().absolutePath + File.separator + "DevYK/Cache",
        nameprefix: String = "DevYK",
        cacheDay: Int = 3
    ) {
        if (!File(logPath).exists())
            File(logPath).mkdirs()
        if (!File(cachePath).exists())
            File(cachePath).mkdirs()
        Xlog.open(isLogOpen, Xlog.LEVEL_DEBUG, Xlog.AppednerModeAsync, cachePath, logPath, nameprefix, cacheDay, "")
        Log.setLogImp(Xlog())
        Log.e(TAG, "LogInit ---> ");
    }

}