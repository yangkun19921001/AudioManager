package com.devyk.audioplayer

/**
 * <pre>
 *     author  : devyk on 2020-04-29 00:07
 *     blog    : https://juejin.im/user/578259398ac2470061f3a3fb/posts
 *     github  : https://github.com/yangkun19921001
 *     mailbox : yang1001yk@gmail.com
 *     desc    : This is TimeConverUtils
 * </pre>
 */

class TimeConverUtils {
    companion object {
        fun secdsToDateFormat(secds: Int, totalsecds: Int): String {
            val hours = (secds / (60 * 60)).toLong()
            val minutes = (secds % (60 * 60) / 60).toLong()
            val seconds = (secds % 60).toLong()

            var sh = "00"
            if (hours > 0) {
                if (hours < 10) {
                    sh = "0$hours"
                } else {
                    sh = hours.toString() + ""
                }
            }
            var sm = "00"
            if (minutes > 0) {
                if (minutes < 10) {
                    sm = "0$minutes"
                } else {
                    sm = minutes.toString() + ""
                }
            }
            var ss = "00"
            if (seconds > 0) {
                if (seconds < 10) {
                    ss = "0$seconds"
                } else {
                    ss = seconds.toString() + ""
                }
            }
            return if (totalsecds >= 3600) {
                "$sh:$sm:$ss"
            } else "$sm:$ss"

        }
    }
}
