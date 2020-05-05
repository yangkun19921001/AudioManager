# AudioManager

## 介绍

- 主要是对市面上所有的音频的播放，不限于音频格式。

- 对音频数据流的操作，比如裁剪，变声等。

  

  ![](https://devyk.oss-cn-qingdao.aliyuncs.com/blog/20200505195250.gif)

  

**功能点介绍:**

| 功能点                                                       | 是否完成 |
| ------------------------------------------------------------ | -------- |
| 读取任意格式音频流                                           | Yes      |
| FFMPEG 音频解码为 PCM                                        | Yes      |
| 音频 Native OpenSL ES渲染                                    | Yes      |
| 音频音量控制                                                 | Yes      |
| 拖动播放控制                                                 | Yes      |
| 声道切换                                                     | Yes      |
| [变调变速]([SoundTouch](https://gitlab.com/soundtouch/soundtouch)) | Yes      |
| 变声                                                         |          |
| 裁剪音频输出 MP3/PCM 等格式                                  | Pcm-ok   |
| 边播边录制                                                   |          |
| 音频编码为 AAC、MP3、WAV                                     |          |
|                                                              |          |
|                                                              |          |



## 版本记录

### v1.0.1

 - AudioControlManager##stop 会导致 crash 等开发完了在解决->已解决。
 - seek 崩溃已解决

