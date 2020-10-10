//
// Created by lyhao on 20-10-10.
//

#ifndef XPLAY_FFDEMUX_H
#define XPLAY_FFDEMUX_H

#include "IDemux.h"
struct AVFormatContext;

class FFDemux : public IDemux {
public:
    //打开文件，或者流媒体
    virtual bool Open(const char *url);

    //读取一帧数据，数据由调用者清理
    virtual XData Read();

    FFDemux();

private:
    AVFormatContext *avFormatContext = 0;
};


#endif //XPLAY_FFDEMUX_H
