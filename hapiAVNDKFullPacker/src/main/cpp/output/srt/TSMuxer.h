//
// Created by 1 on 2022/4/6.
//

#ifndef TSMUXER_TSMUXER_H
#define TSMUXER_TSMUXER_H


#include <ts_packet.h>
#include <simple_buffer.h>
#include "mpegts_muxer.h"

class TSMuxer {


private:
    std::map<uint8_t, int> stream_pid_map;
    MpegTsMuxer muxer;

public:
      uint16_t MPEGTS_PMT_PID = 0x100;
      int videoStreamId=0x00020;
      int audioStreamId=0x00021;
    TSMuxer() {
        stream_pid_map.insert(pair<uint8_t, int>(0x1b, videoStreamId));
        stream_pid_map.insert(pair<uint8_t, int>(0x0f, audioStreamId));
    }

    ~TSMuxer() {

    }

    void create_pat(SimpleBuffer *sb){
        muxer.create_pat(sb,MPEGTS_PMT_PID,0);
    }
    void create_pmt(SimpleBuffer *sb){
        muxer.create_pmt(sb,stream_pid_map,MPEGTS_PMT_PID,0);
    }
    void pes_video(SimpleBuffer *sb, uint8_t *buffer,
                    int len,
                    long pts,
                    long dts,bool completed){
        TsFrame *frame = new TsFrame();
        frame->pts=pts;
        frame->dts=dts;
        frame->pid=videoStreamId;
        frame->completed= true;
        frame->stream_type=MpegTsStream::AVC;
       // SimpleBuffer packet;
        frame->_data.get()->append(reinterpret_cast<const char *>(buffer), len);
        //frame->pcr=
        frame->completed=completed;
        frame->expected_pes_packet_length=1316;
        muxer.create_pes(frame,sb);

    }
    void pes_audio(SimpleBuffer *sb, uint8_t *buffer,
                   int len,
                   long pts,
                   long dts){
        TsFrame *frame = new TsFrame();
        frame->pts=pts;
        frame->dts=dts;
        frame->pid=audioStreamId;
        frame->completed= true;
        frame->stream_type=MpegTsStream::AAC;
        // SimpleBuffer packet;
        frame->_data.get()->append(reinterpret_cast<const char *>(buffer), len);
        //frame->pcr=
        frame->expected_pes_packet_length=1316;
        muxer.create_pes(frame,sb);
    }

};

#endif //TSMUXER_TSMUXER_H
