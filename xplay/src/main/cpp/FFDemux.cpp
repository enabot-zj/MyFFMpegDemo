
#include "XLog.h"
#include "FFDemux.h"
extern "C"{
#include <libavformat/avformat.h>
}

//打开文件，或者流媒体
bool FFDemux::Open(const char *url){
    XLOGI("open file %s begin", url);
    //打开url
    int re = avformat_open_input(&avFormatContext, url, 0, 0);
    if(re != 0){
        char buf[1024] = {0};
        av_strerror(re, buf, sizeof(buf));
        XLOGE("FFDemux open %s failed because %s", url, buf);
        return false;
    }
    XLOGI("FFDemux open %s success", url);
    //读取文件信息
    re = avformat_find_stream_info(avFormatContext, 0);
    if(re != 0){
        char buf[1024] = {0};
        av_strerror(re, buf, sizeof(buf));
        XLOGE("avformat_find_stream_info %s failed because %s", url, buf);
        return false;
    }
    this->totalMs = avFormatContext->duration / (AV_TIME_BASE / 1000);
    XLOGI("totalMs = %lld", this->totalMs);
    return true;
}

//读取一帧数据，数据由调用者清理
XData FFDemux::Read(){
    if(avFormatContext == nullptr)
        return XData();

    XData data;
    AVPacket *pkt = av_packet_alloc();
    int re = av_read_frame(avFormatContext, pkt);
    if(re != 0){
        av_packet_free(&pkt);
        XLOGE("av_read_frame error re = %d", re);
        data.size = -1;
        return data;
    }
    XLOGI("pack size is %d , pts is %lld", pkt->size, pkt->pts);
    data.data = (unsigned char *) pkt;
    data.size = pkt->size;
    return data;
}
static bool isFirst = true;
FFDemux::FFDemux() {
    if(isFirst){
        isFirst = false;
        //注册所有封装器
        av_register_all();

        //注册所有的解码器
        avcodec_register_all();

        //初始化网络
        avformat_network_init();
        XLOGI("register ffmpeg!");
    }
}

//获取视频参数
XParameter FFDemux::getVPara() {
    if(avFormatContext == nullptr){
        XLOGE("getVPara failed avFormatContext == nullptr");
        return {};
    }
    int re = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_VIDEO,-1,
            -1, nullptr, 0);
    if(re < 0){
        XLOGE("av_find_best_stream failed!");
        return {};
    }
    XParameter parameter;
    parameter.para = avFormatContext->streams[re]->codecpar;
    return parameter;
}