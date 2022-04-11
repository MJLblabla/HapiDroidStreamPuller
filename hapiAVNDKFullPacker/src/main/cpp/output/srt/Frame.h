//
// Created by 1 on 2022/4/5.
//

#ifndef TSMUXER_FRAME_H
#define TSMUXER_FRAME_H


#include "iostream"

struct Frame {
    uint8_t *buffer;
    int len;
 //   MIMETYPE mimeType;
    long pts;
    long dts;
    bool isKeyFrame;
    short trackId;

    // uint8_t *extra;
    Frame(
            uint8_t *buffer,
            int len,
          //  MIMETYPE mimeType,
            long pts,
            long dts,
            bool isKeyFrame,
            short trackId
    ) {
        this->buffer = buffer;
        this->len = len;
      //  this->mimeType = mimeType;
        this->pts = pts;
        this->dts = dts;
        this->isKeyFrame = isKeyFrame;
        this->trackId = trackId;
    }

    ~Frame() {
        if (buffer != nullptr) {
            free(buffer);
        }
        buffer = nullptr;
    }
};

#endif //TSMUXER_FRAME_H
