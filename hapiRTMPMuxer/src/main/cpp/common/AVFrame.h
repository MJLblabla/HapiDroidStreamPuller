//
// Created by 1 on 2022/4/9.
//

#ifndef HAPI_AVFRAME_H
#define HAPI_AVFRAME_H

#include <iostream>

enum AVFrameType {
    TYPE_VIDEO = 0,
    TYPE_AUDIO
};

class AVFrame {
public:
    AVFrameType frameType;

    int len;
    uint8_t *data = nullptr;
    int spsLen = 0;
    int ppsLen = 0;

    AVFrame() {}

    ~AVFrame() {
//        if (data != nullptr) {
//            free(data);
//            data = nullptr;
//        }

    }
};

#endif //HAPISTREAMING_AVFRAME_H
