package com.devyk.audioplayer

import android.annotation.SuppressLint
import android.graphics.Color
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.os.Handler
import android.os.Message
import android.view.View
import android.widget.SeekBar
import com.devyk.audio_library.callback.IPlayerCallback
import com.devyk.audio_library.manager.ChannelConfig
import com.devyk.audio_library.manager.NativeManager
import com.tbruyelle.rxpermissions2.RxPermissions
import com.tencent.mars.xlog.Log
import kotlinx.android.synthetic.main.activity_main.*
import android.view.WindowManager
import android.view.animation.Animation
import android.view.animation.AnimationUtils
import android.view.animation.LinearInterpolator
import com.devyk.audio_library.callback.ICutCallback
import java.io.File
import java.io.FileOutputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.util.*


class MainActivity : AppCompatActivity(), View.OnClickListener {


    private val TAG = this.javaClass.simpleName


    /**
     * 直播源：http://livestream.1766.today:1768/live1.mp3
     * 音频源：https://wp.zp68.com/file/47301.html
     * 北京北京MP3：http://0.wp.zp68.com:811/sub/filestores/2018/04/01//a6e2224850a9c65bd6cf99f785fe2c46.mp3?lx=xzwj&k=26779cd9492d42ecba7f
     */
    private val SOURCE =
        "http://0.wp.zp68.com:811/sub/filestores/2018/04/01//a6e2224850a9c65bd6cf99f785fe2c46.mp3?lx=xzwj&k=26779cd9492d42ecba7f"
    //    private val SOURCE = "${Environment.getExternalStorageDirectory()}${File.separator}test.mp3"
    private val SOURCE_LIVE = "http://livestream.1766.today:1768/live1.mp3"

    /**
     * 动画
     */
    private var animator: Animation? = null;

    /**
     *  seek 的状态
     */
    var isSeek = false

    /**
     * 拖动需要设置的 seek value
     */
    var seekValue = -1;


    /**
     * 是否截取
     */
    var isCut = false;

    /**
     * 是否编码为 MP3
     */
    var isEncodeMp3 = false;

    var byteArray: ByteArray? = null;
    var out_mp3_write: FileOutputStream? = null


    /**
     * 更新 UI
     */
    private var handler: Handler = @SuppressLint("HandlerLeak")
    object : Handler() {
        var count = 0;
        override fun handleMessage(msg: Message) {
            super.handleMessage(msg)

            when (msg.what) {
                0x1 -> {
                    if (msg.what == 0x1) {//播放进度


                        //设置百分比进度
                        var cur = msg.data.getInt("cur")
                        var total = msg.data.getInt("total")

                        tv_time.text = msg.obj as String

                        if (total == 0) {
                            return
                        }
                        var pro3 = cur * 100 / total;
                        seekbar_seek.setProgress(pro3)
                    }
                }
                0x2 -> {
                    tv_volume.text = msg.obj as String
                }
                0x3 -> {
                    tv_db.text = "分贝:${msg?.obj as String}"
                }
                0x4 -> {
                    tv_time.text = "00:00 / 00:00"
                    seekbar_seek.setProgress(0)
                    seekbar_volume.setProgress(0)
                    Log.e("开始释放编解码器", "释放成功");
                    animator?.cancel()
                    cd.clearAnimation()
                }
                0x5 -> {
                    NativeManager.stop()
                    autoTestPlay()
                }
                0x6 -> {
                    cd.startAnimation(animator)
                }
            }

        }


    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val window = window
        window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS or WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION)
        window.getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
        )
        window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS)
        window.setStatusBarColor(Color.TRANSPARENT)
        window.setNavigationBarColor(Color.TRANSPARENT)
        setContentView(R.layout.activity_main)


        checkPermission()
        Log.e(TAG, "FFmpegVersion:${NativeManager.getFFmpegVersion()}")
        initViews();
        initClickListener()
        addListener()
        initAnim()
    }

    @SuppressLint("WrongConstant", "ResourceType")
    private fun initAnim() {
        animator = AnimationUtils.loadAnimation(this, R.anim.rotate);
        animator?.setInterpolator(LinearInterpolator())

    }


    private fun initViews() {
        cd.setImageResource(R.mipmap.girl)
    }

    private fun addListener() {
        //添加播放回调
        addPlayerCallback()
        //添加裁剪回调
        addCutCallback()
        //添加声音改变回调
        addSeekVolumeCallback()
        //添加指定 seek 到某一播放时间处
        addSeekCallback()
    }

    private fun addSeekCallback() {
        seekbar_seek.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {

                if (NativeManager.getStreanPlayDuration() > 0 && isSeek) {
                    seekValue = NativeManager.getStreanPlayDuration() * progress / 100;
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
                isSeek = true
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                isSeek = false
                NativeManager.seek(seekValue);
            }
        });
    }

    private fun addSeekVolumeCallback() {
        seekbar_volume.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                updateVolume(progress)
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
            }
        });
    }

    private fun addCutCallback() {
        NativeManager.addCutPcmCallback(object : ICutCallback {
            override fun onStart() {
                Log.e(TAG, "start cut pcm")
            }

            override fun onComplete() {
                Log.e(TAG, "stop cut pcm")
                NativeManager.stop()
                isCut = false
                isEncodeMp3 = false;
                out_mp3_write?.close()
                out_mp3_write = null
            }

            override fun onCutPcmData(byte: ByteArray, sampleRate: Int, channel: Int, bit: Int) {
                //将裁剪出来的 PCM 利用 lame 编码为 mp3
                if (byteArray == null || byteArray?.size != (byte.size * 1.25 + 7200).toInt())
                    byteArray = ByteArray((byte.size * 1.25 + 7200).toInt())

                val encodeMp3 = NativeManager.encodeMp3(byte, byteArray!!, byte.size);
                if (encodeMp3 > 0)//将 MP3 写入文件，其实 native 也可以做写入，看个人选择吧。
                    out_mp3_write?.write(byteArray, 0, encodeMp3);
            }
        });
    }

    private fun addPlayerCallback() {
        NativeManager.addPlayCallback(object : IPlayerCallback {

            /**
             * 播放进度
             */
            override fun onPlayProgress(cur: Int, total: Int) {
                //                Log.e(TAG, "执行线程:${{ Thread.currentThread().name }} onPlayProgress:cur:${cur} total:${total}")
                sendMessage(0x1, cur, total)
            }

            /**
             * 初始化工作成功，可以开始播放了
             */
            @SuppressLint("CheckResult")
            override fun onCallParpared(
                sampleRate: Int, //采样率
                channel: Int, //通道啊
                bitRate: Long, //码率
                duration: Long //总长
            ) {
                updateVolume(20)
                NativeManager.setSpeed(1.0f, true)
                seekbar_volume.setProgress(NativeManager.getVolumePercent())

                if (isCut)
                    NativeManager.cutAudio2Pcm(20, 100, true);


                if (isEncodeMp3) {
                    NativeManager.encodeMP3init(
                        "${Environment.getExternalStorageDirectory()}${File.separator}audioplay/mp3/test.mp3",
                        sampleRate,
                        channel,
                        32
                    )

                    var mp3File =
                        File("${Environment.getExternalStorageDirectory()}${File.separator}audioplay/mp3/test.mp3");
                    if (mp3File.exists())
                        mp3File.delete()
                    mp3File.createNewFile()
                    out_mp3_write = FileOutputStream(mp3File);
                }
                NativeManager.play()
                audio_wave_view.executeAnim(true)
                sendMessage(0x6);


            }

            /**
             * 开始播放了
             */
            override fun onPlay() {
                Log.e(TAG, "执行线程:${{ Thread.currentThread().name }} onPlay")
            }

            /**
             * 停止播放
             */
            override fun onPause() {
                super.onPause()
            }

            /**
             * 播放完成
             */
            override fun onComplete() {
                super.onComplete()
                Log.e(TAG, "执行线程:${{ Thread.currentThread().name }} onStop")
                NativeManager.stop();
            }


            /**
             * 播放出错
             */
            override fun onError(error: String) {
                Log.e(TAG, "执行线程:${{ Thread.currentThread().name }} onError:${error}")
            }

            /**
             * native 释放成功
             */
            override fun onRelease() {
                Log.e(TAG, "执行线程:${{ Thread.currentThread().name }} onRelease")
                sendMessage(0x4, 0, 0)
                audio_wave_view.executeAnim(false)
                sendMessage(0x3, 0, 0, "00")

            }

            /**
             * 分贝回调
             */
            override fun onVoiceDBInfo(db: Int) {
                sendMessage(0x3, 0, 0, db.toString())
                //                Log.e(TAG, "执行线程:${{ Thread.currentThread().name }} 分贝值:${db}")
            }
        })
    }

    private fun updateVolume(progress: Int) {
        NativeManager.setVolume(progress)
        sendMessage(0x2, 0, 0, "音量：${NativeManager.getVolumePercent()}%")
    }

    private fun initClickListener() {
        begin.setOnClickListener(this)
        stop.setOnClickListener(this)
        pause.setOnClickListener(this)
        resume.setOnClickListener(this)
        btn_left.setOnClickListener(this)
        btn_right.setOnClickListener(this)
        btn_center.setOnClickListener(this)
        btn_speed_1_00.setOnClickListener(this)
        btn_speed_2_50.setOnClickListener(this)
        btn_speed_pitch_1_50.setOnClickListener(this)
        btn_speed__pitch_2_00.setOnClickListener(this)
        btn_cut_pcm.setOnClickListener(this)
        btn_pcm_2_mp3.setOnClickListener(this)
    }

    private fun sendMessage(id: Int, a: Int = 0, b: Int = 0, string: String = "") {
        val obtain = Message.obtain()
        obtain.what = id;
        when (id) {
            0x1 -> {
                if (b <= 0)//说明是直播源
                    obtain.obj =
                        "${TimeConverUtils.secdsToDateFormat(a, a)} / Live"
                else
                    obtain.obj =
                        "${TimeConverUtils.secdsToDateFormat(a, a)} / ${TimeConverUtils.secdsToDateFormat(
                            b,
                            b
                        )}"

                val bundle = Bundle();
                bundle.putInt("cur", a);
                bundle.putInt("total", b);
                obtain.data = bundle;
            }
            0x2 -> {
                obtain.obj =
                    string
            }
            0x3 -> {
                obtain.obj =
                    string
            }
            0x4 -> {
                obtain.obj = 0;
            }

            0x5 -> {
                handler.sendMessageDelayed(obtain, (getRandomValue() * 1000).toLong())
                return
            }
        }

        handler.sendMessage(obtain)
    }


    /**
     * 检查权限
     */
    @SuppressLint("CheckResult")
    private fun checkPermission() {
        var rxPermissions = RxPermissions(this);
        rxPermissions.requestEach(
            android.Manifest.permission.READ_EXTERNAL_STORAGE,
            android.Manifest.permission.RECORD_AUDIO,
            android.Manifest.permission.WRITE_EXTERNAL_STORAGE
        ).subscribe {
            if (it.granted) {
                Log.i(TAG, "权限获取成功");
            } else if (it.shouldShowRequestPermissionRationale) {
                Log.i(TAG, "权限获取失败");
            }
        }
    }

    override fun onClick(v: View?) {
        when (v?.id) {
            R.id.begin -> {
                NativeManager.prepare(SOURCE_LIVE)

            }
            R.id.pause -> {
                NativeManager.pause()

            }
            R.id.resume -> {
                NativeManager.resume()
                animator?.start()
                cd.startAnimation(animator)
            }
            R.id.stop -> {
                NativeManager.stop()
            }


            R.id.btn_left -> {
                NativeManager.setChannel(ChannelConfig.CHANNEL_LEFT)
            }
            R.id.btn_center -> {
                NativeManager.setChannel(ChannelConfig.CHANNEL_CENTER)
            }
            R.id.btn_right -> {
                NativeManager.setChannel(ChannelConfig.CHANNEL_RIGHT)
            }

            R.id.btn_speed_1_00 -> {
                NativeManager.setSpeed(1.00f, true)
            }
            R.id.btn_speed_2_50 -> {
                NativeManager.setSpeed(2.50f, false)
            }

            R.id.btn_speed_pitch_1_50 -> {
                NativeManager.setSpeed(1.50f, true)

            }

            R.id.btn_speed__pitch_2_00 -> {
                NativeManager.setSpeed(2.00f, true)
            }

            R.id.btn_cut_pcm -> {
                isCut = true
                NativeManager.prepare(SOURCE)
            }

            R.id.btn_pcm_2_mp3 -> {
                isCut = true
                isEncodeMp3 = true;
                NativeManager.prepare(SOURCE)
            }
        }
    }

    private fun autoTestPlay() {
        NativeManager.prepare(SOURCE)
        sendMessage(0x5, 0, 0)
    }

    override fun onPause() {
        super.onPause()
    }

    override fun onRestart() {
        super.onRestart()
    }

    override fun onDestroy() {
        super.onDestroy()
        cd.clearAnimation()


    }


    public fun getRandomValue(): Int {
        val max = 60;
        val min = 0;
        var delayValue = Random().nextInt(max) % (max - min + 1) + min;
        Log.e(TAG, "自动执行的定时时间为:${delayValue}");
        return delayValue
    }


}
