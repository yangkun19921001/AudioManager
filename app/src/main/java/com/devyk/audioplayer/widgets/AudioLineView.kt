package com.devyk.audioplayer.widgets

import android.view.View
import android.content.Context
import android.graphics.*
import android.util.AttributeSet
import com.devyk.audioplayer.R


/**
 * <pre>
 *     author  : devyk on 2020-05-03 13:38
 *     blog    : https://juejin.im/user/578259398ac2470061f3a3fb/posts
 *     github  : https://github.com/yangkun19921001
 *     mailbox : yang1001yk@gmail.com
 *     desc    : This is AudioLineView
 * </pre>
 */
public class AudioLineView : View {
    private var paintVoiceLine: Paint? = null
    private var translateX = 0f
    private var isSet = false  //是否有语音流输入
    private var isShow = true  //是否开始绘制
    private var amplitude = 1f    //振幅
    private var volume = 10f      //音量
    private var fineness = 1       //精细度  值越小，曲线越顺滑，但在一些旧手机上，会出现帧率过低的情况，可以把这个值调大一点，在图片的顺滑度与帧率之间做一个取舍
    private var targetVolume = 1f
    private var voicelinecolor = 0;
    private var lineSpeed = 90     //波动线的横向移动速度，线的速度的反比，即这个值越小，线横向移动越快，越大线移动越慢
    private var lastTime: Long = 0

    var paths: MutableList<Path>? = null

    private var indexColor = -1    //颜色渲染的数组index
    private var inputCount = 0     //用于统计语音输入次数
    private val color = intArrayOf(
        Color.parseColor("#FFFF4744"),
        Color.parseColor("#FFFF8C44"),
        Color.parseColor("#FFFFF344"),
        Color.parseColor("#FF57FF44"),
        Color.parseColor("#FF44FFDD"),
        Color.parseColor("#FF9E44FF"),
        Color.parseColor("#FFFF44BA"),
        Color.parseColor("#FFFF444D")
    )

    private val colors = arrayOf(
        color,
        intArrayOf(color[6], color[0], color[1], color[2], color[3], color[4], color[5]),
        intArrayOf(color[5], color[6], color[0], color[1], color[2], color[3], color[4]),
        intArrayOf(color[4], color[5], color[6], color[0], color[1], color[2], color[3]),
        intArrayOf(color[3], color[4], color[5], color[6], color[0], color[1], color[2]),
        intArrayOf(color[2], color[3], color[4], color[5], color[6], color[0], color[1]),
        intArrayOf(color[1], color[2], color[3], color[4], color[5], color[6], color[0])
    )

    private var linearGradient: LinearGradient? = null

    constructor(context: Context?) : this(context, null)
    constructor(context: Context?, attrs: AttributeSet?) : this(context, attrs, 0)
    constructor(context: Context?, attrs: AttributeSet?, defStyleAttr: Int) : super(context, attrs, defStyleAttr) {
        initAtts(context!!, attrs!!)
    }


    //初始化  设置自定义属性
    private fun initAtts(context: Context, attrs: AttributeSet) {
        val typedArray = context.obtainStyledAttributes(attrs, R.styleable.voiceView)
        lineSpeed = typedArray.getInt(R.styleable.voiceView_lineSpeed, 150)
        fineness = typedArray.getInt(R.styleable.voiceView_fineness, 3)
        voicelinecolor = typedArray.getInt(R.styleable.voiceView_voiceLine, Color.RED)


        paths = ArrayList(15)
        for (i in 0..14) {
            paths!!.add(Path())
        }
        typedArray.recycle()
    }

    protected override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        drawVoiceLine(canvas)
        if (isShow) {//如果是true则刷新 产生动画  否则不刷新  即没有动画效果
            invalidate()
        }
    }

    //绘制图形
    private fun drawVoiceLine(canvas: Canvas) {
        lineChange()

        paintVoiceLine = Paint()
        paintVoiceLine!!.setAntiAlias(true)
        paintVoiceLine!!.setStyle(Paint.Style.STROKE)
        paintVoiceLine!!.setStrokeWidth(5f)
        paintVoiceLine!!.setColor(voicelinecolor)

        canvas.save()
        val moveX = width
        val moveY = height / 2

        //将所有path起点移到左边中点
        for (i in paths!!.indices) {
            paths!![i].reset()
            paths!![i].moveTo(0F, moveY.toFloat())
        }


        //依次从左至右绘制图形
        var i = 0f
        while (i < moveX) {
            amplitude = 4f * volume * i / moveX - 4f * volume * i * i / moveX.toFloat() / moveX.toFloat()
            for (n in 1..paths!!.size) {
                val sin =
                    amplitude * Math.sin((i - Math.pow(1.22, n.toDouble())) * Math.PI / 180 - translateX).toFloat()
                paths!![n - 1].lineTo(i, 2f * n.toFloat() * sin / paths!!.size - 15 * sin / paths!!.size + moveY)
            }
            i += fineness.toFloat()
        }

        //通过设置透明度产生层次效果  180  越大线越不透明
        for (n in paths!!.indices) {
            if (n == paths!!.size - 1) {
                paintVoiceLine!!.setAlpha(255)
            } else {
                paintVoiceLine!!.setAlpha(n * 180 / paths!!.size)
            }
            if (paintVoiceLine!!.getAlpha() > 0) {
                canvas.drawPath(paths!![n], paintVoiceLine!!)
            }
        }
        canvas.restore()
    }

    //对线条颜色进行渲染  inputCount可以控制渲染的速度
    private fun setLinearGradientColor() {
        inputCount++
        if (inputCount % 3 == 0) {
            indexColor++
            if (indexColor > 6) {
                indexColor = 0
            }
            linearGradient = LinearGradient(
                0f,
                (height / 2).toFloat(),
                width.toFloat(),
                (height / 2).toFloat(),
                colors[indexColor],
                null,
                Shader.TileMode.MIRROR
            )
            paintVoiceLine!!.setShader(linearGradient)
        }
    }

    //通过声音改变振幅
    fun setVolume(vol: Int) {
        isSet = true
        if (vol < 30) {
            targetVolume = 0f
        } else if (vol >= 30 && vol < 40) {
            targetVolume = (0.25 * height / 2).toFloat()
        } else if (vol >= 40 && vol < 45) {
            targetVolume = (0.50 * height / 2).toFloat()
        } else if (vol >= 45 && vol < 60) {
            targetVolume = (0.75 * height / 2).toFloat()
        } else {
            targetVolume = (height / 2).toFloat()
        }
    }

    fun isShow(): Boolean {
        return isShow
    }

    //启动动画效果   即开始绘制
    fun startWave() {
        isShow = true
        invalidate()
    }

    //停止动画效果   即停止绘制
    fun stopWave() {
        isShow = false
        invalidate()
    }

    //通过每次改变产生位移量  形成动画
    private fun lineChange() {
        setLinearGradientColor()
        //根据时间改变偏移量
        if (lastTime == 0L) {
            lastTime = System.currentTimeMillis()
            translateX += 1.5f
        } else {
            if (System.currentTimeMillis() - lastTime > lineSpeed) {
                lastTime = System.currentTimeMillis()
                translateX += 1.5f
            } else {
                return
            }
        }
        if (volume < targetVolume && isSet) {     //声贝大于volume  而且是处于声音输入状态
            volume = targetVolume
        } else {
            isSet = false

            if (volume <= 10) {
                volume = 0f
            } else {//当结束说话的时候，将振幅缓缓降下来
                if (volume < height / 10) {
                    volume -= (height / 20).toFloat()
                } else {
                    volume -= (height / 10).toFloat()
                }
            }
        }
    }

}