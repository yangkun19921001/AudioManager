package com.devyk.audioplayer

import android.app.Application
import android.os.Process
import android.text.TextUtils
import com.devyk.audio_library.YKAudioManager
import com.tencent.bugly.crashreport.CrashReport
import java.io.BufferedReader
import java.io.FileReader
import java.io.IOException
import com.devyk.crash_module.Crash
import androidx.core.content.ContextCompat.getSystemService
import android.icu.lang.UCharacter.GraphemeClusterBreak.T
import android.os.Environment
import com.devyk.crash_module.inter.JavaCrashUtils
import com.tencent.mars.xlog.Log
import java.io.File


/**
 * <pre>
 *     author  : devyk on 2020-04-25 17:15
 *     blog    : https://juejin.im/user/578259398ac2470061f3a3fb/posts
 *     github  : https://github.com/yangkun19921001
 *     mailbox : yang1001yk@gmail.com
 *     desc    : This is App
 * </pre>
 */

class App : Application() {

    private val TAG = javaClass.simpleName;

    override fun onCreate() {
        super.onCreate()
        initDevYKNativeCrash();
        initBugly();
        YKAudioManager.initLog(true)
    }

    private fun initDevYKNativeCrash() {
        val filePath ="${Environment.getExternalStorageDirectory()}${File.separator}audioplay";
        if (!File(filePath).exists())
            File(filePath).mkdirs()
        Crash.CrashBuild(applicationContext)
            .nativeCrashPath(filePath)
            .javaCrashPath(filePath, object :JavaCrashUtils.OnCrashListener{
                override fun onCrash(crashInfo: String?, e: Throwable?) {
                    Log.e(TAG,crashInfo);
                }
            })
            .build()
    }

    private fun initBugly() {
        val context = applicationContext
        // 获取当前包名
        val packageName = context.packageName
        // 获取当前进程名
        val processName = getProcessName(Process.myPid())
        // 设置是否为上报进程
        val strategy = CrashReport.UserStrategy(context)
        strategy.isUploadProcess = processName == null || processName == packageName
        // 初始化Bugly
        CrashReport.initCrashReport(context, "bbb5267739", BuildConfig.DEBUG, strategy)
    }

    /**
     * 获取进程号对应的进程名
     *
     * @param pid 进程号
     * @return 进程名
     */
    private fun getProcessName(pid: Int): String? {
        var reader: BufferedReader? = null
        try {
            reader = BufferedReader(FileReader("/proc/$pid/cmdline"))
            var processName = reader.readLine()
            if (!TextUtils.isEmpty(processName)) {
                processName = processName.trim({ it <= ' ' })
            }
            return processName
        } catch (throwable: Throwable) {
            throwable.printStackTrace()
        } finally {
            try {
                if (reader != null) {
                    reader?.close()
                }
            } catch (exception: IOException) {
                exception.printStackTrace()
            }

        }
        return null
    }
}