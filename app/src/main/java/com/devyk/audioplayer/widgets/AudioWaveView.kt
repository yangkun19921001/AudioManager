package com.devyk.audioplayer.widgets


import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.RectF
import android.os.Handler
import android.os.Looper
import android.os.Message
import android.util.AttributeSet
import android.view.View
import java.util.Random
import android.icu.lang.UCharacter.GraphemeClusterBreak.T


/**
 * <pre>
 *     author  : devyk on 2020-05-04 23:11
 *     blog    : https://juejin.im/user/578259398ac2470061f3a3fb/posts
 *     github  : https://github.com/yangkun19921001
 *     mailbox : yang1001yk@gmail.com
 *     desc    : This is AudioWaveView
 * </pre>
 *
 * 小细节:
 *  invalidate只会调onDraw方法且必须在UI线程中调用
 *  postInvalidate只会调onDraw方法，可以再UI线程中回调
 *  requestLayout会调onMeasure、onLayout和onDraw(特定条件下)方法
 */
class AudioWaveView : View {
    private var paint: Paint? = null
    private var rectF1: RectF? = null
    private var rectF2: RectF? = null
    private var rectF3: RectF? = null
    private var rectF4: RectF? = null
    private var rectF5: RectF? = null


    private var isLooper = false;
    private var viewWidth: Int = 0
    private var viewHeight: Int = 0
    /** 每个条的宽度  */
    private var rectWidth: Int = 0
    /** 条数  */
    private val columnCount = 5
    /** 条间距  */
    private val space = 6
    /** 条随机高度  */
    private var randomHeight: Int = 0
    private var random: Random? = null


    private val handler = object : Handler() {
        override fun handleMessage(msg: Message) {
            invalidate()
        }
    }


    constructor(context: Context) : super(context) {
        init()
    }

    constructor(context: Context, attrs: AttributeSet) : super(context, attrs) {
        init()
    }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec)

        viewWidth = MeasureSpec.getSize(widthMeasureSpec)
        viewHeight = MeasureSpec.getSize(heightMeasureSpec)

        rectWidth = (viewWidth - space * (columnCount - 1)) / columnCount
    }

    private fun init() {
        paint = Paint()
        paint!!.color = Color.WHITE
        paint!!.style = Paint.Style.FILL
        random = Random()

        initRect()
    }

    private fun initRect() {
        rectF1 = RectF()
        rectF2 = RectF()
        rectF3 = RectF()
        rectF4 = RectF()
        rectF5 = RectF()
    }


    /**
     * 是否开启执行动画
     */
    public fun executeAnim(isLooper: Boolean) {
        this.isLooper = isLooper;
        //必须在 UI 线程中回调
        if (isMainThread())
            invalidate()
        else
            postInvalidate()

    }

    fun isMainThread(): Boolean {
        return Looper.getMainLooper() == Looper.myLooper()
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        val left = rectWidth + space
        if (viewHeight == 0)
            viewHeight+=50;

        //画每个条之前高度都重新随机生成
        randomHeight = random!!.nextInt(viewHeight)
        rectF1!!.set(
            (left * 0).toFloat(),
            randomHeight.toFloat(),
            (left * 0 + rectWidth).toFloat(),
            viewHeight.toFloat()
        )
        randomHeight = random!!.nextInt(viewHeight)
        rectF2!!.set(
            (left * 1).toFloat(),
            randomHeight.toFloat(),
            (left * 1 + rectWidth).toFloat(),
            viewHeight.toFloat()
        )
        randomHeight = random!!.nextInt(viewHeight)
        rectF3!!.set(
            (left * 2).toFloat(),
            randomHeight.toFloat(),
            (left * 2 + rectWidth).toFloat(),
            viewHeight.toFloat()
        )
        randomHeight = random!!.nextInt(viewHeight)
        rectF4!!.set(
            (left * 3).toFloat(),
            randomHeight.toFloat(),
            (left * 3 + rectWidth).toFloat(),
            viewHeight.toFloat()
        )
        randomHeight = random!!.nextInt(viewHeight)
        rectF5!!.set(
            (left * 4).toFloat(),
            randomHeight.toFloat(),
            (left * 4 + rectWidth).toFloat(),
            viewHeight.toFloat()
        )

        canvas.drawRect(rectF1!!, paint!!)
        canvas.drawRect(rectF2!!, paint!!)
        canvas.drawRect(rectF3!!, paint!!)
        canvas.drawRect(rectF4!!, paint!!)
        canvas.drawRect(rectF5!!, paint!!)

        if (isLooper)
            postInvalidateDelayed(200)
//            handler.sendEmptyMessageDelayed(0, 200) //每间隔200毫秒发送消息刷新
    }

}