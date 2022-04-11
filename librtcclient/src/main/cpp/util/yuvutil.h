#ifndef OPENGLCAMERA2_HAPIRYUVUTI_H
#define OPENGLCAMERA2_HAPIRYUVUTI_H

#include "libyuv.h"

void YuvUtil_ScaleI420(
        uint8_t *srcData, int width, int height,
        uint8_t *dst_data, int dst_width, int dst_height,
        int mode) {


    int src_i420_y_size = width * height;
    int src_i420_u_size = (width >> 1) * (height >> 1);
    uint8_t *src_i420_y_data = srcData;
    uint8_t *src_i420_u_data = srcData + src_i420_y_size;
    uint8_t *src_i420_v_data = srcData + src_i420_y_size + src_i420_u_size;

    int dst_i420_y_size = dst_width * dst_height;
    int dst_i420_u_size = (dst_width >> 1) * (dst_height >> 1);
    uint8_t *dst_i420_y_data = dst_data;
    uint8_t *dst_i420_u_data = dst_data + dst_i420_y_size;
    uint8_t *dst_i420_v_data = dst_data + dst_i420_y_size + dst_i420_u_size;

    I420Scale((const uint8_t *) src_i420_y_data, width,
              (const uint8_t *) src_i420_u_data, width >> 1,
              (const uint8_t *) src_i420_v_data, width >> 1,
              width, height,
              (uint8_t *) dst_i420_y_data, dst_width,
              (uint8_t *) dst_i420_u_data, dst_width >> 1,
              (uint8_t *) dst_i420_v_data, dst_width >> 1,
              dst_width, dst_height,
              (libyuv::FilterMode) mode);


}

#endif