//
// Created by lyhao on 21-1-20.
//

#include "FFDecode.h"
#include "XLog.h"

extern "C"{
#include "libavcodec/avcodec.h"
}

bool FFDecode::open(XParameter para) {
    if(para.para == nullptr){
        return false;
    }
    AVCodecParameters *p = para.para;
    // 查找解码器
    AVCodec *cd = avcodec_find_decoder(p->codec_id);
    if(cd == nullptr){
        XLOGE("avcodec_find_decoder %d failed", p->codec_id);
        return false;
    }
    // 创建解码上下文，并复制参数
    avCodecContext = avcodec_alloc_context3(cd);
    avcodec_parameters_to_context(avCodecContext, p);

    // 打开解码器
    int re = avcodec_open2(avCodecContext, 0, 0);
    if(re != 0){
        char buf[1024] = {0};
        av_strerror(re,buf,sizeof(buf)-1);
        XLOGE("avcodec_open2 failed = %s",buf);
        return false;
    }
    XLOGI("avcodec_open2 success!");
    return true;
}

//future 模型，发送数据到线程解码
bool FFDecode::sendPacket(XData pkt) {
    if(pkt.size <= 0 || pkt.data == nullptr){
        return false;
    }
    if(avCodecContext == nullptr){
        XLOGE("avCodecContext == nullptr");
        return false;
    }
    int re = avcodec_send_packet(avCodecContext, (AVPacket*)pkt.data);
    if(re != 0){
        XLOGE("avcodec_send_packet failed");
        return false;
    }
    return true;
}

//从线程中读取解码结果
XData FFDecode::recvFrame() {
    if(avCodecContext == nullptr){
        XLOGE("avCodecContext == nullptr");
        return {};
    }
    if(avFrame == nullptr){
        avFrame = av_frame_alloc();
    }
    int re = avcodec_receive_frame(avCodecContext, avFrame);
    if(re != 0){
        XLOGE("avcodec_receive_frame failed");
        return {};
    }
    XData xData;
    xData.data = (unsigned char*) avFrame;
    if(avCodecContext->codec_type == AVMEDIA_TYPE_VIDEO){
        xData.size = (avFrame->linesize[0] + avFrame->linesize[1] + avFrame->linesize[2]) * avFrame->height;
    }
    return xData;
}