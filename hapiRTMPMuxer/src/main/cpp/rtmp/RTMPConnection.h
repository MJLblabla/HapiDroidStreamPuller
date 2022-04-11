//
// Created by 1 on 2022/4/9.
//

#ifndef HAPISTREAMING_RTMPCONNECTION_H
#define HAPISTREAMING_RTMPCONNECTION_H

#include <unistd.h>
#include <iostream>
#include <librtmp/rtmp.h>
#include "stdio.h"
#include "IConnection.h"
#include "AVFrame.h"

using namespace std;


class RTMPConnection : public IConnection {
private:
    string url;
    RTMP *mRtmp = 0;
    long mStartTime = 0;
    //音频帧队列
    ThreadSafeQueue<RTMPPacket *>
            rtmpPackQueue;
protected:
    void loopPack() override;

    int innerConnect() override;

    void innerDisconnect() override;

    void clear() override;

public:
    RTMPConnection();

    ~RTMPConnection();

    void config(string pushUrl);

    void checkConnectStatus(int setResult);

    void onAVFrame(AVFrame *frame);

};


#endif //HAPISTREAMING_RTMPCONNECTION_H
