1、编译说明
A、toolchain设置
修改env/env.cmake中的toolchain设置
B、编译命令
cd skyipc/build
cmake ..
cmake

2、目录结构
|---- app   上层应用Applacation目录
|         |
|         |---- aac  音频编码模块，将PCM音频数据编码为aac格式。
|         |---- aad  音频解码模块，将aac格式的音频数据解码为PCM.
|         |---- ffmpeg  利用ffmpeg实现音视频处理的代码。
|         |                   本程序实现了将H26X打包到MP4容器的功能。
|         |---- live555rtsp 利用live555开源库，实现RTSP功能。
|         |                        本程序实现了建立RTSP服务器，将H.26X发送到客户端的功能。
|         |---- live555rtsp 利用live555开源库，实现RTSP功能。
|         |---- main.cpp 程序主入口
|         |---- queue 自定义的队列模板类
|         |---- system 系统层。
|         |            |      与芯片基础功能相关的所有系统级操作，在此目录的代码中实现。
|         |            |---- audio_in 芯片音频输入功能实现。
|         |            |---- audio_out 芯片音频输出功能实现。
|         |            |---- audio_out 芯片音频输出功能实现。
|         |            |---- ircut 红外滤波片切换功能
|         |            |---- rgn OSD功能，可以在视频流中叠加文字。
|         |            |---- sensor sensor初始化、启用、设置帧率等相关代码。
|         |            |---- sys 芯片系统层。例如芯片系统初始化、模块绑定。
|         |            |---- venc 编码功能模块。
|         |            |---- vif 视频输入设备、通道绑定。
|         |            |---- vpe ISP图像效果调整模块。
|         |
|         |---- testing 测试用例
|         |                  所有模块的调用示例，在testing下实现。
|         |---- wifi  wifi相关功能
|         |              本程序验证的wifi为Sigmastar SSW101B.
|
|---- build	编译目录
|			编译时进入该目录下 cd build ** cmake ..
|
|---- env   Cmake依赖的环境、变量。
|			可用于设置编译器路径。
|
|---- soc_sdk    芯片原厂的的SDK.
|            |       本套代码就是基于原厂的SDK进行开发的。
|            |---- dynamic 动态库
|            |---- include 头文件
|
|---- thirdparty 第三方依赖库、组件、源码。
|            |
|            |---- libfaac  faac音频编码库源文件，编译后可生成我们所需的头文件、库文件。
|            |---- libfaad  faad音频解码库源文件，编译后可生成我们所需的头文件、库文件。
|            |---- libffmpeg Ffmpeg音视频编解码、处理库。
|            |                      本程序使用Ffmpeg将H.26X裸流放进MP4容器。
|            |---- live555  live555的头文件、库文件。
|            |                   本程序使用live555建立了RTSP服务器，用于将视频传输到RTSP客户端。
|            |---- openssl  没有被用到，仅被live555依赖
